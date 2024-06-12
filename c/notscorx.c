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

void
token (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{
   // Check headers

   j_t rx = j_find (cgi, "formdata");

               j_store_string(tx, "access_token", "TODO");
            j_store_string(tx, "token_type", "bearer");
            j_store_string(tx, "scope", "default");
            j_store_int(tx, "expires_in", 3600);
}

void
directory (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{
   // Check headers

   j_t rx = j_find (cgi, "formdata");

}

void
letterbox (SQL * sqlp, int tester, j_t cgi, FILE * rxe, j_t tx, FILE * txe)
{
   // Check headers

   j_t rx = j_find (cgi, "formdata");

}

int
main (int argc, const char *argv[])
{
   sqldebug = 1;
   SQL sql;
   sql_safe_connect (&sql, NULL, NULL, NULL, "notsco", 0, NULL, 0);
   sql_transaction (&sql);
   // Errors
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
               token (&sql, tester, cgi, rxe, tx, txe);
            else
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
               tester = atoi (sql_colz (res, "ID"));
               // TODO expiry?

            }
            sql_free_result (res);
            if (!tester)
               status = totscoerror (tx, txe, 401, 0, 900901, NULL, "Invalid Credentials", NULL);
            else if (!strcmp (script, "/directory/v1/entry"))
               directory (&sql, tester, cgi, rxe, tx, txe);
            else if (!strcmp (script, "/letterbox/v1/post"))
               letterbox (&sql, tester, cgi, rxe, tx, txe);
            else
               fail ("Incorrect path for API", 500);
         }
      } else
         fail ("Unknown Host header", 500);
   }

   // Log
   fclose (txe);
   fclose (rxe);
   // TODO

   // Return
   printf ("Status: %d\r\n", status);
   printf ("Content-Type: application/json\r\n");
   printf ("\r\n");
   j_err (j_write (tx, stdout));
   j_err (j_write_pretty (cgi, stderr));        // TODO debug
   j_err (j_write_pretty (tx, stderr)); // TODO debug
   free (txerror);
   free (rxerror);
   j_delete (&tx);
   j_delete (&cgi);
   sql_safe_commit (&sql);
   sql_close (&sql);
   return 0;
}
