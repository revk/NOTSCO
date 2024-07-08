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
   if (sid && *sid)
      j_store_string (source, "correlationID", sid);
   j_t destination = j_store_object (envelope, "destination");
   j_store_string (destination, "type", "RCPID");
   if ((v = sql_colz (res, "rcpid")) && *v)
      j_store_string (destination, "identity", v);
   if (did && *did)
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
   sql_safe_query_f (sqlp, "REPLACE INTO `pending` SET `correlation`=%#s,`tester`=%#s,`request`=%#s", sid, sql_colz (res, "ID"),
                     "residentialSwitchMatch");
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

void
residentialSwitchOrderRequests (SQL * sqlp, SQL_RES * res, j_t tx, const char *routing, const char *rcpid, const char *sor,
                                const char *dated)
{
   char *sid = NULL;
   SQL_RES *u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS `U`");
   if (sql_fetch_row (u))
      sid = strdupa (sql_colz (u, "U"));
   sql_free_result (u);
   j_t payload = makemessage (res, tx, routing, sid, NULL);
   j_store_string (j_find (tx, "envelope.source"), "identity", rcpid);
   if (*sor)
      j_store_string (payload, "switchOrderReference", sor);
   if (dated && *dated && !strstr (routing, "Cancellation"))
   {
      j_store_string (payload, strstr (routing, "Trigger") ? "activationDate" : "plannedSwitchDate", dated);
      sql_safe_query_f (sqlp, "UPDATE `sor` SET `dated`=%#s WHERE `tester`=%#s AND `issuedby`='THEM' AND `sor`=%#s AND `rcpid`=%#s",
                        dated, sql_colz (res, "ID"), sor, rcpid);
   }
   char *suffix = strstr (routing, "Request");
   if (suffix)
      sql_safe_query_f (sqlp, "REPLACE INTO `pending` SET `correlation`=%#s,`tester`=%#s,`request`=%#.*s", sid,
                        sql_colz (res, "ID"), (int) (suffix - routing), routing);
}

void
makebad (SQL * sqlp, SQL_RES * res, j_t tx, const char *send)
{
   const char *routing = "residentialSwitchMatchRequest";
   if (!strcmp (send, "BadRouting"))
      routing = "Silly";
   char *sid = NULL;
   SQL_RES *u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS `U`");
   if (sql_fetch_row (u))
      sid = strdupa (sql_colz (u, "U"));
   sql_free_result (u);
   j_t payload = makemessage (res, tx, routing, sid, NULL);
   j_store_string (j_find (tx, "envelope"), "test", send);
   if (!strcmp (send, "BadEnvelope1"))
      j_store_string (j_find (tx, "envelope.source"), "type", "Silly");
   else if (!strcmp (send, "BadEnvelope2"))
      j_store_string (j_find (tx, "envelope.destination"), "type", "Silly");
   else if (!strcmp (send, "BadEnvelope3"))
      j_store_string (j_find (tx, "envelope.destination"), "identity", "NOTYOU");
   else if (!strcmp (send, "BadEnvelope4"))
      j_free (j_find (tx, "envelope.destination.identity"));
   else if (!strcmp (send, "BadEnvelope5"))
      j_free (j_find (tx, "envelope.destination.type"));
   else if (!strcmp (send, "BadEnvelope6"))
      j_free (j_find (tx, "envelope.source.type"));
   else if (!strncmp (send, "TestMatch", 9))
   {
      int test = atoi (send + 9);
      char *val;
      if (*(val = sql_colz (res, "brand")))
         j_store_string (payload, "grcpBrandName", val);
      if (*(val = sql_colz (res, "surname")))
         j_store_string (payload, "name", val);
      if (*(val = sql_colz (res, "account")))
         j_store_string (payload, "account", val);
      j_t address = j_store_object (payload, "address");
      if (*(val = sql_colz (res, "posttown")))
         j_store_string (address, "postTown", val);
      if (*(val = sql_colz (res, "postcode")))
         j_store_string (address, "postCode", val);
      j_t lines = j_store_array (address, "addressLines");
      if (test == 1)
      {
         j_append_string (lines, "Post Office");
         j_append_string (lines, "75A South Street");
         j_store_string (address, "postTown", "BISHOP'S STORTFORD");
         j_store_string (address, "postCode", "CM23 3AL");
      } else if (test == 2)
      {
         j_append_string (lines, "Royal Parks Office");
         j_append_string (lines, "Store Yard");
         j_append_string (lines, "St. James's Park");
         j_store_string (address, "postTown", "LONDON");
         j_store_string (address, "postCode", "SW1A 3BJ");
      } else
      {
         j_append_string (lines, "Prime Minister & First Lord Of The Treasury");
         j_append_string (lines, "10 Downing Street");
         j_store_string (address, "postTown", "LONDON");
         j_store_string (address, "postCode", "SW1A 2AA");
      }
      if (test == 3)
         j_store_string (payload, "name", "Starmér");
      if (test == 4)
         j_store_string (payload, "name", "O'Conner");
      if (test == 5)
         j_store_string (payload, "name", "Tables`ls`");
      if (test == 6)
         j_store_string (payload, "name", "\"Shakespear");
      if (test == 7)
         j_store_string (payload, "name", "\\Slasher");
      if (test == 8)
         j_store_string (payload, "account", "1'234");
      if (test == 9)
         j_store_string (payload, "account", "1234`ls`");
      if (test == 10)
         j_store_string (payload, "account", "\"1234");
      if (test == 11)
         j_store_string (payload, "account", "\\1234");
      if (test == 12)
         j_store_string (payload, "account", "1234\n");
      j_t services = j_store_array (payload, "services");
      j_t s = j_append_object (services);
      j_store_string (s, "serviceType", "IAS");
      j_store_string (s, "action", "cease");
      sql_safe_query_f (sqlp, "REPLACE INTO `pending` SET `correlation`=%#s,`tester`=%#s,`request`='residentialSwitchMatch'",
                        j_get (tx, "envelope.source.correlationID"), sql_colz (res, "ID"));
   }
}

int
main (int argc, const char *argv[])
{
   int errorchoice = 0;
   int tester = 0;
   const char *send = NULL;
   const char *sor = NULL;
   const char *dated = NULL;
   const char *rcpid = NULL;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"send", 's', POPT_ARG_STRING, &send, 0, "Send", "MessageType"},
         {"sor", 0, POPT_ARG_STRING, &sor, 0, "Switch Order Reference", "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"},
         {"dated", 0, POPT_ARG_STRING, &dated, 0, "Dated", "YYYY-MM-DD"},
         {"rcpid", 0, POPT_ARG_STRING, &rcpid, 0, "RCPID", "XXXX"},
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
      else if (!strcmp (send, "residentialSwitchOrderRequest") || !strcmp (send, "residentialSwitchOrderUpdateRequest")
               || !strcmp (send, "residentialSwitchOrderTriggerRequest")
               || !strcmp (send, "residentialSwitchOrderCancellationRequest"))
         residentialSwitchOrderRequests (&sql, res, tx, send, rcpid, sor, dated);
      else
         makebad (&sql, res, tx, send);
      notscotx (&sql, tester, tx);
   }
   sql_free_result (res);
   sql_close (&sql);
   poptFreeContext (optCon);
   return 0;
}
