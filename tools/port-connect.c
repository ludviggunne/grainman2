#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jack/jack.h>

static jack_client_t *client = NULL;

static void
cleanup (void)
{
  if (client) {
    jack_client_close (client);
  }
}

static char *
findport (const char *pat, unsigned long flags)
{
  const char **ports = jack_get_ports (client, pat, JACK_DEFAULT_MIDI_TYPE, flags);

  if (ports == NULL || ports[0] == NULL) {
    fprintf (stderr, "fatal: no port matching pattern '%s'\n", pat);
    exit (EXIT_FAILURE);
  }

  if (ports[1]) {
    fprintf (stderr, "fatal: ambiguous port pattern '%s'\n", pat);
    exit (EXIT_FAILURE);
  }

  char *port = strdup (ports[0]);
  jack_free (ports);

  return port;
}

int
main (int argc, char **argv)
{
  atexit (cleanup);

  if (argc < 3) {
    fprintf (stderr, "%s [src] [dst]\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  const char *srcpat = argv[1], *dstpat = argv[2];

  if ((client = jack_client_open ("port-connect", 0, NULL)) == NULL) {
    fprintf (stderr, "fatal: failed to open client\n");
    exit (EXIT_FAILURE);
  }

  char *srcport = findport (srcpat, JackPortIsOutput);
  char *dstport = findport (dstpat, JackPortIsInput);

  if (jack_connect (client, srcport, dstport) != 0) {
    fprintf (stderr, "fatal: failed to connect '%s' to '%s'\n", srcport, dstport);
    exit (EXIT_FAILURE);
  }
}
