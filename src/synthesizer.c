#include "synthesizer.h"

struct slot {
  int length;
  int cooldown; 
  int cursor;
  float gain;
};

const char *synattr_str (enum synattr attr)
{
  switch (attr) {
  case SYN_MIN_LENGTH: return "min length";
  case SYN_MAX_LENGTH: return "max length";
  case SYN_MIN_COOLDOWN: return "min cooldown";
  case SYN_MAX_COOLDOWN: return "max cooldown";
  case SYN_MIN_GAIN: return "min gain";
  case SYN_MAX_GAIN: return "max gain";
  case SYN_MIN_POINT: return "min point";
  case SYN_MAX_POINT: return "max point";
  default: return "???";
  }
}

void
synthesize (struct syn *syn, jack_default_audio_sample_t *buf, jack_nframes_t nframes)
{
  // TODO
}

