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
gboolean apply_to_existing = TRUE;

/**
 * Run a flurb. Called by the window_opened handler
 */
static void run_flurb(Flurb * flurb, WnckWindow *window) {
  GList *l;

  /* If there are no matchers, abort the Flurb */
  if (g_list_first (flurb->matchers) == NULL) return;
  /* First, run all matchers. If any return false, abort this Flurb */
  for (l = g_list_first (flurb->matchers); l != NULL; l = g_list_next(l)) {
    DevilsPieMatcher *m = l->data;
    if (!devilspie_matcher_test(m, window)) return;
  }
  /* If we got here, this is a matching Flurb. Run all actions. */
  for (l = g_list_first (flurb->actions); l != NULL; l = g_list_next(l)) {
    DevilsPieAction *a = (DevilsPieAction*)l->data;
    devilspie_action_run(a, window);
  }
}

/**
 * This callback is called whenever a window is opened on a screen.
 */
void window_opened_cb(WnckScreen *screen, WnckWindow *window) {
  g_list_foreach(flurbs, (GFunc)run_flurb, window);
  return;
}

/*
 * Dedicated to Vicky.
 */
int main(int argc, char **argv) {
  WnckScreen *screen;
  gchar *filename;

  /* Initialize GTK+ and the Flurb type system */
  gtk_init(&argc, &argv);
  flurb_init();

  /* if no arguments, look for ~/.devilspie.xml. If 1 argument, open
     that filename. Otherwise, bitch */
  if (argc == 2 ) {
    filename = g_strdup(argv[1]);
  } else if (argc == 1) {
    filename = g_strdup_printf("%s/%s", g_get_home_dir(), ".devilspie.xml");
  } else {
    g_print("Usage: devilspie [configuration file]\n"
            "If no configuration file is specified, ~/.devilspie.xml is used.\n");
    return 1;
  }
  /* Check if the file exists. A bit crap but works. Should I use
     access() instead? */
  if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
    g_print("File %s does not exist\n", filename);
    return 1;
  }
  /* Load the configuration file */
  load_configuration(filename);
  g_free(filename);

  /* Get the default screen from libwnck. Obviously needs work here
     for multi screen systems */
  screen = wnck_screen_get_default ();
  /* TODO: use popt to parse arguments and set this if --apply-to-all | -a is set */
  if (!apply_to_existing) wnck_screen_force_update(screen);

  /* Connect a callback to the window opened event in libwnck */
  g_signal_connect (screen, "window_opened", (GCallback)window_opened_cb, NULL);

  /* Go go go! */
  gtk_main();
  return 0;
}
