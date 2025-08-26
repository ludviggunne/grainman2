#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "context.h"
#include "synthesizer.h"
#include "port.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <jack/midiport.h>

#define RBUF_SIZE 1024

static int process_cb (jack_nframes_t, void *);
static int srate_cb (jack_nframes_t, void *);
static void port_connect_cb (jack_port_id_t, jack_port_id_t, int, void *);

void
ctx_cleanup (struct ctx *ctx)
{
  /* Free synthesizers */
  for (int i = 0; i < ctx->nsyns; i++) {
    struct syn *syn = ctx->syns[i];
    free (syn->slots);
    free (syn);
  }
  free (ctx->syns);

  /* Disconnect, unregister and free ports */
  for (int i = 0; i < ctx->nports; i++) {
    struct port *port = ctx->ports[i];
    if (jack_port_connected (port->handle)) {
      jack_port_disconnect (ctx->client, port->handle);
    }
    jack_port_unregister (ctx->client, port->handle);
    free (port);
  }
  free (ctx->ports);

  /* Free up some JACK objects */
  if (ctx->rbuf) {
    jack_ringbuffer_free (ctx->rbuf);
  }

  if (ctx->client) {
    jack_client_close (ctx->client);
  }

  memset (ctx, 0, sizeof (*ctx));
}

void
ctx_init (struct ctx *ctx, const char *client_name)
{
  memset (ctx, 0 , sizeof (*ctx));

  /* Open JACK client */
  ctx->client = jack_client_open (client_name, 0, NULL);
  if (ctx->client == NULL) {
    fprintf (stderr, "Failed to open JACK client '%s'\n", client_name);
    exit (EXIT_FAILURE);
  }

  /* Set JACK callbacks */
  jack_set_process_callback (ctx->client, process_cb, ctx);
  jack_set_port_connect_callback (ctx->client, port_connect_cb, ctx);
  jack_set_sample_rate_callback (ctx->client, srate_cb, ctx);

  /* Create ringbuffer */
  ctx->rbuf = jack_ringbuffer_create (RBUF_SIZE);

  /* Create lua state and load libraries */
  ctx->state = luaL_newstate ();
  luaL_openlibs (ctx->state);

  /* Store pointer to context in lua state */
  *(struct ctx **) lua_getextraspace (ctx->state) = ctx;

  /* Register objects, methods, functions... */
  extern void lua_synthesizer_init (lua_State *state);
  extern void lua_port_init (lua_State *state);
  extern void lua_sample_init (lua_State *state);

  lua_synthesizer_init (ctx->state);
  lua_port_init (ctx->state);
  lua_sample_init (ctx->state);
}

void
ctx_append_syn (struct ctx *ctx, struct syn *syn)
{
  ctx->nsyns++;
  ctx->syns = realloc (ctx->syns, sizeof (struct syn *) * ctx->nsyns);
  ctx->syns [ctx->nsyns-1] = syn;
}

void
ctx_append_port (struct ctx *ctx, struct port *port)
{
  ctx->nports++;
  ctx->ports = realloc (ctx->ports, sizeof (struct port *) * ctx->nports);
  ctx->ports [ctx->nports-1] = port;
}

void
ctx_append_samp (struct ctx *ctx, struct samp *samp)
{
  ctx->nsamps++;
  ctx->samps = realloc (ctx->samps, sizeof (struct samp *) * ctx->nsamps);
  ctx->samps [ctx->nsamps-1] = samp;
}

static int
process_cb (jack_nframes_t nframes, void *arg)
{
  struct ctx *ctx = arg;
  for (int i = 0; i < ctx->nports; ++i) {

    struct port *port = ctx->ports[i];
    if (port->cbref == LUA_NOREF) {
      /* No callback for this port */
      continue;
    }

    void *buf = jack_port_get_buffer (port->handle, nframes);
    int nevents = jack_midi_get_event_count (buf);

    for (int j = 0; j < nevents; j++) {
      jack_midi_event_t event;
      jack_midi_event_get (&event, buf, j);

      // TODO: match timestamp

      /* Run callback with new message object */
      lua_rawgeti (ctx->state, LUA_REGISTRYINDEX, port->cbref);
      lua_newtable (ctx->state);
      lua_pushinteger (ctx->state, event.buffer[0]);
      lua_setfield (ctx->state, -2, "status");
      lua_pushinteger (ctx->state, event.buffer[1]);
      lua_setfield (ctx->state, -2, "data1");
      lua_pushinteger (ctx->state, event.buffer[2]);
      lua_setfield (ctx->state, -2, "data2");

      if (lua_pcall (ctx->state, 1, 0, 0) != LUA_OK) {
        const char *err = luaL_checkstring (ctx->state, -1);
        fprintf (stderr, "lua error: %s\n", err);
      }

      for (int k = 0; k < ctx->nsyns; k++) {
        struct syn *syn = ctx->syns[k];

        if (syn->port != port) {
          continue;
        }

        /* Set/unset keys for connected synthesizer */
        int status = event.buffer[0];
        int data1 = 127 & event.buffer[1];

        if (status == 144 /* note on */) {
          syn->tkeys[data1] = 1;
        } else if (status == 128 /* note off */) {
          syn->tkeys[data1] = 0;
        }
      }
    }
  }

  return 0;
}

static int
srate_cb (jack_nframes_t new_srate, void *arg)
{
  ((struct ctx *) arg)->srate = new_srate;
  printf ("new sample rate %d\n", new_srate);
  return 0;
}

static void
port_connect_cb (jack_port_id_t port_id_1, jack_port_id_t port_id_2, int connect, void *arg)
{
  struct ctx *ctx;
  const jack_port_t *port_1, *port_2;
  const char *name_1, *name_2, *status;

  ctx = arg;

  port_1 = jack_port_by_id (ctx->client, port_id_1);
  port_2 = jack_port_by_id (ctx->client, port_id_2);

  /* Just print info if one of the ports is
   * registered by us. */
  int one_is_ours = 0;
  for (int i = 0; i < ctx->nports; ++i) {
    jack_port_t *port = ctx->ports[i]->handle;
    if (port == port_1 || port == port_2) {
      one_is_ours = 1;
      break;
    }
  }

  if (!one_is_ours) {
    return;
  }

  name_1 = jack_port_name (port_1);
  name_2 = jack_port_name (port_2);

  status = connect ? "connected" : "disconnected";

  printf ("%s ports '%s' and '%s'\n", status, name_1, name_2);
}
