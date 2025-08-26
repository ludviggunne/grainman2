#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "metatables.h"
#include "../context.h"
#include "../sample.h"


static int
new_sample (lua_State *state)
{
  const char *path = luaL_checkstring (state, 1);
  struct samp *samp = loadsamp (state, path);

  struct ctx *ctx = *(struct ctx **) lua_getextraspace (state);
  resamp (samp, ctx->srate);

  *(struct samp **) lua_newuserdata (state, sizeof (struct samp **)) = samp;
  ctx_append_samp (ctx, samp);

  luaL_setmetatable (state, SAMPLE_MT);
  return 1;
}

void
lua_sample_init (lua_State *state)
{
  luaL_newmetatable (state, SAMPLE_MT);
  lua_pop (state, 1);
  lua_pushcfunction (state, new_sample);
  lua_setglobal (state, "new_sample");
}
