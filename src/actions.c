/*
 * Copyright (C) 2005 Ross Burton <ross@burtonini.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <libwnck/application.h>
#include <libwnck/class-group.h>
#include <libwnck/workspace.h>
#include <libwnck/window.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "e-sexp.h"
#include "xutils.h"
#include "devilspie.h"
#include "actions.h"

/*
 * Actions to perform on windows.
 */

/**
 * Debugging function, outputs the current window's title, name, role and geometry.
 */
ESExpResult *func_debug(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  gint xoffset, yoffset, width, height;

  wnck_window_get_geometry(c->window, &xoffset, &yoffset, &width, &height);

  if (argc == 1 && argv[0]->type == ESEXP_RES_STRING)
    g_print("%s\n", argv[0]->value.string);

  g_print(_("Window Title: '%s'; Application Name: '%s'; Class: '%s'; Geometry: %dx%d+%d+%d\n"),
          wnck_window_get_name (c->window),
          wnck_application_get_name (wnck_window_get_application (c->window)),
          wnck_class_group_get_res_class (wnck_window_get_class_group (c->window)),
          width, height, xoffset, yoffset);
  
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Print args
 */
ESExpResult *func_print(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  int i;
  for (i = 0; i < argc; i++) {
    switch (argv[i]->type) {
    case ESEXP_RES_ARRAY_PTR:
      g_print (_("(array pointer: %p)\n"), argv[i]->value.ptrarray);
      break;
    case ESEXP_RES_BOOL:
      g_print (argv[i]->value.bool ? _("TRUE\n") : _("FALSE\n"));
      break;
    case ESEXP_RES_INT:
      g_print ("%d\n", argv[i]->value.number);
      break;
    case ESEXP_RES_TIME:
      {
        char buf[256];
        struct tm time;
        localtime_r(&argv[i]->value.time, &time);
        strftime(buf, sizeof(buf), "%c", &time);
        g_print ("time: %s", buf);
        break;
      }
    case ESEXP_RES_STRING:
      g_print ("%s\n", argv[i]->value.string);
      break;
    case ESEXP_RES_UNDEFINED:
      g_print (_("(undefined)"));
      break;
    }
  }
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Set position + size of current window.
 */
ESExpResult *func_geometry(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  gint xoffset, yoffset, width, height;
  int retmask, new_xoffset, new_yoffset;
  unsigned int new_width, new_height;

  if (argc < 1 || argv[0]->type != ESEXP_RES_STRING)
    return e_sexp_result_new_bool (f, FALSE); 

  /* read in old geom + parse param */
  wnck_window_get_geometry (c->window,
                            &xoffset, &yoffset, &width, &height);
  retmask = XParseGeometry (argv[0]->value.string,
                            &new_xoffset, &new_yoffset,
                            &new_width, &new_height);

  /* check which values to modify */
  new_xoffset = (retmask & XValue)      ? new_xoffset : xoffset;
  new_yoffset = (retmask & YValue)      ? new_yoffset : yoffset;
  new_width   = (retmask & WidthValue)  ? new_width   : width;
  new_height  = (retmask & HeightValue) ? new_height  : height;

  /* try to set new position.. */
  my_wnck_error_trap_push ();
  XMoveResizeWindow (gdk_display,
                     wnck_window_get_xid (c->window),
                     new_xoffset, new_yoffset,
                     new_width, new_height);

  if (my_wnck_error_trap_pop ()) {
    g_printerr(_("Setting geometry '%s' failed\n"),
               argv[0]->value.string);
    return e_sexp_result_new_bool (f, FALSE);
  }

  if (debug)
    g_printerr(_("Setting geometry '%s'\n"), argv[0]->value.string);

  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Center position of current window.
 */
ESExpResult *func_center(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  gint xoffset, yoffset, window_width, window_height,
    workspace_width, workspace_height;
  int new_xoffset, new_yoffset;

  /* read in window geometry */
  wnck_window_get_geometry (c->window,
                            &xoffset, &yoffset, &window_width, &window_height);

  /* read in workspace geometry */
  WnckScreen *screen;
  WnckWorkspace *workspace;
  screen           = wnck_window_get_screen (c->window);
  workspace        = wnck_screen_get_active_workspace (screen);
  workspace_width  = wnck_workspace_get_width  (workspace);
  workspace_height = wnck_workspace_get_height (workspace);

  /* calculate offset for upper left corner */
  new_xoffset = (workspace_width - window_width) / 2;
  new_yoffset = (workspace_height - window_height) / 2;

  /* try to set new position.. */
  my_wnck_error_trap_push ();
  XMoveWindow (gdk_display,
               wnck_window_get_xid (c->window),
               new_xoffset, new_yoffset);

  if (my_wnck_error_trap_pop ()) {
    g_printerr (_("Centering '%s' failed\n"),
               argv[0]->value.string);
    return e_sexp_result_new_bool (f, FALSE);
  }

  if (debug)
    g_printerr (_("Centering\n"));

  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Make the current window fullscreen.
 */
ESExpResult *func_fullscreen(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_set_fullscreen (c->window, TRUE);
  if (debug) g_printerr(_("Setting fullscreen\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Focus the current window.
 */
ESExpResult *func_focus(ESExp *f, int argc, ESExpResult **argv, Context *c) {
#if NEED_TIMESTAMPS
  wnck_window_activate (c->window, GDK_CURRENT_TIME);
#else
  wnck_window_activate (c->window);
#endif
  if (debug) g_printerr (_("Focusing\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Maximise the current window.
 */
ESExpResult *func_maximize(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_maximize (c->window);
  if (debug) g_printerr(_("Maximising\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Maximise vertically the current window.
 */
ESExpResult *func_maximize_vertically(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_maximize_vertically (c->window);
  if (debug) g_printerr(_("Maximising vertically\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Maximise horizontally the current window.
 */
ESExpResult *func_maximize_horizontally(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_maximize_horizontally (c->window);
  if (debug) g_printerr(_("Maximising horizontally\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Minimise the current window.
 */
ESExpResult *func_minimize(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_minimize (c->window);
  if (debug) g_printerr(_("Minimising\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Un-minimise (i.e. restore) the current window.
 */
ESExpResult *func_unminimize(ESExp *f, int argc, ESExpResult **argv, Context *c) {
#if NEED_TIMESTAMPS
  wnck_window_unminimize (c->window, GDK_CURRENT_TIME);
#else
  wnck_window_unminimize (c->window);
#endif
  if (debug) g_printerr(_("Un-minimising\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Shade ("roll up") the current window.
 */
ESExpResult *func_shade(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_shade (c->window);
  if (debug) g_printerr(_("Shaded\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Unshade ("roll down") the current window.
 */
ESExpResult *func_unshade(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_unshade (c->window);
  if (debug) g_printerr(_("Unshaded\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/** 
 * Close the current window.
 */
ESExpResult *func_close(ESExp *f, int argc, ESExpResult **argv, Context *c) {
#if NEED_TIMESTAMPS
  wnck_window_close (c->window, GDK_CURRENT_TIME);
#else
  wnck_window_close (c->window);
#endif
  if (debug) g_printerr(_("Closed\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Pin the current window to all workspaces.
 */
ESExpResult *func_pin(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_pin (c->window);
  if (debug) g_printerr(_("Setting pinned\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Unpin the current window from all workspaces.
 */
ESExpResult *func_unpin(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_unpin (c->window);
  if (debug) g_printerr(_("Unsetting pinned\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Move the window to a specific workspace number, counting from 1.
 */
ESExpResult *func_set_workspace(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  WnckScreen *screen;
  WnckWorkspace *workspace;
  int num;

  if (argc != 1 || argv[0]->type != ESEXP_RES_INT) {
    g_printerr(_("set_workspace expects a single integer argument\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  num = argv[0]->value.number;

  screen = wnck_window_get_screen(c->window);
  /* Adjust for 0-offset in workspaces list */
  workspace = wnck_screen_get_workspace(screen, num-1);
  if (!workspace) {
    g_warning(_("Workspace number %d does not exist"), num);
  }
  wnck_window_move_to_workspace(c->window, workspace);

  if (debug) g_printerr(_("Changing workspace to %d\n"), num);
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Remove the current window from the window list.
 */
ESExpResult *func_skip_tasklist(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_set_skip_tasklist (c->window, TRUE);
  if (debug) g_printerr(_("Skipping tasklist\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Remove the current window from the pager.
 */
ESExpResult *func_skip_pager(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_set_skip_pager (c->window, TRUE);
  if (debug) g_printerr(_("Skipping pager\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Set the current window to be above all normal windows.
 */
ESExpResult *func_above(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  my_wnck_change_state (my_wnck_window_get_xscreen(c->window),
                        wnck_window_get_xid(c->window),
                        TRUE,
                        my_wnck_atom_get ("_NET_WM_STATE_ABOVE"),
                        0);
  if (debug) g_printerr(_("Setting above\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Set the current window to be below all normal windows.
 */
ESExpResult *func_below(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  my_wnck_change_state (my_wnck_window_get_xscreen(c->window),
                        wnck_window_get_xid(c->window),
                        TRUE,
                        my_wnck_atom_get ("_NET_WM_STATE_BELOW"),
                        0);
  if (debug) g_printerr(_("Setting below\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Remove the window manager decorations from the current window.
 */
ESExpResult *func_undecorate(ESExp *f, int argc, ESExpResult **argv, Context *c) {
#define PROP_MOTIF_WM_HINTS_ELEMENTS 5
#define MWM_HINTS_DECORATIONS (1L << 1)
  struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long inputMode;
    unsigned long status;
  } hints = {0,};

  hints.flags = MWM_HINTS_DECORATIONS;
  hints.decorations = 0;

  my_wnck_error_trap_push ();
  
  /* Set Motif hints, most window managers handle these */
  XChangeProperty(GDK_DISPLAY(), wnck_window_get_xid (c->window),
                  my_wnck_atom_get ("_MOTIF_WM_HINTS"), 
                  my_wnck_atom_get ("_MOTIF_WM_HINTS"), 32, PropModeReplace, 
                  (unsigned char *)&hints, PROP_MOTIF_WM_HINTS_ELEMENTS);

  /* Apart from OpenBox, which doesn't respect it changing after mapping.
     Instead it has this workaround. */
  my_wnck_change_state (my_wnck_window_get_xscreen(c->window),
                        wnck_window_get_xid(c->window), TRUE,
                        my_wnck_atom_get ("_OB_WM_STATE_UNDECORATED"), 0);

  if (my_wnck_error_trap_pop () != 0) {
    g_printerr(_("Removing decorations failed"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  if (debug) g_printerr(_("Removed decorations\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

#if ! HAVE_SET_WINDOW_TYPE
/*
 * This function is only available in recent libwncks, so include it here if it
 * isn't present.
 */
static void my_wnck_window_set_window_type (WnckWindow *window, WnckWindowType wintype)
{
  Atom atom;

  g_return_if_fail (WNCK_IS_WINDOW (window));

  switch (wintype) {
  case WNCK_WINDOW_NORMAL:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_NORMAL");
    break;
  case WNCK_WINDOW_DESKTOP:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_DESKTOP");
    break;
  case WNCK_WINDOW_DOCK:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_DOCK");
    break;
  case WNCK_WINDOW_DIALOG:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_DIALOG");
    break;
  case WNCK_WINDOW_MODAL_DIALOG:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_MODAL_DIALOG");
    break;
  case WNCK_WINDOW_TOOLBAR:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_TOOLBAR");
    break;
  case WNCK_WINDOW_MENU:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_MENU");
    break;
  case WNCK_WINDOW_UTILITY:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_UTILITY");
    break;
  case WNCK_WINDOW_SPLASHSCREEN:
    atom = my_wnck_atom_get ("_NET_WM_WINDOW_TYPE_SPLASHSCREEN");
    break;
  default:
    return;
  }
  my_wnck_error_trap_push ();

  XChangeProperty (GDK_DISPLAY(), wnck_window_get_xid(window),
                   my_wnck_atom_get ("_NET_WM_WINDOW_TYPE"),
                   XA_ATOM, 32, PropModeReplace, (guchar *)&atom, 1);

  my_wnck_error_trap_pop ();
}
#define wnck_window_set_window_type(a, b) my_wnck_window_set_window_type(a, b)
#endif

/**
 * Set the window type of the current window.
 *
 * Accepted values are: normal, dialog, menu, toolbar, splashscreen, utility,
 * dock, desktop.
 */
ESExpResult *func_wintype(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  const char *str;
  WnckWindowType wintype;

  if (argc != 1 || argv[0]->type != ESEXP_RES_STRING) {
    g_printerr(_("wintype expects a single string argument\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  str = argv[0]->value.string;
  if (g_ascii_strcasecmp (str, "normal") == 0)
    wintype = WNCK_WINDOW_NORMAL;
  else if (g_ascii_strcasecmp (str, "dialog") == 0)
    wintype = WNCK_WINDOW_DIALOG;
  else if (g_ascii_strcasecmp (str, "menu") == 0)
    wintype = WNCK_WINDOW_MENU;
  else if (g_ascii_strcasecmp (str, "toolbar") == 0)
    wintype = WNCK_WINDOW_TOOLBAR;
  else if (g_ascii_strcasecmp (str, "splashscreen") == 0)
    wintype = WNCK_WINDOW_SPLASHSCREEN;
  else if (g_ascii_strcasecmp (str, "utility") == 0)
    wintype = WNCK_WINDOW_UTILITY;
  else if (g_ascii_strcasecmp (str, "dock") == 0)
    wintype = WNCK_WINDOW_DOCK;
  else if (g_ascii_strcasecmp (str, "desktop") == 0)
    wintype = WNCK_WINDOW_DESKTOP;
  else {
    g_printerr(_("Unknown window type '%s'"), str);
    return e_sexp_result_new_bool (f, FALSE);
  }

  wnck_window_set_window_type (c->window, wintype);

  if (debug) g_printerr(_("Set wintype\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

ESExpResult *func_opacity(ESExp *f, int argc, ESExpResult **argv, Context *c) {
	int opacity;
	unsigned int v;

	if (argc!=1 || argv[0]->type != ESEXP_RES_INT) {
		g_printerr(_("opacity expects a single integer argument\n"));
		return e_sexp_result_new_bool (f, FALSE);
	}
	opacity=argv[0]->value.number;
	if (opacity < 0 || opacity > 100) {
		g_printerr(_("opacity expects a single integer argument between 0 and 100\n"));
		return e_sexp_result_new_bool (f, FALSE);
	}
	my_wnck_error_trap_push ();
	v=0xffffffff/100*opacity;
	XChangeProperty (GDK_DISPLAY(), wnck_window_get_xid(c->window),
		my_wnck_atom_get ("_NET_WM_WINDOW_OPACITY"),
		XA_CARDINAL, 32, PropModeReplace, (guchar *)&v, 1);

	my_wnck_error_trap_pop ();
	return e_sexp_result_new_bool (f, TRUE);
}

