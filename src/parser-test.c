/*
 * Devil's Pie -- a window matching tool.
 * Copyright (C) 2002 Ross Burton <ross@burtonini.com>
 * Licensed under the GPL (2.0 or above)
 * 
 * Inspired by the Matched Windows feature in Sawfish, and implemented
 * as a seperate program so that Metacity can remain clean, lean, and
 * crack-free.
 */

#include "devilspie.h"
#include "flurb.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

#include "gtk/gtk.h"
#include "glib/glist.h"
#include "libwnck/libwnck.h"

/*
 * The list of flurbs to execute
 */
GList *flurbs = NULL;

static void free_a_flurb(Flurb *flurb, gpointer user_data) {
  flurb_free(flurb);
  return;
}

int main(int argc, char **argv) {
  int counter = 1000;

  if (argc != 2) {
    g_print("Usage: %s configuration-filename\n", argv[0]);
    return 1;
  }

  g_type_init();
  flurb_init();

  while (counter--) {
    /* Load the config file */
    load_configuration(argv[1]);
    /* Free the memory */
    g_list_foreach (flurbs, (GFunc)free_a_flurb, NULL);
    g_list_free(flurbs);
    flurbs = NULL;
  }

  return 0;
}
