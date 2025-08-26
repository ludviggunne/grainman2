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

int
main (int argc, char **argv)
{
  atexit (cleanup);

  (void) argc;
  (void) argv;

  if ((client = jack_client_open ("port-connect", 0, NULL)) == NULL) {
    fprintf (stderr, "fatal: failed to open client\n");
    exit (EXIT_FAILURE);
  }

  {
    const char **ports = jack_get_ports (client, NULL, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput);
    printf ("Destinations:\n");
    for (const char **port = ports; *port; ++port) {
      printf ("\t%s\n", *port);
    }
    jack_free (ports);
    printf ("\n");
  }

  {
    const char **ports = jack_get_ports (client, NULL, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput);
    printf ("Sources:\n");
    for (const char **port = ports; *port; ++port) {
      printf ("\t%s\n", *port);
    }
    jack_free (ports);
    printf ("\n");
  }
}
