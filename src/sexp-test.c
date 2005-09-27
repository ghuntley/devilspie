#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include "devilspie.h"
#include "parser.h"
#include "e-sexp.h"

GList *sexps = NULL;
gboolean debug = FALSE;
Context context = {NULL};

int main(int argc, char **argv)
{
  int count = 0;
  GError *error = NULL;
  char *srcdir;
  const char *testpath, *name;
  GDir *dir;

  g_type_init ();

  srcdir = getenv("srcdir");
  if (!srcdir) srcdir = ".";
  testpath = g_build_filename (srcdir, "../tests", NULL);

  dir = g_dir_open (testpath, 0, &error);
  if (!dir) {
    g_printerr ("Cannot open %s: %s\n", testpath, error->message);
    g_error_free (error);
    return 1;
  }
  
  while ((name = g_dir_read_name (dir)) != NULL) {
    char *filepath;
    gboolean expected;
    ESExp *sexp;
    ESExpResult *result;
    
    if (!g_str_has_suffix (name, ".ds"))
      continue;
    
    if (g_str_has_suffix (name, "-true.ds"))
      expected = TRUE;
    else if (g_str_has_suffix (name, "-false.ds"))
      expected = TRUE;
    else {
      g_printerr ("Cannot determin result for %s\n", name);
      return 1;
    }
    
    filepath = g_build_filename (testpath, name, NULL);
    sexp = load_configuration_file (filepath);
    g_free (filepath);
    
    result = e_sexp_eval(sexp);
    if (result->type != ESEXP_RES_BOOL) {
      g_printerr("Invalid result type for test %s\n", name);
      return 1;
    }

    g_print(".");
    count++;
  }
  g_dir_close (dir);

  if (count) {
    g_print("\nSuccessfully completed %d tests\n", count);
    return 0;
  } else {
    g_print("Didn't find any tests\n");
    return 1;
  }
}
