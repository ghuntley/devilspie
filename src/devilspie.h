#ifndef DEVILSPIE_H
#define DEVILSPIE_H

/*
 * Shared header file.
 *
 * Defines types and the public API within Devil's Pie.
 */

#include "glib.h"

/* Declare that I know WNCK is a library in flux */
#define WNCK_I_KNOW_THIS_IS_UNSTABLE

/* Declare that life is good */
#define LIFE_IS_GOOD TRUE

void load_configuration(const char *filename) ;
void save_configuration(const char* filename) ;

#if 0
typedef enum {
    T_FALSE = FALSE,
    T_TRUE = TRUE,
    T_UNSET
} tristate;
#endif

/* I18n mojo */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) gettext (String)
#define N_(String) (String)
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#endif /* DEVILSPIE_H */
