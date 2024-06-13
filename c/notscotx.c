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
   int tester = 0;
   const char *send = NULL;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"send", 's', POPT_ARG_STRING, &send, 0, "Send", "MessageType"},
         {"tester", 't', POPT_ARG_INT, &tester, 0, "Tester", "N"},
         {"error-choice", 'e', POPT_ARG_NONE, &errorchoice, 0, "Error choice list"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (!send && poptPeekArg (optCon))
         send = poptGetArg (optCon);
      if (poptPeekArg (optCon) || (!errorchoice && (!tester || !send)))
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
   sqldebug = 1;
   SQL sql;
   sql_safe_connect (&sql, NULL, NULL, NULL, "notsco", 0, NULL, 0);
   SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   if (!sql_fetch_row (res))
      errx (1, "Unknown tester (%d)", tester);
   if (send)
   {
      j_t tx = j_create ();

      notscotx (&sql, tester, tx);
   }
   sql_free_result (res);
   sql_close (&sql);

   poptFreeContext (optCon);
   return 0;
}
