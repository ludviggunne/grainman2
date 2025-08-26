#ifndef SYNTHESIZER_H_INCLUDED
#define SYNTHESIZER_H_INCLUDED

#include <jack/jack.h>
#include <pthread.h>

#include "sample.h"

enum synattr {
  SYN_MIN_LENGTH,
  SYN_MAX_LENGTH,
  SYN_MIN_COOLDOWN,
  SYN_MAX_COOLDOWN,
  SYN_MIN_GAIN,
  SYN_MAX_GAIN,
  SYN_MIN_POINT,
  SYN_MAX_POINT,
  _SYN_ATTR_COUNT,
}; 

struct syn {
  float attrs [_SYN_ATTR_COUNT];
  struct slot *slots;

  /* If non-null, the port used for setting
   * 'tkeys'. */
  struct port *port;

  /* Always updated */
  int tkeys[128];

  /* The keys used when synthesizing.
   * Only copied from 'tkeys' when 'lock'
   * is non-zero, or 'update' is called. */
  int keys[128];

  /* Wether to lock keys */
  int klock;

  struct samp *samp;
};

const char *synattr_str (enum synattr attr);

void synthesize (struct syn *syn, jack_default_audio_sample_t *buf, jack_nframes_t nframes);

#endif
