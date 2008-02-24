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
 * Transform parameters into a string and concat them
 */
static char * internal_str(ESExp *f, int argc, ESExpResult **argv,
			   Context *c)
{
  char *s_arg0, *s_other_args, *result = NULL;

  if (argc < 1)
    return NULL;

  switch (argv[0]->type) {
  case ESEXP_RES_BOOL:
    s_arg0 = g_strdup((argv[0]->value.bool ? "TRUE" : "FALSE"));
    break;

  case ESEXP_RES_INT:
    s_arg0 = g_strdup_printf("%d", argv[0]->value.number);
    break;

  case ESEXP_RES_TIME:
    {
      char buf[256];
      struct tm time;
      localtime_r(&argv[0]->value.time, &time);
      strftime(buf, sizeof(buf), "%c", &time);
      s_arg0 = g_strdup(buf);
      break;
    }

  case ESEXP_RES_STRING:
    s_arg0 = g_strdup(argv[0]->value.string);
    break;

  case ESEXP_RES_ARRAY_PTR:
    s_arg0 = g_strdup_printf(_("(array pointer: %p)"),
			     argv[0]->value.ptrarray);
    break;

  case ESEXP_RES_UNDEFINED:
  default:
    g_printerr(_("Cannot convert element into a string"));
    return NULL;
    break;
  }

  s_other_args = internal_str(f, argc-1, argv+1, c);

  if (s_arg0 && s_other_args)
    {
      result = g_strdup_printf("%s%s", s_arg0, s_other_args);
      g_free(s_arg0);
      g_free(s_other_args);
    }
  else if (s_arg0)
    result = s_arg0;
  else if (s_other_args)
    result = s_other_args;

  return result;
}

/**
 * Print args (without trailing \n).
 */
ESExpResult *func_print(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  char * s = internal_str(f, argc, argv, c);
  if (! s)
    return e_sexp_result_new_bool (f, FALSE);

  g_print("%s", s);
  g_free(s);

  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Print args (with trailing \n).
 */
ESExpResult *func_println(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  char * s = internal_str(f, argc, argv, c);
  if (! s)
    return e_sexp_result_new_bool (f, FALSE);

  g_print("%s\n", s);
  g_free(s);

  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Transform parameters into a string and concat them.
 */
ESExpResult *func_str(ESExp *f, int argc, ESExpResult **argv, Context *c)
{
  ESExpResult *r;
  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = internal_str(f, argc, argv, c);
  if (! r->value.string)
    r->value.string = g_strdup("");
  return r;
}

/**
 * Transform the integer parameter into an unsigned hexadecimal string
 * (with 0x prefix).
 */
ESExpResult *func_hex(ESExp *f, int argc, ESExpResult **argv, Context *c)
{
  ESExpResult *r;

  if (argc != 1 || argv[0]->type != ESEXP_RES_INT)
    return e_sexp_result_new_bool (f, FALSE); 

  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = g_strdup_printf("0x%x", (unsigned)argv[0]->value.number);
  return r;
}


/**
 * Set position + size of current window.
 */
ESExpResult *func_geometry(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  gint xoffset, yoffset, width, height;
  int retmask, new_xoffset, new_yoffset;
  unsigned int new_width, new_height;
  WnckScreen *screen;

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

  screen      = wnck_window_get_screen (c->window);
  if (retmask & XNegative)
    new_xoffset = wnck_screen_get_width(screen) + new_xoffset - new_width;
  if (retmask & YNegative)
    new_yoffset = wnck_screen_get_height(screen) + new_yoffset - new_height;

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
  int xoffset, yoffset, window_width, window_height, workspace_width, workspace_height;
  WnckScreen *screen;
  WnckWorkspace *workspace;

  /* Read in window geometry */
  wnck_window_get_geometry (c->window, NULL, NULL, &window_width, &window_height);

  /* Read in workspace geometry */
  screen = wnck_window_get_screen (c->window);
  
  workspace = wnck_screen_get_active_workspace (screen);
  if (workspace == NULL)
    workspace = wnck_screen_get_workspace (screen, 0);
  if (workspace == NULL) {
    g_printerr (_("Cannot get workspace\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  workspace_width = wnck_workspace_get_width (workspace);
  workspace_height = wnck_workspace_get_height (workspace);

  /* Calculate offset for upper left corner */
  xoffset = (workspace_width - window_width) / 2;
  yoffset = (workspace_height - window_height) / 2;

  /* Try to set new position.. */
  my_wnck_error_trap_push ();
  XMoveWindow (gdk_display,
               wnck_window_get_xid (c->window),
               xoffset, yoffset);

  if (my_wnck_error_trap_pop ()) {
    g_printerr (_("Centering '%s' failed\n"), argv[0]->value.string);
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
 * Make the current window not fullscreen.
 */

ESExpResult *func_unfullscreen(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_set_fullscreen (c->window, FALSE);
  if (debug) g_printerr(_("Unsetting fullscreen\n"));
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
 * Un-maximise the current window.
 */
ESExpResult *func_unmaximize(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_unmaximize (c->window);
  if (debug) g_printerr(_("Un-maximising\n"));
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
 * Make the current window stick to all viewports.
 */
ESExpResult *func_stick(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_stick (c->window);
  if (debug) g_printerr(_("Setting sticky\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Unstick the window from viewports.
 */
ESExpResult *func_unstick(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  wnck_window_unstick (c->window);
  if (debug) g_printerr(_("Unsetting sticky\n"));
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
 * Change current workspace, counting from 1
 */
ESExpResult *func_change_workspace(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  WnckScreen *screen;
  WnckWorkspace *workspace;
  int num;
  GTimeVal timestamp;

  if (argc != 1 || argv[0]->type != ESEXP_RES_INT) {
    g_printerr(_("change_workspace expects a single integer argument\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  num = argv[0]->value.number;

  screen = wnck_window_get_screen(c->window);
  /* Adjust for 0-offset in workspaces list */
  workspace = wnck_screen_get_workspace(screen, num-1);
  if (!workspace) {
    g_warning(_("Workspace number %d does not exist"), num);
    return e_sexp_result_new_bool (f, FALSE);
  }
  g_get_current_time(&timestamp);
  wnck_workspace_activate(workspace, timestamp.tv_sec);

  if (debug) g_printerr(_("Switching workspace to %d\n"), num);
  return e_sexp_result_new_bool (f, TRUE);
}

/**
 * Move the window to a specific viewport number, counting from 1.
 */
ESExpResult *func_set_viewport(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  WnckScreen *screen;
  int num, x, y, width, height, viewport_start;

  if (argc != 1 || argv[0]->type != ESEXP_RES_INT) {
    g_printerr(_("set_viewport expects a single integer argument\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  num = argv[0]->value.number;

  if (num <= 0) {
    g_printerr(_("set_viewport expects an integer greater than 0\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  screen = wnck_window_get_screen(c->window);

  wnck_window_get_geometry(c->window, &x, &y, &width, &height);

  viewport_start = my_wnck_get_viewport_start(c->window);
  if (viewport_start < 0) {
    g_printerr(_("could not find current viewport\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }
  
  x = ((num - 1) * wnck_screen_get_width (screen)) - viewport_start + x;

  my_wnck_error_trap_push ();
  XMoveResizeWindow (gdk_display,
                     wnck_window_get_xid (c->window),
                     x, y, width, height);
  if (my_wnck_error_trap_pop ()) {
    g_printerr(_("Setting viewport failed\n"));
    return e_sexp_result_new_bool (f, FALSE);
  }
  
  if (debug) g_printerr(_("Changing viewport to %d\n"), num);
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

static void
set_decorations (Context *c, gboolean decorate)
{
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
  hints.decorations = decorate ? 1 : 0;

  /* Set Motif hints, most window managers handle these */
  XChangeProperty(GDK_DISPLAY(), wnck_window_get_xid (c->window),
                  my_wnck_atom_get ("_MOTIF_WM_HINTS"), 
                  my_wnck_atom_get ("_MOTIF_WM_HINTS"), 32, PropModeReplace, 
                  (unsigned char *)&hints, PROP_MOTIF_WM_HINTS_ELEMENTS);

  /* Apart from OpenBox, which doesn't respect it changing after mapping.
     Instead it has this workaround. */
  my_wnck_change_state (my_wnck_window_get_xscreen(c->window),
                        wnck_window_get_xid(c->window), !decorate,
                        my_wnck_atom_get ("_OB_WM_STATE_UNDECORATED"), 0);

}

/**
 * Remove the window manager decorations from the current window.
 */
ESExpResult *func_undecorate(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  my_wnck_error_trap_push ();

  set_decorations (c, FALSE);

  if (my_wnck_error_trap_pop () != 0) {
    g_printerr(_("Removing decorations failed"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  if (debug) g_printerr(_("Removed decorations\n"));
  return e_sexp_result_new_bool (f, TRUE);
}


/**
 * Add the window manager decorations to the current window.
 */
ESExpResult *func_decorate(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  my_wnck_error_trap_push ();
  
  set_decorations (c, TRUE);

  if (my_wnck_error_trap_pop () != 0) {
    g_printerr(_("Adding decorations failed"));
    return e_sexp_result_new_bool (f, FALSE);
  }

  if (debug) g_printerr(_("Added decorations\n"));
  return e_sexp_result_new_bool (f, TRUE);
}

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

/**
 * Change the opacity level (as integer in 0..100) of the current
 * window (returns boolean).
 */
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

/**
 * Execute a command in the foreground (returns command output as
 * string, or FALSE on error). Command is given as a single string, or
 * as a series of strings (similar to execl).
 */
ESExpResult *func_spawn_sync(ESExp *f, int argc, ESExpResult **argv, Context *c)
{
  gboolean spawn_result;
  gint exit_status = 0;
  gchar * stdout_contents = NULL;
  GError * errs = NULL;
  ESExpResult *r;

  if (argc < 1)
    return e_sexp_result_new_bool (f, FALSE);

  if (argc == 1) {
    if (debug) g_printerr(_("spawn_sync(string)\n"));
    if (argv[0]->type != ESEXP_RES_STRING) {
      g_printerr(_("spawn_sync: Command parameter is not a string"));
      return e_sexp_result_new_bool (f, FALSE);
    }
    
    spawn_result = g_spawn_command_line_sync(argv[0]->value.string,
					     & stdout_contents, NULL,
					     & exit_status, & errs);
  }
  else {
    /* Build spawn arguments */
    gchar ** spawn_args = g_new(char*, argc+1);
    int i;

    if (debug) g_printerr(_("spawn_sync(list)\n"));

    if (! spawn_args)
      return e_sexp_result_new_bool (f, FALSE);

    for (i = 0 ; i < argc ; i ++) {
      if (argv[i]->type != ESEXP_RES_STRING) {
	g_printerr(_("spawn_sync: Command parameter %d is not a string"), i);
	g_free(spawn_args);
	return e_sexp_result_new_bool (f, FALSE);
      }
      
      spawn_args[i] = argv[i]->value.string;
    }
    spawn_args[i] = NULL;

    spawn_result = g_spawn_sync(NULL, spawn_args, NULL, G_SPAWN_SEARCH_PATH,
				NULL, NULL, &stdout_contents, NULL,
				& exit_status, & errs);
    g_free(spawn_args);
  }

  if (! spawn_result) {
    if (errs && errs->message)
      g_printerr(_("Error spawning child process (fg): %s\n"), errs->message);
    else
      g_printerr(_("Error spawning child process (fg).\n"));
  }
  if (exit_status)
    g_printerr(_("Warning: child process returned %d\n"), exit_status);

  if (! stdout_contents)
      return e_sexp_result_new_bool (f, spawn_result);

  r = e_sexp_result_new(f, ESEXP_RES_STRING);
  r->value.string = stdout_contents;
  return r;
}

/**
 * Execute a command in the background (returns boolean). Command is
 * given as a single string, or as a series of strings (similar to
 * execl).
 */
ESExpResult *func_spawn_async(ESExp *f, int argc, ESExpResult **argv, Context *c)
{
  gboolean spawn_result;
  GError * errs = NULL;

  if (argc < 1)
    return e_sexp_result_new_bool (f, FALSE);

  if (argc == 1) {
    if (debug) g_printerr(_("spawn_async(string)\n"));
    if (argv[0]->type != ESEXP_RES_STRING) {
      g_printerr(_("spawn_async: Command parameter is not a string"));
      return e_sexp_result_new_bool (f, FALSE);
    }
    
    spawn_result = g_spawn_command_line_async(argv[0]->value.string, & errs);
  }
  else {
    /* Build spawn arguments */
    gchar ** spawn_args = g_new(char*, argc+1);
    int i;
    GPid child_pid;

    if (debug) g_printerr(_("spawn_async(list)\n"));

    if (! spawn_args)
      return e_sexp_result_new_bool (f, FALSE);

    for (i = 0 ; i < argc ; i ++) {
      if (argv[i]->type != ESEXP_RES_STRING) {
	g_printerr(_("spawn_async: Command parameter %d is not a string"), i);
	g_free(spawn_args);
	return e_sexp_result_new_bool (f, FALSE);
      }
      
      spawn_args[i] = argv[i]->value.string;
    }
    spawn_args[i] = NULL;

    spawn_result = g_spawn_async(NULL, spawn_args, NULL, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, &child_pid, & errs);
    g_free(spawn_args);

    if (spawn_result && debug) g_printerr(_("Spawned pid %u (bg)\n"),
					  child_pid);
  }

  if (! spawn_result) {
    if (errs && errs->message)
      g_printerr(_("Error spawning child process (bg): %s\n"), errs->message);
    else
      g_printerr(_("Error spawning child process (bg).\n"));
  }
  
  return e_sexp_result_new_bool (f, spawn_result);
}

/**
 * Quit
 */
ESExpResult *func_quit(ESExp *f, int argc, ESExpResult **argv, Context *c) {
  g_main_loop_quit (loop);
  if (debug) g_printerr(_("Quiting...\n"));
  return e_sexp_result_new_bool (f, TRUE);
}
