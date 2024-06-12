// NOTSCO Rx

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <jcgi.h>
#include <sqllib.h>
#include "notscolib.h"

int debug = 0;

int
totscoerror (j_t tx, FILE * txe, int res, int ecode, int code, const char *text, const char *message, const char *description)
{                               // Direct error to TOTSCO hub
   fprintf (txe, "Status %d: error %d %s\n", res, ecode ? : code, text ? : message ? : description);
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

int
letterbox (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{
   // Check headers

   j_t rx = j_find (cgi, "formdata");
   return 200;
}

int
main (int argc, const char *argv[])
{
   sqldebug = 1;
   SQL sql;
   sql_safe_connect (&sql, NULL, NULL, NULL, "notsco", 0, NULL, 0);
   sql_transaction (&sql);
   // Errors
   const char *description = "Received";
   char *txerror = NULL;
   size_t txlen = 0;
   FILE *txe = open_memstream (&txerror, &txlen);
   char *rxerror = NULL;
   size_t rxlen = 0;
   FILE *rxe = open_memstream (&rxerror, &rxlen);
   int status = 200;
   int tester = 0;
   j_t tx = j_create ();
   void fail (const char *e, int s)
   {                            // Simple failure
      if (!e)
         return;
      status = totscoerror (tx, txe, s, 0, 0, e, NULL, NULL);
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
         status =
            totscoerror (tx, txe, 401, 0, 900902, NULL, "Missing Credentials",
                         "Invalid Credentials. Make sure your API invocation call has a header: 'Authorization : Bearer ACCESS_TOKEN' or 'Authorization : Basic ACCESS_TOKEN'");
      else if (!strncmp (host, "otshub-token.", 13))
      {
         if (strncasecmp (auth, "Basic ", 6))
            status = totscoerror (tx, txe, 401, 0, 401, NULL, "Expecting Basic auth", NULL);
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
               status = totscoerror (tx, txe, 401, 0, 900901, NULL, "Invalid Credentials", NULL);
            else if (!strcmp (script, "/oauth2/token"))
            {
               description = "Received OAUTH2 token request";
               status = token (&sql, tester, cgi, rxe, tx, txe);
            } else
               fail ("Incorrect path for token", 500);
         }
      } else if (!strncmp (host, "otshub.", 7))
      {
         if (strncasecmp (auth, "Bearer ", 7))
            status = totscoerror (tx, txe, 401, 0, 401, NULL, "Expecting Bearer auth", NULL);
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
               status = totscoerror (tx, txe, 401, 0, 900901, NULL, "Invalid Credentials", NULL);
            else if (!strcmp (script, "/directory/v1/entry"))
            {
               description = "Received directory API request";
               status = directory (&sql, tester, cgi, rxe, tx, txe);
            } else if (!strcmp (script, "/letterbox/v1/post"))
            {
               description = "Received letterbox API post";
               status = letterbox (&sql, tester, cgi, rxe, tx, txe);
            } else
               fail ("Incorrect path for API", 500);
         }
      } else
         fail ("Unknown Host header", 500);
   }

   // Log
   fclose (txe);
   fclose (rxe);
   char *rxt = j_write_str (j_find (cgi, "formdata"));
   char *txt = j_write_str (tx);
   sql_safe_query_f (&sql,
                     "INSERT INTO `log` SET `ID`=0,`ts`=NOW(),`ip`=%#s,`description`=%#s,`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                     j_get (cgi, "info.remote_addr"), description, rxt, *rxerror ? rxerror : NULL, txt, *txerror ? txerror : NULL);
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
   sql_safe_commit (&sql);
   sql_close (&sql);
   return 0;
}
