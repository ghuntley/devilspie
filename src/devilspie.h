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

#endif /* DEVILSPIE_H */
