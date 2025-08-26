#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "metatables.h"
#include "../context.h"
#include "../synthesizer.h"

static int
new_synthesizer (lua_State *state)
{
  struct samp *samp = *(struct samp **) luaL_checkudata (state, 1, SAMPLE_MT);
  struct syn *syn = calloc (1, sizeof (*syn));
  syn->samp = samp;
  *(struct syn **) lua_newuserdata (state, sizeof (struct syn *)) = syn;
  struct ctx *ctx = *(struct ctx **) lua_getextraspace(state);
  ctx_append_syn (ctx, syn);
  luaL_setmetatable (state, SYNTHESIZER_MT);
  return 1;
}

static int
setattr (lua_State *state)
{
  struct syn *syn = *(struct syn **) luaL_checkudata (state, 1, SYNTHESIZER_MT);
  float v = luaL_checknumber (state, 2);
  enum synattr attr = luaL_checkinteger (state, lua_upvalueindex(1));
  syn->attrs[attr] = v;
  return 0;
}

static int
map_port (lua_State *state)
{
  struct syn *syn = *(struct syn **) luaL_checkudata (state, 1, SYNTHESIZER_MT);
  struct port *port = *(struct port **) luaL_checkudata (state, 2, PORT_MT);
  syn->port = port;
  return 0;
}

static int
lock_keys (lua_State *state)
{
  struct syn *syn = *(struct syn **) luaL_checkudata (state, 1, SYNTHESIZER_MT);
  syn->klock = 1;
  return 0;
}

static int
update_keys (lua_State *state)
{
  struct syn *syn = *(struct syn **) luaL_checkudata (state, 1, SYNTHESIZER_MT);
  memcpy (syn->keys, syn->tkeys, sizeof (syn->keys));
  printf ("hej\n");
  return 0;
}

static void
reg_setattr (lua_State *state, enum synattr attr, const char *name)
{
  /* Register 'setattr' as function 'name' with upvalue 'attr' */
  lua_pushinteger (state, attr);
  lua_pushcclosure (state, setattr, 1);
  lua_setfield (state, -2, name);
}

/* Methods for setting attributes */
static struct {
  enum synattr attr;
  const char *name;
} setattr_regs[] = {
  { SYN_MIN_LENGTH, "min_length" },
  { SYN_MAX_LENGTH, "max_length" },
  { SYN_MIN_COOLDOWN, "min_cooldown" },
  { SYN_MAX_COOLDOWN, "max_cooldown" },
  { SYN_MIN_GAIN, "min_gain" },
  { SYN_MAX_GAIN, "max_gain" },
  { SYN_MIN_POINT, "min_point" },
  { SYN_MAX_POINT, "max_point" },
};

#define METHOD(name_) { .name = # name_, .func = name_, }

const static luaL_Reg methods[] = {
  METHOD (map_port),
  METHOD (lock_keys),
  METHOD (update_keys),
  { NULL, NULL },
};

void lua_synthesizer_init (lua_State *state)
{
  luaL_newmetatable (state, SYNTHESIZER_MT);
  lua_newtable (state);

  for (size_t i = 0; i < _SYN_ATTR_COUNT; i++) {
    reg_setattr (state, setattr_regs[i].attr, setattr_regs[i].name);
  }

  luaL_setfuncs (state, methods, 0);

  lua_setfield (state, -2, "__index");
  lua_pop (state, 1);

  lua_pushcfunction (state, new_synthesizer);
  lua_setglobal (state, "new_synthesizer");
}
