// NOTSCO Library

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <err.h>
#include <sqllib.h>
#include <ajlcurl.h>
#include "notscolib.h"
#include "notscosyntax.h"

static long long
ms (void)
{
   struct timeval tv = { 0 };
   struct timezone tz = { 0 };
   gettimeofday (&tv, &tz);
   return (long long) tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

void
notscotx (SQL * sqlp, int tester, j_t tx)
{                               // Send message
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   if (sql_fetch_row (res))
   {
      CURL *curl = curl_easy_init ();
      const char *bearer = NULL;
      const char *auth = sql_colz (res, "auth");
      if (strcmp (auth, "APIKEY"))
      {
         curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
         curl_easy_setopt (curl, CURLOPT_USERAGENT, "Synapse-PT-HttpComponents-NIO");   // As per TOTSCO
         int secs = 0;
         if (j_time (sql_colz (res, "expiry")) > time (0))
            bearer = sql_col (res, "bearer");
         if (!bearer)
         {                      // New token
            int try = 0;
            while (!bearer && try < 5)
            {
               if (try++)
                  sleep (5);
               long status = 0;
               long long t = 0;
               char *txerror = NULL;
               size_t txlen = 0;
               FILE *txe = open_memstream (&txerror, &txlen);
               char *rxerror = NULL;
               size_t rxlen = 0;
               FILE *rxe = open_memstream (&rxerror, &rxlen);
               j_t rx = j_create ();
               j_t tx = j_create ();
               char *er = NULL;
               const char *url = sql_col (res, "tokenurl");
               const char *clientsecret = sql_colz (res, "farclientsecret");
               const char *clientid = sql_colz (res, "farclientid");
               sql_safe_query (sqlp, "INSERT INTO `log` SET `ID`=0");   // Get ID in advance to ensure correct order if other end sends reply before completing
               long id = sql_insert_id (sqlp);
               if (!url || !*url)
                  fprintf (txe, "No token URL defined");
               else if (!*clientsecret)
                  fprintf (txe, "No client secret defined");
               else if (*clientid)
               {                // Get bearer
                  j_store_string (tx, "grant_type", "client_credentials");
                  if (!strcmp (auth, "OAUTH2Scope"))
                     j_store_string (tx, "scope", "full");
                  char *valid = NULL;
                  asprintf (&valid, "%s:%s", clientid, clientsecret);
                  t = ms ();
                  er = j_curl (J_CURL_POST | J_CURL_BASIC, curl, tx, rx, valid, "https://%s", url);
                  t = ms () - t;
                  free (valid);
                  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status);
                  if (er)
                     fprintf (rxe, "%s\n", er);
                  else
                  {
                     const char *tokentype = j_get (rx, "token_type");
                     if (!tokentype)
                        fprintf (rxe, "Missing token_type\n");
                     if (strcasecmp (tokentype, "Bearer"))
                        fprintf (rxe, "Expected \"token_type\":\"Bearer\" (is \"%s\")", tokentype);
                     const char *expiresin = j_get (rx, "expires_in");
                     if (!expiresin)
                        fprintf (rxe, "No expires_in\n");
                     else if ((secs = atoi (expiresin)) < 10)
                        fprintf (rxe, "Really short expiry (%d)\n", secs);
                     else if (secs > 86400)
                        fprintf (rxe, "Really long expiry (%d)\n", secs);
                     const char *token = j_get (rx, "access_token");
                     if (!token)
                        fprintf (rxe, "Missing access_token");
                     else
                        bearer = strdupa (token);
                  }
                  char *ct = NULL;
                  if (curl_easy_getinfo (curl, CURLINFO_CONTENT_TYPE, &ct))
                     ct = NULL;
                  if (!ct)
                     fprintf (rxe, "No Content-Type on response, expecting application/json\n");
                  else if (strncasecmp (ct, "application/json", 16) || (ct[16] && ct[16] != ';'))
                     fprintf (rxe, "Content-Type is %s, expected application/json\n", ct);
               }
               if (t > 3000)
                  fprintf (txe, "One Touch Switch Message Delivery Policies v1.0: Total response time greater than 3s (%lldms)\n",
                           t);
               fclose (txe);
               fclose (rxe);
               if (*clientid)
               {
                  char *txt = j_formdata (tx);
                  char *rxt = NULL;
                  if (j_isstring (rx))
                     rxt = strdup (j_val (rx));
                  else
                     rxt = j_write_pretty_str (rx);
                  sql_safe_query_f (sqlp,
                                    "UPDATE `log` SET `ms`=%lld,`tester`=%d,`status`=%ld,`description`='Sent OAUTH2 token request',`rxerror`=%#s,`tx`=%#s,`txerror`=%#s WHERE `ID`=%ld",
                                    t, tester, status, *rxerror ? rxerror : NULL, txt, *txerror ? txerror : NULL, id);
                  if (sql_query_f (sqlp, "UPDATE `log` SET `rx`=%#s WHERE `ID`=%ld", j_isnull (rx) ? NULL : rxt, id))
                     sql_safe_query_f (sqlp, "UPDATE `log` SET `rx`=%#s WHERE `ID`=%ld",
                                       "Unable to store in database, may be too long or bad UTF-8", id);
                  free (rxt);
                  free (txt);
               }
               if (!j_isnull (rx))
                  er = NULL;    // Give up
               j_delete (&rx);
               j_delete (&tx);
               free (er);
               if (!er)
                  break;
            }
            if (bearer)
               sql_safe_query_f (sqlp, "UPDATE `tester` SET `bearer`=%#s,`expiry`=%#T WHERE `ID`=%d", bearer, time (0) + secs - 10,
                                 tester);
         }
      }
      const char *routing = j_get (tx, "envelope.routingID");
      const char *description = j_get (tx, "envelope.test");
      if (description)
      {
         description = strdupa (description);
         j_free (j_find (tx, "envelope.test"));
      } else
         description = routing;
      int try = 0;
      while (try < 5)
      {
         if (try++)
            sleep (5);
         long status = 0;
         long long t = 0;
         char *txerror = NULL;
         size_t txlen = 0;
         FILE *txe = open_memstream (&txerror, &txlen);
         char *rxerror = NULL;
         size_t rxlen = 0;
         FILE *rxe = open_memstream (&rxerror, &rxlen);
         char *er = NULL;
         const char *apikey = sql_colz (res, "apikey");
         const char *url = sql_col (res, "apiurl");
         syntaxcheck (tx, txe);
         if (!url || !*url)
            fprintf (txe, "No API URL defined. Not sending request.\n");
         if (!(!strcmp (auth, "APIKEY") ? *apikey : *bearer))
            fprintf (txe, "We have no authorisation to send this request. Not sending.\n");
         j_t rx = j_create ();
         sql_safe_query (sqlp, "INSERT INTO `log` SET `ID`=0"); // Get ID in advance to ensure correct order if other end sends reply before completing
         long id = sql_insert_id (sqlp);
         if (url && *url && (!strcmp (auth, "APIKEY") ? *apikey : *bearer))
         {
            // Send message
            t = ms ();
            if (!strcmp (auth, "APIKEY"))
               er = j_curl (J_CURL_SEND | J_CURL_APIKEY, curl, tx, rx, apikey, "https://%s", url);
            else
               er = j_curl (J_CURL_SEND, curl, tx, rx, bearer, "https://%s", url);
            t = ms () - t;
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status);
            if (er)
               fprintf (rxe, "%s\n", er);
            char *ct = NULL;
            if (curl_easy_getinfo (curl, CURLINFO_CONTENT_TYPE, &ct))
               ct = NULL;
            if (!ct)
               fprintf (rxe, "No Content-Type on response, expecting application/json\n");
            else if (strncasecmp (ct, "application/json", 16) || (ct[16] && ct[16] != ';'))
               fprintf (rxe, "Content-Type is %s, expected application/json\n", ct);
            responsecheck (status, rx, rxe);
         }
         if (t > 3000)
            fprintf (txe, "One Touch Switch Message Delivery Policies v1.0: Total response time greater than 3s (%lldms)\n", t);
         fclose (rxe);
         fclose (txe);
         char *txt = j_write_pretty_str (tx);
         char *rxt = NULL;
         if (j_isstring (rx))
            rxt = strdup (j_val (rx));
         else
            rxt = j_write_pretty_str (rx);
         sql_safe_query_f (sqlp,
                           "UPDATE `log` SET `tester`=%d,`description`='Sent %#S',`tx`=%#s,`txerror`=%#s,`status`=%ld,`ms`=%lld,`rxerror`=%#s WHERE `ID`=%ld",
                           tester, description, j_isnull (tx) ? *txerror ? "" : NULL : txt, *txerror ? txerror : NULL, status, t,
                           j_isnull (rx) ? *rxerror ? "" : NULL : *rxerror ? rxerror : NULL, id);
         if (sql_query_f (sqlp, "UPDATE `log` SET `rx`=%#s WHERE `ID`=%ld", j_isnull (rx) ? NULL : rxt, id))
            sql_safe_query_f (sqlp, "UPDATE `log` SET `rx`=%#s WHERE `ID`=%ld",
                              "Unable to store in database, may be too long or bad UTF-8", id);
         if (routing && tester)
            sql_safe_query_f (sqlp,
                              "INSERT INTO `scorecard` SET `tester`=%d,`routing`=%#s,`status`=%#s,`direction`='Tx',`first`=NOW(),`last`=NOW(),`count`=1 ON DUPLICATE KEY UPDATE `count`=`count`+1,`last`=NOW()",
                              tester, routing, *rxerror || (!strstr (routing, "Failure") && *txerror) ? "ERRORS" : "CLEAN");
         free (rxt);
         free (txt);
         if (!j_isnull (rx))
            er = NULL;          // Give up
         j_delete (&rx);
         free (er);
         if (!er)
            break;
      }
      curl_easy_cleanup (curl);
   }
   sql_free_result (res);
}

j_t
notscoreply (SQL * sqlp, j_t rx, j_t tx, const char *type)
{                               // Make reply message, set tx, return payload
   char *routing = strdupa (j_get (rx, "envelope.routingID"));
   if (!routing)
      errx (1, "No routingID");
   char *request = strstr (routing, "Request");
   if (!request)
      errx (1, "Not a Request: %s", routing);
   if (!type)
      routing = strdup ("messageDeliveryFailure");
   else
      asprintf (&routing, "%.*s%s", (int) (request - routing), routing, type);
   const char *v = NULL;
   j_t envelope = j_store_object (tx, "envelope");
   j_t source = j_store_object (envelope, "source");
   j_store_string (source, "type", "RCPID");
   j_store_string (source, "identity", !type ? "TOTSCO" : j_get (rx, "envelope.destination.identity")); // Note, example is TOTSCO, but surely it should be original destination identity regardless?
   if (type && (v = j_get (rx, "envelope.destination.correlationID")) && *v)
      j_store_string (source, "correlationID", v);
   else
   {                            // Make one up...
      SQL_RES *u = sql_safe_query_store_f (sqlp, "SELECT UUID() AS U");
      if (sql_fetch_row (u))
         j_store_string (source, "correlationID", sql_col (u, "U"));
      sql_free_result (u);
   }
   j_t destination = j_store_object (envelope, "destination");
   j_store_string (destination, "type", "RCPID");
   j_store_string (destination, "identity", j_get (rx, "envelope.source.identity"));
   if ((v = j_get (rx, "envelope.source.correlationID")))
      j_store_string (destination, "correlationID", v);
   j_store_string (envelope, "routingID", routing);
   j_t payload = j_store_object (tx, routing);
   free (routing);
   return payload;
}

void
notscofailure (SQL * sqlp, int tester, j_t rx, int code, const char *sor)
{                               // Fault code return message to other RCP
   j_t t = j_create ();
   j_t payload = notscoreply (sqlp, rx, t, code >= 9000 ? NULL : "Failure");
   if (sor && *sor)
      j_store_stringf (payload, "switchOrderReference", sor);
   char codes[20];
   sprintf (codes, "%d", code);
   j_store_string (payload, code >= 9000 ? "code" : "faultCode", codes);
   const char *msg (void)
   {
      switch (code)
      {
#define e(c,e) case c:return e;
#include "notscoerrors.m"
      default:
         return "Unspecified error";
      }
   }
   j_store_string (payload, code >= 9000 ? "text" : "faultText", msg ());
   if (code >= 9000)
      j_store_string (payload, "severity", "failure");
   j_t audit = j_store_array (j_find (t, "envelope"), "auditData");
   void add (const char *name, const char *value)
   {
      if (!value)
         return;
      j_t e = j_append_object (audit);
      j_store_string (e, "name", name);
      j_store_string (e, "value", value);
   }
   if (code >= 9000)
   {
      add ("originalDestinationType", j_get (rx, "envelope.destination.type"));
      add ("originalDestination", j_get (rx, "envelope.destination.identity"));
      add ("originalRoutingID", j_get (rx, "envelope.routingID"));
   }
   add ("faultCode", codes);
   notscotx (sqlp, tester, t);
}
