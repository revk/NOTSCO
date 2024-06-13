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

static j_t
makemessage (SQL_RES * res, j_t tx, const char *routing, const char *sid, const char *did)
{
   const char *v = NULL;
   j_t envelope = j_store_object (tx, "envelope");
   j_t source = j_store_object (envelope, "source");
   j_store_string (source, "type", "RCPID");
   if ((v = sql_colz (res, "fromrcpid")) && *v)
      j_store_string (source, "identity", v);
   if (sid)
      j_store_string (source, "correlationID", sid);
   j_t destination = j_store_object (envelope, "destination");
   j_store_string (destination, "type", "RCPID");
   if ((v = sql_colz (res, "rcpid")) && *v)
      j_store_string (destination, "identity", v);
   if (did)
      j_store_string (destination, "correlationID", did);
   j_store_string (envelope, "routingID", routing);
   return j_store_object (tx, routing);
}

void
residentialSwitchMatchRequest (SQL * sqlp, SQL_RES * res, j_t tx)
{
   const char *val = NULL;
   char *sid = NULL;
   SQL_RES *u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS `U`");
   if (sql_fetch_row (u))
      sid = strdupa (sql_colz (u, "U"));
   sql_free_result (u);
   j_t payload = makemessage (res, tx, "residentialSwitchMatchRequest", sid, NULL);
   if (*(val = sql_colz (res, "brand")))
      j_store_string (payload, "grcpBrandName", val);
   if (*(val = sql_colz (res, "surname")))
      j_store_string (payload, "name", val);
   if (*(val = sql_colz (res, "account")))
      j_store_string (payload, "account", val);
   j_t address = j_store_object (payload, "address");
   if (*(val = sql_colz (res, "uprn")))
      j_store_string (address, "uprn", val);
   j_t lines = j_store_array (address, "addressLines");
   if (*(val = sql_colz (res, "address1")))
      j_append_string (lines, val);
   if (*(val = sql_colz (res, "address2")))
      j_append_string (lines, val);
   if (*(val = sql_colz (res, "address3")))
      j_append_string (lines, val);
   if (*(val = sql_colz (res, "address4")))
      j_append_string (lines, val);
   if (*(val = sql_colz (res, "address5")))
      j_append_string (lines, val);
   if (*(val = sql_colz (res, "posttown")))
      j_store_string (address, "postTown", val);
   if (*(val = sql_colz (res, "postcode")))
      j_store_string (address, "postCode", val);
   j_t services = j_store_array (payload, "services");
   const char *pdn = sql_colz (res, "portdn");
   if (*pdn)
   {                            // NBICS only
      j_t s = j_append_object (services);
      j_store_string (s, "serviceType", "NBICS");
      j_store_string (s, "serviceIdentifier", pdn);
      j_store_string (s, "action", "port");
   } else
   {                            // IAS only
      j_t s = j_append_object (services);
      j_store_string (s, "serviceType", "IAS");
      j_store_string (s, "action", "cease");
      if (*(val = sql_colz (res, "circuit")))
         j_store_string (s, "serviceIdentifier", val);
   }
   const char *idn = sql_colz (res, "identifydn");
   if (*idn)
   {
      j_t s = j_append_object (services);
      j_store_string (s, "serviceType", "NBICS");
      j_store_string (s, "serviceIdentifier", idn);
      j_store_string (s, "action", "identify");
   }

}

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
      if (!strcmp (send, "residentialSwitchMatchRequest"))
         residentialSwitchMatchRequest (&sql, res, tx);
      notscotx (&sql, tester, tx);
   }
   sql_free_result (res);
   sql_close (&sql);
   poptFreeContext (optCon);
   return 0;
}
