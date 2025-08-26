#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include <stddef.h>

#include <lua.h>

struct samp {
  float *data;
  size_t size;
  unsigned int srate;
  char *path;
};

struct samp *loadsamp (lua_State *state, const char *path);
void resamp (struct samp *samp, unsigned int srate);

#endif
