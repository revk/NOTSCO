// NOTSCO Rx

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <err.h>
#include <jcgi.h>
#include <sqllib.h>
#include "notscolib.h"

int debug = 0;

int
notscoerror (j_t tx, int res, int ecode, int code, const char *text, const char *message, const char *description)
{                               // Direct error to TOTSCO hub
   if (ecode)
      j_store_int (tx, "errorCode", ecode);     // spec says integer but examples are a string with numeric content
   if (text)
      j_store_string (tx, "errorText", text);
   if (code)
      j_store_int (tx, "code", code);   // spec says integer but examples are a string with numeric content
   if (message)
      j_store_string (tx, "message", message);
   if (description)
      j_store_string (tx, "description", description);  // spec says Description but examples sy descrioptipn
   return res;
}

int
token (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{                               // Token request
   const char *method = j_get (cgi, "info.request_method");
   if (!method)
      fprintf (rxe, "No method?!\n");
   else if (strcasecmp (method, "POST"))
      fprintf (rxe, "Expecting POST (is %s)\n", method);
   const char *ct = j_get (cgi, "header.Content-Type");
   if (!ct)
      fprintf (rxe, "No Content-Type\n");
   else if (strcmp (ct, "application/x-www-form-urlencoded"))
      fprintf (rxe, "Expected Content-Type: application/x-www-form-urlencoded (is %s)\n", ct);
   j_t rx = j_find (cgi, "formdata");
   const char *gt = j_get (rx, "grant_type");
   if (!gt)
      fprintf (rxe, "No grant_type\n");
   else if (strcmp (gt, "client_credentials"))
      fprintf (rxe, "Expected grant_type=client_credentials (is %s)\n", gt);
   int secs = 3600;
   sql_safe_query_f (sqlp, "INSERT INTO `auth` SET `tester`=%d,`bearer`=to_base64(random_bytes(45)),`expiry`=%#T", tester,
                     time (0) + secs);
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `auth` WHERE `ID`=%d", sql_insert_id (sqlp));
   if (sql_fetch_row (res))
      j_store_string (tx, "access_token", sql_colz (res, "bearer"));
   else
      fprintf (rxe, "Failed to allocate token\n");
   sql_free_result (res);
   j_store_string (tx, "token_type", "bearer");
   j_store_string (tx, "scope", "default");
   j_store_int (tx, "expires_in", 3600);
   return 200;
}

int
directory (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{                               // Directory request
   const char *method = j_get (cgi, "info.request_method");
   if (!method)
      fprintf (rxe, "No method?!\n");
   else if (strcasecmp (method, "GET"))
      fprintf (rxe, "Expecting GET (is %s)\n", method);
   j_t rx = j_find (cgi, "formdata");
   const char *lt = j_get (rx, "listType");
   if (!lt)
      fprintf (rxe, "listType not specified\n");
   else if (strcmp (lt, "RCPID"))
      fprintf (rxe, "Expected listType=RCPID (is %s)\n", lt);
   const char *identity = j_get (rx, "identity");       // can be all, or a list (how is it a list?)
   if (identity && !strcmp (identity, "all"))
      identity = NULL;
   j_t list = j_store_array (tx, "list");
   list = j_append_object (list);
   j_store_string (list, "listType", lt);
   list = j_store_array (list, "identity");
   void add (const char *rcpid, const char *company, const char *support, const char *sales, int active)
   {
      if (identity && !strstr (identity, rcpid))
         return;
      j_t e = j_append_object (list);
      j_store_string (e, "id", rcpid);
      if (company && *company)
         j_store_string (e, "name", company);
      j_t j = j_store_array (e, "processSupport");
      j = j_append_object (j);
      j_store_string (j, "process", "OTS");
      j_store_string (j, "status", active ? "Active" : "Suspend");
      if (sales || support)
      {
         j = j_store_array (e, "resource");
         if (support)
         {
            j_t r = j_append_object (j);
            j_store_string (r, "name", "customerAssistURL");
            j_store_string (r, "type", "URL");
            j_store_string (r, "value", support);
         }
         if (sales)
         {
            j_t r = j_append_object (j);
            j_store_string (r, "name", "salesAssistURL");
            j_store_string (r, "type", "URL");
            j_store_string (r, "value", sales);
         }
      }
   }
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `directory`");
   while (sql_fetch_row (res))
      add (sql_col (res, "rcpid"), sql_col (res, "company"), sql_col (res, "support"), sql_col (res, "sales"),
           *sql_colz (res, "active") == 'Y');
   sql_free_result (res);
   res = sql_safe_query_store_f (sqlp, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   if (sql_fetch_row (res))
      add (sql_col (res, "rcpid"), sql_col (res, "company"), NULL, NULL, 1);
   sql_free_result (res);
   return 200;
}

void
residentialSwitchMatchRequest (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t payload)
{
   int code = 0;
   // Sanity checking e.g. envelope, etc
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   if (sql_fetch_row (res))
   {
      const char *reply = sql_colz (res, "matchresponse");
      if (!strcmp (reply, "Failure"))
         code = atoi (sql_colz (res, "matcherror"));
      else if (strcmp (reply, "None"))
      {
         j_t t = j_create ();
         j_t payload = notscoreply (rx, t, "Confirmation");
         SQL_RES *u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS U");
         if (sql_fetch_row (u))
            j_store_string (j_find (t, "envelope.source"), "correlationID", sql_col (u, "U"));
         sql_free_result (u);
         j_t implications = j_store_array (payload, "implicationsSent");
         const char *sentto = sql_colz (res, "sentto");
         if (*sentto)
         {
            j_t j = j_append_object (implications);
            j_store_string (j, "sentMethod", strchr (sentto, '@') ? "email" : "sms");
            j_store_string (j, "sentTo", sentto);
            j_store_datetime (j, "sentBy", time (0));
         } else
         {
            j_t j = j_append_object (implications);
            j_store_string (j, "sentMethod", "1st class post");
            j_store_datetime (j, "sentBy", time (0));
         }
         j_t match = j_store_object (payload, "matchResult");
         void add (int alt)
         {
            char *sor = NULL;
            u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS U");
            if (sql_fetch_row (u))
               sor = strdupa (sql_col (u, "U"));
            sql_free_result (u);
            j_store_string (match, "switchOrderReference", sor);
            sql_safe_query_f (sqlp,
                              "INSERT INTO `sor` SET `ID`=0,`tester`=%d,`sor`=%#s,`issuedby`='US',`rcpid`=%#s,`nearid`=%#s,`farid`=%#s ON DUPLICATE KEY UPDATE `created`=NOW()",
                              tester, sor, j_get (t, "envelope.source.identity"), j_get (t, "envelope.source.correlationID"),
                              j_get (t, "envelope.destination.correlationID"));
            j_t services = j_store_array (match, "services");
            const char *cupid = sql_colz (res, "cupid");
            const char *no = sql_colz (res, "networkoperator");
            const char *sn = sql_colz (res, "servicename");
            const char *dn = sql_colz (res, "dn");
            const char *partialdn = sql_colz (res, "partialdn");
            const char *alid = sql_colz (res, "alid");
            const char *ontref = sql_colz (res, "ontref");
            const char *ontport = sql_colz (res, "ontport");
            void add (j_t j, const char *tag, const char *val)
            {
               if (!val || !*val)
                  return;
               j = j_append_object (j);
               j_store_string (j, "identifierType", tag);
               j_store_string (j, "identifier", val);
            }
            if (*alid || *ontref || *ontport)
            {                   // IAS
               j_t j = j_append_object (services);
               j_store_string (j, "serviceType", "IAS");
               j_store_string (j, "switchAction", "ServiceFound");
               j = j_store_array (j, "serviceIdentifiers");
               add (j, "ONTReference", ontref);
               if (atoi (ontport))
                  add (j, "PortNumber", ontport);
               add (j, "AccessLineId", alid);
               add (j, "ServiceInformation", sn);
               add (j, "NetworkOperator", no);
            }
            if (*dn || *partialdn)
            {                   // NBICS
               j_t j = j_append_object (services);
               j_store_string (j, "serviceType", "NBICS");
               j_store_string (j, "switchAction", alt ? "OptionToCease" : "OptionToRetain");
               j = j_store_array (j, "serviceIdentifiers");
               add (j, "CUPID", cupid);
               add (j, "DN", dn);
               add (j, "PartialDN", partialdn);
            }
         }
         add (0);
         if (strchr (reply, '+'))
         {
            j_t alt = j_store_array (payload, "alternativeSwitchOrders");
            match = j_store_object (j_append_object (alt), "matchResult");
            add (1);
         }
         notscotx (sqlp, tester, t);
      }
   }
   if (code)
      notscofailure (sqlp, tester, rx, code, NULL);
}

void
checksor (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t payload, const char *sor, const char *rcpid, int rev)
{
   if (!sor || !rcpid)
      return;
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `sor` WHERE `tester`=%d AND `issuedby`=%#s AND `sor`=%#s", tester,
                                          rev ? "THEM" : "US", sor);
   if (!sql_fetch_row (res))
      fprintf (rxe, "\"switchOrderReference\" is not one we were expecting.\n");
   else
   {
      const char *val;
      const char *farid = j_get (rx, "envelope.source.correlationID");
      const char *nearid = j_get (rx, "envelope.destination.correlationID");
      if (rcpid && strcmp (rcpid, (val = sql_colz (res, "rcpid"))))
         fprintf (rxe, "The RCPID is not what we expected for this switch order (expected %s).\n", val);
      if (farid && strcmp (farid, (val = sql_colz (res, "farid"))))
         fprintf (rxe, "The source correlationID is not what we expected for this switch order (expected %s).\n", val);
      if (nearid && strcmp (nearid, (val = sql_colz (res, "nearid"))))
         fprintf (rxe, "The source correlationID is not what we expected for this switch order (expected %s).\n", val);
   }
   sql_free_result (res);
}

void
progressRequest (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t payload, const char *routing)
{
   int code = 0;
   const char *rcpid = j_get (rx, "envelope.destination.identity");
   const char *sor = j_get (payload, "switchOrderReference");
   checksor (sqlp, tester, rx, rxe, payload, sor, rcpid, 0);
   int base = 1200;
   const char *newstatus = "confirmed";
   if (strstr (routing, "Update"))
   {
      base = 1300;
      newstatus = "updated";
   } else if (strstr (routing, "Trigger"))
   {
      base = 1400;
      newstatus = "triggered";
   } else if (strstr (routing, "Cancellation"))
   {
      base = 1500;
      newstatus = "cancelled";
   }
   time_t now = time (0);
   SQL_RES *res =
      sql_safe_query_store_f (sqlp, "SELECT * FROM `sor` WHERE `tester`=%d AND `issuedby`='US' AND `sor`=%#s AND `rcpid`=%#s",
                              tester, sor, rcpid);
   int process (void)
   {
      const char *status = sql_colz (res, "status");
      const char *date = NULL;
      if (j_time (sql_colz (res, "created")) < now - 86400 * 31)
         return base + 2;
      if ((base == 1200 || base == 1300) && !(date = j_get (payload, "plannedSwitchDate")))
         return base + 3;
      if (base == 1400 && !(date = j_get (payload, "activationDate")))
         return base + 3;
      if (!strcmp (status, "triggered"))
         return base + 4;
      if (!strcmp (status, "cancelled"))
         return base + 5;
      if (base != 1200 && !strcmp (status, "new"))
         return base + 6;
      if (base == 1200 && strcmp (status, "new"))
         return base + 13;
      return 0;
   }
   if (!sql_fetch_row (res))
      code = base + 1;
   else
      code = process ();
   sql_free_result (res);
   if (!code)
   {
      j_t t = j_create ();
      j_t payload = notscoreply (rx, t, "Confirmation");
      j_store_string (payload, "switchOrderReference", sor);
      j_store_string (payload, "status", newstatus);
      notscotx (sqlp, tester, t);
      sql_safe_query_f (sqlp, "UPDATE `sor` SET `status`=%#s WHERE `tester`=%d AND `issuedby`='US' AND `sor`=%#s AND `rcpid`=%#s",
                        newstatus, tester, sor, rcpid);
   } else
      notscofailure (sqlp, tester, rx, code, sor);
}

int
progressConfirmation (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t tx, FILE * txe, j_t payload, const char *routing)
{
   const char *rcpid = j_get (rx, "envelope.destination.identity");
   const char *sor = j_get (payload, "switchOrderReference");
   const char *status = j_get (payload, "status");
   checksor (sqlp, tester, rx, rxe, payload, sor, rcpid, 1);
   if (rcpid && sor && status)
      sql_safe_query_f (sqlp, "UPDATE `sor` SET `status`=%#s WHERE `tester`=%d AND `rcpid`=%#s AND `sor`=%#s AND `issuedby`='THEM'",
                        status, tester, rcpid, sor);
   return 202;
}

int
progressFailure (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t tx, FILE * txe, j_t payload, const char *routing)
{
   const char *rcpid = j_get (rx, "envelope.destination.identity");
   const char *sor = j_get (payload, "switchOrderReference");
   checksor (sqlp, tester, rx, rxe, payload, sor, rcpid, 1);
   return 202;
}

int
residentialSwitchMatchConfirmation (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t tx, FILE * txe, j_t payload)
{
   void check (j_t j)
   {
      j = j_find (j, "matchResult");
      if (!j)
         return;
      const char *sor = j_get (j, "switchOrderReference");
      if (sor)
         sql_safe_query_f (sqlp,
                           "INSERT INTO `sor` SET `ID`=0,`tester`=%d,`sor`=%#s,`issuedby`='THEM',`rcpid`=%#s,`nearid`=%#s,`farid`=%#s ON DUPLICATE KEY UPDATE `created`=NOW()",
                           tester, sor, j_get (rx, "envelope.destination.identity"), j_get (rx,
                                                                                            "envelope.destination.correlationID"),
                           j_get (rx, "envelope.source.correlationID"));
   }
   check (payload);
   for (j_t a = j_first (j_find (payload, "alternativeSwitchOrders")); a; a = j_next (a))
      check (a);
   return 202;
}

int
residentialSwitchMatchFailure (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t tx, FILE * txe, j_t payload)
{
   return 202;
}

int
messageDeliveryFailure (SQL * sqlp, int tester, j_t rx, FILE * rxe, j_t tx, FILE * txe, j_t payload)
{
   return 202;
}

int
letterbox (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{                               // Handle posted message
   int status = 0;
   const char *method = j_get (cgi, "info.request_method");
   if (!method)
      fprintf (rxe, "No method?!\n");
   else if (strcasecmp (method, "POST"))
      fprintf (rxe, "Expecting POST (is %s)\n", method);
   const char *ct = j_get (cgi, "header.Content-Type");
   if (!ct)
      fprintf (rxe, "No Content-Type\n");
   else if (strcmp (ct, "application/json"))
      fprintf (rxe, "Expected Content-Type: application/json (is %s)\n", ct);
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   sql_fetch_row (res);
   const char *us = strdupa (sql_colz (res, "rcpid"));
   int delay = atoi (sql_colz (res, "delay"));
   sql_free_result (res);
   j_t rx = j_find (cgi, "formdata");
   syntaxcheck (rx, rxe);
   j_t envelope = j_find (rx, "envelope");
   if (!envelope)
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Missing envelope");
   j_t source = j_find (envelope, "source");
   if (!source)
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Missing source");
   if (strcmp (j_get (source, "type") ? : "", "RCPID"))
      return notscoerror (tx, 400, 9002, 0, "Unknown or invalid source Type.", NULL, NULL);
   const char *rcpid = j_get (source, "identity");
   if (!rcpid)
      return notscoerror (tx, 400, 9003, 0, "Unknown or invalid source Id.", NULL, NULL);
   if (!*us)
      fprintf (rxe, "We do not have an RCPID set\n");
   else if (strcmp (rcpid, us))
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Incorrect source RCPID");
   j_t destination = j_find (envelope, "destination");
   if (!destination)
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Missing destination");
   if (strcmp (j_get (destination, "type") ? : "", "RCPID"))
      return notscoerror (tx, 400, 9000, 0, "Unknown or invalid destination Type.", NULL, NULL);
   rcpid = j_get (destination, "identity");
   if (!rcpid || !*rcpid)
      return notscoerror (tx, 400, 9001, 0, "Unknown of invalid destination.", NULL, NULL);
   if (*us && !strcmp (rcpid, us))
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Talking to ourselves");
   res = sql_safe_query_store_f (sqlp, "SELECT * FROM `directory` WHERE `rcpid`=%#s", rcpid);
   if (!sql_fetch_row (res))
      status = notscoerror (tx, 400, 9001, 0, "Unknown of invalid destination.", NULL, NULL);
   sql_free_result (res);
   if (status)
      return status;
   const char *routing = j_get (envelope, "routingID");
   if (!routing)
      return notscoerror (tx, 400, 9012, 0, "Unknown of invalid routing ID.", NULL, NULL);
   // Check payload
   const char *t = routing;
   if (strcmp (t, "messageDeliveryFailure"))
   {
      if (t && !strncmp (t, "residentialSwitch", 17))
         t += 17;
      else
         t = NULL;
      if (t && !strncmp (t, "Match", 5))
         t += 5;
      else if (t && !strncmp (t, "Order", 5))
      {
         t += 5;
         if (!strncmp (t, "Update", 6))
            t += 6;
         else if (!strncmp (t, "Trigger", 7))
            t += 7;
         else if (!strncmp (t, "Cancellation", 12))
            t += 12;
      } else
         t = NULL;
      if (t && strcmp (t, "Request") && strcmp (t, "Failure") && strcmp (t, "Confirmation"))
         t = NULL;
   }
   if (!t)
      return notscoerror (tx, 400, 9012, 0, "Unknown of invalid routing ID.", NULL, NULL);
   j_t payload = j_find (rx, routing);
   if (!payload)
      return notscoerror (tx, 400, 0, 400, NULL, "Bad Request", "Missing payload");
   // Handle specific messages
   if (!status)
   {
      if (strstr (routing, "Request"))
      {                         // Request handling, send reply
         if (!fork ())
         {
            if (!sqldebug)
            {
               close (0);
               close (1);
               close (2);
               setpgid (0, 0);
            }
            if (delay)
               sleep (delay);
            SQL sql;
            sql_safe_connect (&sql, NULL, NULL, NULL, "notsco", 0, NULL, 0);
            if (!strcmp (routing, "residentialSwitchMatchRequest"))
               residentialSwitchMatchRequest (&sql, tester, rx, rxe, payload);
            else if (strstr (routing, "Request"))
               progressRequest (&sql, tester, rx, rxe, payload, routing);
            sql_close (&sql);
            _exit (0);
         }
      } else
      {                         // Response handling
         if (!strcmp (routing, "residentialSwitchMatchConfirmation"))
            status = residentialSwitchMatchConfirmation (sqlp, tester, rx, rxe, tx, txe, payload);
         else if (!strcmp (routing, "residentialSwitchMatchFailure"))
            status = residentialSwitchMatchFailure (sqlp, tester, rx, rxe, tx, txe, payload);
         else if (!strcmp (routing, "messageDeliveryFailure"))
            status = messageDeliveryFailure (sqlp, tester, rx, rxe, tx, txe, payload);
         else if (strstr (routing, "Confirmation"))
            status = progressConfirmation (sqlp, tester, rx, rxe, tx, txe, payload, routing);
         else if (strstr (routing, "Failure"))
            status = progressFailure (sqlp, tester, rx, rxe, tx, txe, payload, routing);
      }
   }
   return status ? : 202;
}

int
main (int argc, const char *argv[])
{
   SQL sql;
   sql_safe_connect (&sql, NULL, NULL, NULL, "notsco", 0, NULL, 0);
   // Errors
   const char *description = "?";
   char *txerror = NULL;
   size_t txlen = 0;
   FILE *txe = open_memstream (&txerror, &txlen);
   char *rxerror = NULL;
   size_t rxlen = 0;
   FILE *rxe = open_memstream (&rxerror, &rxlen);
   int status = 0;
   int tester = 0;
   j_t tx = j_create ();
   void fail (const char *e, int s)
   {                            // Simple failure
      if (!e)
         return;
      status = notscoerror (tx, s, 0, 0, e, NULL, NULL);
   }
   j_t cgi = j_create ();
 fail (j_cgi (cgi, medium:1), 500);
   if (!j_find (cgi, "info.https"))
      fail ("Not https", 500);
   else
   {
      const char *auth = j_get (cgi, "header.Authorization");
      const char *host = j_get (cgi, "header.Host");
      const char *script = j_get (cgi, "info.script_name");
      if (!host)
         fail ("No Host header", 500);
      else if (!script)
         fail ("No script_name", 500);
      else if (!auth || !*auth)
      {
         status =
            notscoerror (tx, 401, 0, 900902, NULL, "Missing Credentials",
                         "Invalid Credentials. Make sure your API invocation call has a header: 'Authorization : Bearer ACCESS_TOKEN' or 'Authorization : Basic ACCESS_TOKEN'");
         if (status && status / 100 != 2)
            fprintf (txe, "HTTP responded with status %d\n", status);
      } else if (!strncmp (host, "otshub-token.", 13))
      {
         if (strncasecmp (auth, "Basic ", 6))
            status = notscoerror (tx, 401, 0, 401, NULL, "Expecting Basic auth", NULL);
         else
         {
            unsigned char *user = NULL,
               *c = NULL;
            ssize_t len = j_base64d (auth + 6, &user);
            if (len > 0)
            {
               int p = 0;
               for (p = 0; p < len && user[p] && user[p] != ':'; p++);
               if (p < len && user[p] == ':')
               {
                  c = user + p;
                  *c++ = 0;
                  SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `tester` WHERE `clientid`=%#s", user);
                  if (sql_fetch_row (res))
                  {
                     tester = atoi (sql_colz (res, "ID"));
                     if (strncmp (sql_colz (res, "clientsecret"), (char *) c, len - p - 1))
                        c = 0;  // Not matched
                  } else
                     c = 0;
                  sql_free_result (res);
               }
            }
            free (user);
            if (!c)
               status = notscoerror (tx, 401, 0, 900901, NULL, "Invalid Credentials", "Access failure");
            if (!strcmp (script, "/oauth2/token"))
            {
               description = "OAUTH2 token request";
               if (!status)
                  status = token (&sql, tester, cgi, rxe, tx, txe);
            } else
               fail ("Incorrect path for token", 500);
         }
         if (status && status / 100 != 2)
            fprintf (txe, "HTTP responded with status %d\n", status);
      } else if (!strncmp (host, "otshub.", 7))
      {
         if (strncasecmp (auth, "Bearer ", 7))
            status = notscoerror (tx, 401, 0, 401, NULL, "Expecting Bearer auth", NULL);
         else
         {
            auth += 7;
            SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `auth` WHERE `bearer`=%#s", auth);
            if (sql_fetch_row (res))
            {
               tester = atoi (sql_colz (res, "tester"));
               if (j_time (sql_colz (res, "expiry")) < time (0))
                  fprintf (rxe, "Using expired bearer\n");
               else
                  auth = NULL;  // OK
            }
            sql_free_result (res);
            if (auth)
               status = notscoerror (tx, 401, 0, 900901, NULL, "Invalid Credentials", "Access failure");
            if (!strcmp (script, "/directory/v1/entry"))
            {
               description = "directory API request";
               if (!status)
                  status = directory (&sql, tester, cgi, rxe, tx, txe);
            } else if (!strcmp (script, "/letterbox/v1/post"))
            {
               description = j_get (cgi, "formdata.envelope.routingID") ? : "letterbox API post";
               if (!status)
                  status = letterbox (&sql, tester, cgi, rxe, tx, txe);
            } else
               fail ("Incorrect path for API", 500);
         }
         responsecheck (status, tx, txe);
      } else
         fail ("Unknown Host header", 500);
   }
   if (!status)
      fail ("Not processed", 500);
   // Log
   fclose (txe);
   fclose (rxe);
   const char *ip = j_get (cgi, "info.remote_addr");
   if (!tester)
   {                            // Guess tester from IP
      SQL_RES *res =
         sql_safe_query_store_f (&sql, "SELECT DISTINCT `tester` FROM `log` WHERE `ip`=%#s AND `tester` IS NOT NULL LIMIT 2", ip);
      if (sql_fetch_row (res))
      {
         int t = atoi (sql_colz (res, "tester"));
         if (!sql_fetch_row (res))
            tester = t;
      }
   }
   j_t rx = j_find (cgi, "formdata");
   char *rxt = NULL;
   if (!strcmp (j_get (cgi, "info.request_method") ? : "", "GET") ||
       !strcmp (j_get (cgi, "header.Content-Type") ? : "", "application/x-www-form-urlencoded"))
      rxt = j_formdata (rx);
   else
      rxt = j_write_pretty_str (rx);
   char *txt = j_write_pretty_str (tx);
   sql_safe_query_f (&sql,
                     "INSERT INTO `log` SET `ID`=0,`ts`=NOW(),`status`=%d,`ip`=%#s,`description`='Received %#S',`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                     status, ip, description, j_isnull (rx) ? NULL : rxt, *rxerror ? rxerror : NULL,
                     j_isnull (tx) ? NULL : txt, *txerror ? txerror : NULL);
   if (tester)
      sql_safe_query_f (&sql, "UPDATE `log` SET `tester`=%d WHERE `ID`=%d", tester, sql_insert_id (&sql));
   free (rxt);
   free (txt);
   // Return
   printf ("Status: %d\r\n", status);
   printf ("Content-Type: application/json\r\n");
   printf ("\r\n");
   j_err (j_write (tx, stdout));
   if (sqldebug)
   {
      if (rxerror && *rxerror)
         fprintf (stderr, "Rx errors:\n%s", rxerror);
      j_err (j_write_pretty (cgi, stderr));
      if (txerror && *txerror)
         fprintf (stderr, "Tx errors:\n%s", txerror);
      j_err (j_write_pretty (tx, stderr));
   }
   free (txerror);
   free (rxerror);
   j_delete (&tx);
   j_delete (&cgi);
   sql_close (&sql);
   return 0;
}
