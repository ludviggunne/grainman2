#include <stdlib.h>
#include <string.h>

#include <sndfile.h>
#include <samplerate.h>

#include <lauxlib.h>
#include <lualib.h>

#include "sample.h"

struct samp *
loadsamp (lua_State *state, const char *path)
{
  SNDFILE *f = NULL;
  SF_INFO info = {0};

  f = sf_open (path, SFM_READ, &info);
  if (f == NULL) {
    luaL_error (state, "failed to load sample %s: %s\n", path, sf_strerror (f));
  }

  struct samp *samp = malloc (sizeof (*samp));
  size_t nsamps = info.frames * info.channels;
  samp->data = malloc (sizeof (*samp->data) * nsamps);

  sf_count_t offset = 0;
  while (offset < nsamps) {
    offset += sf_read_float (f, samp->data+offset, nsamps-offset);
  }

  if (info.channels > 1) {
    printf ("downmixing %s from %d channels to 1\n", path, info.channels);

    for (size_t i = 0; i < info.frames; i++) {
      float acc = 0.f;
      for (size_t c = 0; c < info.channels; c++) {
        acc += samp->data[info.channels*i+c];
      }
      samp->data[i] = acc;
    }
  }

  samp->size = info.frames;
  samp->srate = info.samplerate;
  samp->path = strdup (path);

  return samp;
}

/* TODO: use callback API here to provide progress indicator */
void
resamp (struct samp *samp, unsigned int srate)
{
  double ratio = (double) srate / (double) samp->srate;
  size_t new_size = (size_t) (samp->size * ratio);
  float *new_data = malloc (sizeof (*new_data) * new_size);

  SRC_DATA src_data = {
    .data_in = samp->data,
    .data_out = new_data,
    .input_frames = samp->size,
    .output_frames = new_size,
    .src_ratio = ratio,
  };

  printf ("resampling %s from %d to %d\n", samp->path, samp->srate, srate);
  int error = src_simple (&src_data, SRC_SINC_BEST_QUALITY, 1);

  if (error != 0) {
    free (new_data);
    fprintf (stderr, "Failed to resample '%s': %s", samp->path, src_strerror (error));
    return;
  }

  free (samp->data);
  samp->data = new_data;
  samp->size = new_size;
  samp->srate = srate;
}

