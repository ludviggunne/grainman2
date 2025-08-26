#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include <lua.h>

#include <jack/jack.h>
#include <jack/ringbuffer.h>

#include "synthesizer.h"
#include "port.h"

struct ctx {
  lua_State *state;
  jack_client_t *client;
  jack_nframes_t srate;
  jack_ringbuffer_t *rbuf;
  int nports;
  int nsyns;
  int nsamps;
  struct port **ports;
  struct syn **syns;
  struct samp **samps;
};

void ctx_init (struct ctx *ctx, const char *client_name);
void ctx_cleanup (struct ctx *ctx);
void ctx_append_syn (struct ctx *ctx, struct syn *syn);
void ctx_append_port (struct ctx *ctx, struct port *port);
void ctx_append_samp (struct ctx *ctx, struct samp *samp);

#endif
