#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <jack/jack.h>

#include "metatables.h"
#include "../context.h"
#include "../port.h"

static int
new_port (lua_State *state)
{
  const char *name = luaL_checkstring (state, 1);

  struct port *port = calloc (1, sizeof (*port));
  *(struct port **) lua_newuserdata (state, sizeof (struct port *)) = port;

  struct ctx *ctx = *(struct ctx **) lua_getextraspace (state);

  const char *type = JACK_DEFAULT_MIDI_TYPE;
  unsigned long flags = JackPortIsInput;
  port->handle = jack_port_register (ctx->client, name, type, flags, 0);

  if (port->handle == NULL) {
    luaL_error(state, "Failed to register port '%s'", name);
  }

  ctx_append_port (ctx, port);
  luaL_setmetatable (state, PORT_MT);

  return 1;
}

static int
callback (lua_State *state)
{
  struct port *port = *(struct port **) luaL_checkudata (state, 1, PORT_MT);
  luaL_checktype (state, 2, LUA_TFUNCTION);

  port->cbref = luaL_ref (state, LUA_REGISTRYINDEX);

  return 0;
}

const static luaL_Reg methods[] = {
  { .name = "callback", .func = callback, },
  { NULL, NULL },
};

void
lua_port_init (lua_State *state)
{
  luaL_newmetatable (state, PORT_MT);

  lua_newtable (state);
  luaL_setfuncs (state, methods, 0);
  lua_setfield (state, -2, "__index");

  lua_pop (state, 1);

  lua_pushcfunction (state, new_port);
  lua_setglobal (state, "new_port");
}
