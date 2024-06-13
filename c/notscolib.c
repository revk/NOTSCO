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

static const char *
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

static const char *
isdigits (const char *u)
{
   if (!u)
      return "NULL";
   if (!*u)
      return "Empty string";
   while (*u && isdigit (*u))
      u++;
   if (*u)
      return "Non numeric";
   return NULL;
}

static const char *
ispostcode (const char *u)
{
   if (!u)
      return "NULL";
   if (!*u)
      return "Empty string";
   if (strlen (u) > 8)
      return "Too long";
   if (strlen (u) < 5)
      return "Too short";
   const char *e = u + strlen (u) - 1;
   if (!isalpha (*e) || !isalpha (e[-1]) || !isdigit (e[-2]))
      return "Bad inbound";
   e -= 3;
   if (e > u && *e == ' ')
      e--;
   if (e > u && isalpha (*e))
      e--;
   if (e == u || !isdigit (*e))
      return "Missing digit";
   e--;
   if (e > u && isdigit (*e))
      u--;
   if (e == u || !isalpha (*e))
      return "Missing letter";
   if (e > u && isalpha (*e))
      e--;
   if (e > u)
      return "Extra on start";
   return NULL;
}

static const char *
istelephone (const char *u)
{
   if (!u)
      return "NULL";
   if (!*u)
      return "Empty string";
   if (strlen (u) > 11)
      return "Too long";
   if (strlen (u) < 10)
      return "Too short";
   if (*u != '0')
      return "Needs to start 0";
   u++;
   if (*u == '0')
      return "International";
   if (*u == '4' || *u == '6')
      return "Not sensible";
   while (*u && isdigit (*u))
      u++;
   if (*u)
      return "Non numeric";
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
               if (!j_isnull (rx))
                  break;        // We got an error
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
         if (!j_isnull (rx))
            break;              // We got an error
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
   if (!type)
      routing = strdup ("messageDeliveryFailure");
   else
      asprintf (&routing, "%.*s%s", (int) (request - routing), routing, type);
   const char *v = NULL;
   j_t envelope = j_store_object (tx, "envelope");
   j_t source = j_store_object (envelope, "source");
   j_store_string (source, "type", "RCPID");
   j_store_string (source, "identity", !type ? "TOTSCO" : j_get (rx, "envelope.destination.identity")); // Note, example is TOTSCO, but surely it should be original destination identity regardless?
   if ((v = j_get (rx, "envelope.destination.correlationID")))
      j_store_string (source, "correlationID", v);
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
notscofailure (SQL * sqlp, int tester, j_t rx, int code)
{                               // Fault code return message to other RCP
   int n = code;
   if (n < 0)
      n = 0 - n;
   j_t t = j_create ();
   j_t payload = notscoreply (rx, t, code < 0 ? NULL : "Failure");
   j_store_stringf (payload, code < 0 ? "code" : "faultCode", "%d", n);
   const char *msg (void)
   {
      switch (n)
      {
#define e(c,e) case c:return e;
#include "notscoerrors.m"
      default:
         return "Unspecified error";
      }
   }
   j_store_string (payload, code < 0 ? "text" : "faultText", msg ());
   if (code < 0)
      j_store_string (payload, "severity", "failure");
   j_t audit = j_append_object (j_store_array (j_find (t, "envelope"), "auditData"));
   j_store_string (audit, "name", "faultCode");
   j_store_stringf (audit, "value", "%d", n);
   notscotx (sqlp, tester, t);
}

// Tools for syntax checking

static void *
expected (FILE * e, const char *ref, j_t parent, j_t v, const char *tag, const char *val, const char *type)
{                               // Report expected and return NULL
   if (!v && tag)
      v = j_find (parent, tag);
   fprintf (e, "%s: ", ref);
   if (tag)
      fprintf (e, "\"%s\"", tag);
   else if (v)
      fprintf (e, " entry %d", j_pos (v));
   j_t p = parent;
   while (p)
   {
      j_t up = j_parent (p);
      if (up && !strcmp (j_name (up) ? : "", "formdata"))
         up = NULL;
      const char *name = j_name (p);
      if (name)
         fprintf (e, " in \"%s\"", name);
      else if (up)
         fprintf (e, " entry %d", j_pos (p));
      p = up;
   }
   if (!v)
      fprintf (e, " is missing");
   if (v && val && *val && strcmp (j_val (v), val))
      fprintf (e, " is expected to be \"%s\" but is \"%s\"", val, j_val (v));
   else if (v && type)
      fprintf (e, " is expected to be a %s", type);
   fprintf (e, ".\n");
   return NULL;
}

static j_t
expect_object (FILE * e, const char *ref, j_t parent, const char *tag)
{                               // Return object if it is as expected
   j_t v = j_find (parent, tag);
   if (!v || !j_isobject (v))
      return expected (e, ref, parent, v, tag, NULL, "a JSON object");
   if (!j_len (v))
      return expected (e, ref, parent, v, tag, NULL, "a non empty object");
   return v;
}

static j_t
expect_array (FILE * e, const char *ref, j_t parent, const char *tag)
{                               // Return array if it is as expected
   j_t v = j_find (parent, tag);
   if (!v || !j_isarray (v))
      return expected (e, ref, parent, v, tag, NULL, "a JSON array");
   if (!j_len (v))
      return expected (e, ref, parent, v, tag, NULL, "a non empty array");
   return v;
}

static const char *
expect_string (FILE * e, const char *ref, j_t parent, const char *tag, const char *val)
{                               // Return string if as expected, if val="" then allow missing, if val non null expects to match val
   j_t v = j_find (parent, tag);
   if ((!v && (!val || *val)) || (v && !j_isstring (v)) || (v && val && *val && strcmp (j_val (v), val)))
      return expected (e, ref, parent, v, tag, val, "a JSON string");
   return j_val (v);
}

static const char *
expect_number (FILE * e, const char *ref, j_t parent, const char *tag, const char *val)
{                               // Return string if as expected, if val="" then allow missing, if val non null expects to match val
   j_t v = j_find (parent, tag);
   if ((!v && (!val || *val)) || (v && !j_isnumber (v)) || (v && val && *val && strcmp (j_val (v), val)))
      return expected (e, ref, parent, v, tag, val, "a JSON number");
   return j_val (v);
}

void
responsecheck (int status, j_t j, FILE * e)
{                               // This is the reporting for a response at http level
   if (status)
   {
      if (status / 100 != 2)
         fprintf (e, "- HTTP responded with status %d\n", status);
      else if (status != 202)
         fprintf (e, "API§2.1.8: HTTP response expected is 202, was %d\n", status);
   }
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
   const char *val = NULL;
   if ((val = expect_number (e, "API§2.1.8", j, "errorCode", "")))
      fprintf (e, "- errorCode %s\n", val);
   if ((val = expect_string (e, "API§2.1.8", j, "errorText", "")))
      fprintf (e, "- errorText %s\n", val);
   if ((val = expect_number (e, "API§2.1.8", j, "code", "")))
      fprintf (e, "- code %s\n", val);
   if ((val = expect_string (e, "API§2.1.8", j, "message", "")))
      fprintf (e, "- message %s\n", val);
   if ((val = expect_string (e, "API§2.1.8", j, "description", "")))
      fprintf (e, "- description %s\n", val);
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
   const char *val = NULL;
   const char *routing = NULL;
   int df = 0;
   // Envelope (and audit data)
   j_t envelope = expect_object (e, "API§2.1.5", j, "envelope");
   if (envelope)
   {                            // Check envelope
      if ((routing = expect_string (e, "API§2.1.5", envelope, "routingID", NULL)))
      {
         const char *t = routing;
         if (!strcmp (t, "messageDeliveryFailure"))
            df = 1;
         else
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
      void check (const char *tag)
      {
         j_t v = expect_object (e, "API§2.1.5", envelope, tag);
         expect_string (e, "API§2.1.5", v, "type", "RCPID");
         if ((val = expect_string (e, "API§2.1.5", v, "identity", NULL)) && strlen (val) != 4)
            expected (e, "API§2.1.5", v, NULL, "identity", NULL, "a 4 character string");
         expect_string (e, "API§2.1.5", v, "correlationID", (*tag == 's' && !strstr (routing, "Failure"))
                        || (*tag == 'd' && routing && strcmp (routing, "residentialSwitchMatchRequest")) ? NULL : "");
      }
      check ("source");
      check ("destination");
   }
   if (!routing)
      return;
   // Payload specific checks
   j_t payload = expect_object (e, "API§2.1.5", j, routing);
   if (strstr (routing, "Failure"))
   {                            // Audit requirements
      j_t ad = expect_array (e, "API§2.1.6", envelope, "auditData");
      if (ad)
      {
         ad = j_first (ad);
         if (!ad || !j_isobject (ad))
            expected (e, "API§2.1.6", j_parent (ad), ad, NULL, NULL, "a JSON object");
         else
         {
            expect_string (e, "API§2.1.6", ad, "name", "faultCode");
            if ((val = expect_string (e, "API§2.1.6", ad, "value", NULL)))
            {
               if (isdigits (val))
                  expected (e, "API§2.1.6", ad, NULL, "value", NULL, "numeric");
               else
                  expect_string (e, "API§2.1.6", payload, df ? "code" : "faultCode", val);
            }
            if (j_next (ad))
               expected (e, "API§2.1.6", j_parent (ad), ad, NULL, NULL, "only entry in array");
         }
      }
      if (payload)
      {                         // Failure payload
         const char *tag = df ? "code" : "faultCode";
         if ((val = expect_string (e, "API§2.1.8", payload, tag, NULL)) && isdigits (val))
            expected (e, "API§2.1.6", payload, NULL, tag, NULL, "numeric");
         tag = df ? "text" : "faultText";
         expect_string (e, "API§2.1.8", payload, tag, NULL);
         if (df)
            expect_string (e, "API§2.1.8", payload, "severity", "failure");
         else if (strcmp (routing, "residentialSwitchMatchFailure"))
         {
            const char *ref = "OTS§2.3.2";
            if (!strcmp (routing, "residentialSwitchOrderUpdateFailure"))
               ref = "OTS§2.4.2";
            else if (!strcmp (routing, "residentialSwitchOrderTriggerFailure"))
               ref = "OTS§2.5.2";
            else if (!strcmp (routing, "residentialSwitchOrderCancellationFailure"))
               ref = "OTS§2.6.2";
            if ((val = expect_string (e, ref, payload, "switchOrderReference", NULL)) && isuuid (val))
               expected (e, ref, payload, NULL, "switchOrderReference", NULL, "a valid UUID");
         }
         return;
      }
   }
   if (!payload)
      return;
   // Specific payload checks
   if (!strcmp (routing, "residentialSwitchMatchRequest"))
   {
      expect_string (e, "OTS§2.2", payload, "grcpBrandName", "");
      expect_string (e, "OTS§2.2", payload, "name", NULL);
      expect_string (e, "OTS§2.2", payload, "account", "");
      j_t address = expect_object (e, "OTS§2.2", payload, "address");
      if (address)
      {
         if ((val = expect_string (e, "OTS§2.2", address, "uprn", "")) && (isdigits (val) || strlen (val) > 12))
            expected (e, "API§2.1.6", address, NULL, "uprn", NULL, "up to 12 digits");
         const char *posttown = expect_string (e, "OTS§2.2", address, "postTown", NULL);
         const char *postcode = expect_string (e, "OTS§2.2", address, "postCode", NULL);
         if (postcode && ispostcode (postcode))
            expected (e, "API§2.1.6", address, NULL, "postCode", NULL, "valid post code");
         j_t lines = expect_array (e, "OTS§2.2", address, "addressLines");
         for (j_t l = j_first (lines); l; l = j_next (l))
            if (!j_isstring (l))
               expected (e, "API§2.1.6", lines, l, NULL, NULL, "a JSON string");
            else if (!*j_val (l))
               expected (e, "API§2.1.6", lines, l, NULL, NULL, "a non empty JSON string");
            else if (posttown && !strcasecmp (j_val (l), posttown))
               expected (e, "API§2.1.6", lines, l, NULL, NULL, "not include post town");
            else if (postcode && !strcasecmp (j_val (l), postcode))
               expected (e, "API§2.1.6", lines, l, NULL, NULL, "not include post code");
      }
      j_t services = expect_array (e, "OTS§2.2", payload, "services");
      for (j_t s = j_first (services); s; s = j_next (s))
      {
         const char *st = expect_string (e, "OTS§2.2", s, "serviceType", NULL);
         if (st && strcmp (st, "IAS") && strcmp (st, "NBICS"))
            expected (e, "API§2.1.6", s, NULL, "serviceType", NULL, "\"IAS\" or \"NBICS\"");
         if ((val = expect_string (e, "OTS§2.2", s, "action", NULL)) && strcmp (val, "cease")
             && (!strcmp (st, "IAS") || (strcmp (val, "port") && strcmp (val, "identify"))))
            expected (e, "API§2.1.6", s, NULL, "action", NULL,
                      !strcmp (st, "IAS") ? "\"cease\"" : "\"cease\" or \"port\" or \"idenitfy\"");
         const char *id = expect_string (e, "OTS§2.2", s, "seviceIdentifier", !strcmp (st, "NBICS") ? NULL : "");
         if (id && !strcmp (st, "NBICS") && !istelephone (id))
            expected (e, "API§2.1.6", s, NULL, "seviceIdentifier", NULL, "valid telephone number");
      }
      return;
   }
   if (!strcmp (routing, "residentialSwitchMatchConfirmation"))
   {

      return;
   }
   if (strstr (routing, "Request"))
   {                            // Other Requests

   }
   if (strstr (routing, "Confirmation"))
   {                            // Other Confirmations

   }
}
