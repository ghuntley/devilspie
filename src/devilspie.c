/*
 * Devil's Pie -- a window matching tool.
 * Copyright (C) 2002 Ross Burton <ross@burtonini.com>
 * Licensed under the GPL (2.0 or above)
 * 
 * Inspired by the Matched Windows feature in Sawfish, and implemented
 * as a seperate program so that Metacity can remain clean, lean, and
 * crack-free.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glib/glist.h>
#include <popt.h>
#include <stdlib.h>
#include <libwnck/libwnck.h>
#include "flurb.h"
#include "config-parser.h"
#include "devilspie-matcher.h"
#include "devilspie-action.h"

/**
 * The list of flurbs to execute.
 */
GList *flurbs = NULL;

/**
 * If we apply to existing windows or not.
 */
gboolean apply_to_existing = FALSE;

/**
 * The popt argument list.
 */
static struct poptOption options[] = {
  {
    "apply-to-existing", 'a', POPT_ARG_NONE, &apply_to_existing, 0,
    "Apply to all existing windows instead of just new windows.", NULL
  },
  POPT_AUTOHELP
  { NULL }
};


/**
 * This callback is called whenever a window is opened on a screen.
 */
void window_opened_cb(WnckScreen *screen, WnckWindow *window) {
  g_list_foreach(flurbs, (GFunc)run_flurb, window);
  return;
}

static void init_screens(void) {
  GdkDisplay *display;
  int i, num_screens;

  display = gdk_display_get_default();
  g_assert (display != NULL);
  num_screens = gdk_display_get_n_screens (display);
  for (i = 0 ; i < num_screens; ++i) {
    WnckScreen *screen;
    screen = wnck_screen_get (i);
    /* Connect a callback to the window opened event in libwnck */
    g_signal_connect (screen, "window_opened", (GCallback)window_opened_cb, NULL);
    if (apply_to_existing) wnck_screen_force_update (screen);
  }

}

/*
 * Dedicated to Vicky.
 */
int main(int argc, char **argv) {
  poptContext popt;
  int rc;

  /* Initialise i18n */
  bindtextdomain (GETTEXT_PACKAGE, DEVILSPIE_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Initialise GTK+ */
  gtk_init(&argc, &argv);

  /* Now parse the rest of the arguments */
  popt = poptGetContext(NULL, argc, (const char**)argv, options, 0);
  while ((rc = poptGetNextOpt(popt)) > 0) {}
  if (rc != -1) {
    g_printerr("%s: %s\n", poptBadOption(popt, 0), poptStrerror(rc));
    return 1;
  }

  /* And finally do the flurb type initialisation */
  flurb_init();

  /* If there are no more options, use $HOME/.devilspie.xml */
  if (poptPeekArg (popt) == NULL) {
    char *filename;
    filename = g_strdup_printf("%s/%s", g_get_home_dir(), ".devilspie.xml");
    load_configuration(filename);
    g_free(filename);
  } else {
    const char *filename;
    /* Otherwise parse every remaining argument on the command line as configuration file */
    while ((filename = poptGetArg(popt)) != NULL) {
      load_configuration(filename);
    }
  }
  poptFreeContext (popt);

  init_screens();

  /* Go go go! */
  gtk_main();
  return 0;
}
