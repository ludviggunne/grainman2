#ifndef PORT_H_INCLUDED
#define PORT_H_INCLUDED

#include <jack/jack.h>

struct port {
  jack_port_t *handle;
  int cbref;
};

#endif
