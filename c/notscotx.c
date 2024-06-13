// NOTSCO Tx

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <sqllib.h>
#include <ajl.h>
#include "notscolib.h"

int debug = 0;

int
main (int argc, const char *argv[])
{
   int errorchoice = 0;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
//      {"string", 's', POPT_ARG_STRING, &string, 0, "String", "string"},
//      {"string-default", 'S', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &string, 0, "String", "string"},
         {"error-choice", 'e', POPT_ARG_NONE, &errorchoice, 0, "Error choice list"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
   }

   if (errorchoice)
   {
#define e(c,e) printf("<option value=\"%d\">%d: %s</option>",c,c,e);
#include "notscoerrors.m"
      return 0;
   }

   poptFreeContext (optCon);
   return 0;
}
