#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <pthread.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <jack/jack.h>

#include "context.h"

static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

static void
SIGINT_handler (int sig)
{
  pthread_cond_signal (&cv);
}

int
main (int argc, char **argv)
{
  if (argc < 2) {
    fprintf (stderr, "usage: %s <file> [options...]\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  signal (SIGINT, SIGINT_handler);

  const char *path = argv[1];

  struct ctx ctx;
  ctx_init (&ctx, "grainman");

  if (luaL_dofile (ctx.state, path) != LUA_OK) {
    const char *err = luaL_checkstring (ctx.state, -1);
    fprintf (stderr, "%s\n", err);
    ctx_cleanup (&ctx);
    exit (EXIT_FAILURE);
  }

  jack_activate (ctx.client);

  printf ("started...\n");
  printf ("press ctrl-C to exit\n");

  pthread_cond_wait (&cv, &mx);
  jack_deactivate (ctx.client);
  ctx_cleanup (&ctx);

  printf ("goodbye!\n");
}
