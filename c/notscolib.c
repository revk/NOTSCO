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

const char *
isuuid (const char *u)
{
   if (!u)
      return "NULL";
   if (!*u)
      return "Empty string";
   if (strlen (u) != 36)
      return "Wrong length";
   for (int i = 0; i < 36; i++)
      if ((i == 9 || i == 14 || i == 19 || i == 24) && u[i] != '-')
         return "Expected -";
      else if (!isxdigit (u[i]))
         return "Expecting hex character";
   return NULL;
}

void
notscotx (SQL * sqlp, int tester, j_t tx)
{                               // Send message
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `tester` WHERE `ID`=%d", tester);
   if (sql_fetch_row (res))
   {
      CURL *curl = curl_easy_init ();
      int secs = 0;
      const char *bearer = NULL;
      if (j_time (sql_colz (res, "expiry")) > time (0))
         bearer = sql_col (res, "bearer");
      if (!bearer)
      {                         // New token
         int try = 0;
         while (!bearer && try < 5)
         {
            if (try++)
               sleep (5);
            long status = 0;
            char *txerror = NULL;
            size_t txlen = 0;
            FILE *txe = open_memstream (&txerror, &txlen);
            char *rxerror = NULL;
            size_t rxlen = 0;
            FILE *rxe = open_memstream (&rxerror, &rxlen);
            j_t rx = j_create ();
            j_t tx = j_create ();
            char *er = NULL;
            const char *host = sql_col (res, "tokenhost");
            const char *clientid = sql_col (res, "farclientid");
            const char *clientsecret = sql_col (res, "farclientsecret");
            if (!host || !*host)
               fprintf (txe, "No token host defined");
            else if (!clientid || !*clientid)
               fprintf (txe, "No client ID defined");
            else if (!clientsecret || !*clientsecret)
               fprintf (txe, "No client secret defined");
            else
            {
               j_store_string (tx, "grant_type", "client_credentials");
               char *valid = NULL;
               asprintf (&valid, "%s:%s", clientid, clientsecret);
               er = j_curl (J_CURL_POST | J_CURL_BASIC, curl, tx, rx, valid, "https://%s/oauth2/token", host);
               free (valid);
               curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status);
               if (er)
                  fprintf (rxe, "Failed: %s\n", er);
               else
               {
                  const char *tokentype = j_get (rx, "token_type");
                  if (!tokentype)
                     fprintf (rxe, "Missing token_type\n");
                  if (strcmp (tokentype, "bearer"))
                     fprintf (rxe, "Expected token_type=bearer (is %s)", tokentype);
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
            }
            fclose (txe);
            fclose (rxe);
            char *rxt = j_write_str (rx);
            char *txt = j_write_str (tx);
            sql_safe_query_f (sqlp,
                              "INSERT INTO `log` SET `ID`=0,`tester`=%d,`ts`=NOW(),`status`=%ld,`description`='Sent OAUTH2 topen request',`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                              tester, status, rxt, *rxerror ? rxerror : NULL, txt, *txerror ? txerror : NULL);
            free (rxt);
            free (txt);
            j_delete (&rx);
            j_delete (&tx);
            free (er);
            if (!er)
               break;
         }
         if (bearer)
            sql_safe_query_f (sqlp, "UPDATE `tester` SET `bearer`=%#s,`expiry`=%#T WHERE `ID`=%d", bearer, time (0) + secs, tester);
      }
      const char *routing = j_get (tx, "envelope.routingID");
      int try = 0;
      while (try < 5)
      {
         if (try++)
            sleep (5);
         long status = 0;
         char *txerror = NULL;
         size_t txlen = 0;
         FILE *txe = open_memstream (&txerror, &txlen);
         char *rxerror = NULL;
         size_t rxlen = 0;
         FILE *rxe = open_memstream (&rxerror, &rxlen);
         char *er = NULL;
         const char *host = sql_col (res, "apihost");
         syntaxcheck (tx, txe);
         j_t rx = j_create ();
         if (!host || !*host)
            fprintf (txe, "No API host defined");
         else
         {
            // Send message
            er = j_curl_send (curl, tx, rx, bearer, "https://%s/letterbox/v1/post", host);
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status);
            if (er)
               fprintf (rxe, "Failed: %s\n", er);
            responsecheck (status, rx, rxe);
         }
         fclose (txe);
         fclose (rxe);
         char *rxt = j_write_pretty_str (rx);
         char *txt = j_write_pretty_str (tx);
         sql_safe_query_f (sqlp,
                           "INSERT INTO `log` SET `ID`=0,`tester`=%d,`ts`=NOW(),`status`=%ld,`description`='Sent %#S',`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                           tester, status, routing, j_isnull (rx) ? NULL : rxt, *rxerror ? rxerror : NULL,
                           j_isnull (tx) ? NULL : txt, *txerror ? txerror : NULL);
         free (rxt);
         free (txt);
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
notscoreply (j_t rx, j_t tx, const char *type)
{                               // Make reply message, set tx, return payload
   char *routing = strdupa (j_get (rx, "envelope.routingID"));
   if (!routing)
      errx (1, "No routingID");
   char *request = strstr (routing, "Request");
   if (!request)
      errx (1, "Not a Request: %s", routing);
   asprintf (&routing, "%.*s%s", (int) (request - routing), routing, type);
   j_t envelope = j_store_object (tx, "envelope");
   j_t source = j_store_object (envelope, "source");
   j_store_string (source, "type", "RCPID");
   j_store_string (source, "identity", j_get (rx, "envelope.destination.identity"));
   j_store_string (source, "correlationID", j_get (rx, "envelope.destination.correlationID"));
   j_t destination = j_store_object (envelope, "destination");
   j_store_string (destination, "type", "RCPID");
   j_store_string (destination, "identity", j_get (rx, "envelope.source.identity"));
   j_store_string (destination, "correlationID", j_get (rx, "envelope.source.correlationID"));
   j_store_string (envelope, "routingID", routing);
   j_t payload = j_store_object (tx, routing);
   free (routing);
   return payload;
}

void
notscofailure (SQL * sqlp, int tester, j_t rx, int code)
{                               // Fault code return message to other RCP
   j_t t = j_create ();
   j_t payload = notscoreply (rx, t, "Failure");
   j_store_stringf (payload, "faultCode", "%d", code);
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
   j_store_string (payload, "faultText", msg ());
   j_t audit = j_append_object (j_store_array (j_find (t, "envelope"), "auditData"));
   j_store_string (audit, "name", "faultCode");
   j_store_stringf (audit, "value", "%d", code);
   notscotx (sqlp, tester, t);
}

void
responsecheck (int status, j_t j, FILE * e)
{                               // This is the reporting for a response at http level
   if (status / 100 != 2)
      fprintf (e, "HTTP response %d\n", status);
   else if (status != 202)
      fprintf (e, "API§2.1.8: HTTP response expected is 202, was %d\n", status);
   if (j_isnull (j))
      return;
   if (!j_isobject (j))
   {
      fprintf (e, "API§2.1.8: Response is not a JSON object\n");
      return;
   }
   if (status / 100 == 2)
      fprintf (e, "API§2.1.8: No JSON response is expected for a 2XX response\n");
   // Check for error
   j_t v = NULL;
   if ((v = j_find (j, "errorCode")))
   {
      if (!j_isnumber (v))
         fprintf (e, "API§2.1.8: \"errorCode\" is expected to be an integer\n");
      fprintf (e, "errorCode %s\n", j_val (v));
   }
   if ((v = j_find (j, "errorText")))
   {
      if (!j_isstring (v))
         fprintf (e, "API§2.1.8: \"errorText\" is expected to be an string\n");
      fprintf (e, "errorText %s\n", j_val (v));
   }
   if ((v = j_find (j, "code")))
   {
      if (!j_isnumber (v))
         fprintf (e, "API§2.1.8: \"code\" is expected to be an integer\n");
      fprintf (e, "code %s\n", j_val (v));
   }
   if ((v = j_find (j, "message")))
   {
      if (!j_isstring (v))
         fprintf (e, "API§2.1.8: \"message\" is expected to be an string\n");
      fprintf (e, "message %s\n", j_val (v));
   }
   if ((v = j_find (j, "description")))
   {
      if (!j_isstring (v))
         fprintf (e, "API§2.1.8: \"description\" is expected to be an string\n");
      fprintf (e, "description %s\n", j_val (v));
   }
   if (status / 100 != 2)
   {
      if (!j_find (j, "errorCode") && !j_find (j, "code"))
         fprintf (e, "API§2.1.8: expected \"code\" or \"errorCode\"\n");
      if (j_find (j, "errorCode") && !j_find (j, "errorText"))
         fprintf (e, "API§2.1.8: expected \"errorText\" with \"errorCode\"\n");
      if (j_find (j, "code") && (!j_find (j, "message") || !j_find (j, "description")))
         fprintf (e, "API§2.1.8: expected \"message\" and \"description\" with \"code\"\n");
   }
}

void
syntaxcheck (j_t j, FILE * e)
{                               // This is the main syntax checking and reporting for all messages
   j_t v = NULL;
   const char *routing = NULL;
   // Envelope (and audit data)
   j_t envelope = j_find (j, "envelope");
   if (!envelope)
      fprintf (e, "API§2.1.5: \"envelope\" missing\n");
   else if (!j_isobject (envelope))
      fprintf (e, "API§2.1.5: \"envelope\" should be JSON object\n");
   else
   {                            // Check envelope
      if (!(v = j_find (envelope, "routingID")))
         fprintf (e, "API§2.1.5: \"routingID\" missing from \"envelope\"\n");
      else if (!j_isstring (v))
         fprintf (e, "API§2.1.5: \"routingID\" should be a string\n");
      else
      {
         routing = j_val (v);
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
            fprintf (e, "OTS§2.1: Invalid \"routindID\" (%s)\n", routing);
      }
   }
   if (!routing)
      return;
   // Payload specific checks
   j_t payload = j_find (j, routing);
   if (!payload)
      fprintf (e, "APIU§2.1.5: Missing \"%s\"\n", routing);
   if (strstr (routing, "Failure"))
   {                            // Audit requirements
      if (!(v = j_find (envelope, "auditData")))
         fprintf (e, "API§2.1.6: \"auditData\" missing from \"envelope\"\n");
      else if (!j_isarray (v))
         fprintf (e, "API§2.1.6: \"auditData\" should be a JSON array\n");
      else
      {
         v = j_first (v);
         if (!v)
            fprintf (e, "API§2.1.6: \"auditData\" should contain a entry\n");
         else if (!j_isobject (v))
            fprintf (e, "API§2.1.6: \"auditData\" should contain a JSON object entry\n");
         else
         {
            j_t t = NULL;
            if (!(t = j_find (v, "name")))
               fprintf (e, "API§2.1.6: \"name\" missing from \"auditData\"\n");
            else if (!j_isstring (t))
               fprintf (e, "API§2.1.6: \"name\" in \"auditData\" should be a string\n");
            else if (strcmp (j_val (t), "faultCode"))
               fprintf (e, "API§2.1.6: \"name\" in \"auditData\" should be \"faultCode\"\n");
            if (!(t = j_find (v, "value")))
               fprintf (e, "API§2.1.6: \"value\" missing from \"auditData\"\n");
            else if (!j_isstring (t))
               fprintf (e, "API§2.1.6: \"value\" in \"auditData\" should be a string\n");
            else if (j_number_ok (j_val (t), NULL))
               fprintf (e, "API§2.1.6: \"value\" in \"auditData\" should be numeric (%s)\n", j_val (t));
            else
            {
               const char *code = j_get (payload, "faultCode");
               if (code && strcmp (code, j_val (t)))
                  fprintf (e, "API§2.1.6: \"value\" in \"auditData\" (%s) does not match \"faultCode\" in payload (%s)\n",
                           j_val (t), code);
            }
            if (j_next (v))
               fprintf (e, "API§2.1.6: \"auditData\" should contain only one entry\n");
         }
      }
      if (payload)
      {                         // Failure payload
         if (!(v = j_find (payload, "faultCode")))
            fprintf (e, "API§2.1.6: \"faultCode\" missing from \"%s\"\n", routing);
         else if (!j_isstring (v))
            fprintf (e, "API§2.1.6: \"faultCode\" in \"%s\" should be a string\n", routing);
         else if (j_number_ok (j_val (v), NULL))
            fprintf (e, "API§2.1.6: \"faultCode\" in \"%s\" should be numeric (%s)\n", routing, j_val (v));
         if (!(v = j_find (payload, "faultText")))
            fprintf (e, "API§2.1.6: \"faultText\" missing from \"%s\"\n", routing);
         else if (!j_isstring (v))
            fprintf (e, "API§2.1.6: \"faultText\" in \"%s\" should be a string\n", routing);
         if (strcmp (routing, "residentialSwitchMatchFailure") && strcmp (routing, "messageDeliveryFailure"))
         {
            if (!(v = j_find (payload, "switchOrderReference")))
               fprintf (e, "API§2.1.6: \"switchOrderReference\" missing from \"%s\"\n", routing);
            else if (!j_isstring (v))
               fprintf (e, "API§2.1.6: \"switchOrderReference\" in \"%s\" should be a string\n", routing);
            else if (isuuid (j_val (v)))
               fprintf (e, "API§2.1.6: \"switchOrderReference\" in \"%s\" should be a valid UUID\n", routing);
         }
         return;
      }
   }
   if (!payload)
      return;
   // Specific payload checks
   if (!strcmp (routing, "residentialSwitchMatchRequest"))
   {

      return;
   }
   if (!strcmp (routing, "residentialSwitchMatchConfirmation"))
   {

      return;
   }
   if (strstr (routing, "Request"))
   {                            // Request
   }
   if (strstr (routing, "Confirmation"))
   {                            // Confirmation
   }
}
