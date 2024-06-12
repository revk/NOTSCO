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
                              "INSERT INTO `log` SET `ID`=0,`ts`=NOW(),`status`=%ld,`description`='Sent OAUTH2 topen request',`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                              status, rxt, *rxerror ? rxerror : NULL, txt, *txerror ? txerror : NULL);
            if (tester)
               sql_safe_query_f (sqlp, "UPDATE `log` SET `tester`=%d WHERE `ID`=%d", tester, sql_insert_id (sqlp));
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
      while (try<5)
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
         }
         fclose (txe);
         fclose (rxe);
         char *rxt = j_write_str (rx);
         char *txt = j_write_str (tx);
         sql_safe_query_f (sqlp,
                           "INSERT INTO `log` SET `ID`=0,`ts`=NOW(),`status`=%ld,`description`='Sent %#S',`rx`=%#s,`rxerror`=%#s,`tx`=%#s,`txerror`=%#s",
                           status, routing, rxt, *rxerror ? rxerror : NULL, txt, *txerror ? txerror : NULL);
         if (tester)
            sql_safe_query_f (sqlp, "UPDATE `log` SET `tester`=%d WHERE `ID`=%d", tester, sql_insert_id (sqlp));
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
      case 1101:
         return "Missing or incomplete address";
      case 1102:
         return "Name not provided";
      case 1103:
         return "Account not found";
      case 1104:
         return "Account found but is closed or historic";
      case 1105:
         return "Account found but at least one serviceIdentifier not found on the account.";
      case 1106:
         return "Address not found";
      case 1107:
         return "No customers found with service at that location";
      case 1108:
         return "One or more customers found, but no match on surname";
      case 1109:
         return "Multiple customers found matching on surname";
      case 1110:
         return "Customer found, but they have multiple matching services at the same address.";
      case 1111:
         return "A switch is currently in progress";
      case 1112:
         return "Data Integrity issue detected by LRCP.";
      case 1113:
         return "Account number format not valid for residentialMatchRequest";
      case 1114:
         return "Address does not match despite two other strong points of contact, one of which is account number";
      case 1115:
         return "Address does not match and no account number is included despite two other strong points of contact";
      case 1116:
         return "Service Identifier not found.";
      case 1117:
         return "Services not included or invalid";
      case 1118:
         return "No DN included as service identifier for an NBICS 'port' or 'identify'";
      case 1119:
         return "Account found, but no IAS or NBICS services were found under it";
      case 1120:
         return "The requested service was not found against the matched customer / account / subscription";
      case 1121:
         return "Name does not match and address is only a close match";
      case 1122:
         return "Name does not match, address and service identifier match, but account number is not included";
      case 1201:
         return "Invalid or missing switch order reference";
      case 1202:
         return "Switch order reference has expired";
      case 1203:
         return "Invalid or missing planned switch date";
      case 1204:
         return "Switch order has already been completed";
      case 1205:
         return "Switch order has already been cancelled";
      case 1211:
         return "A switch is currently in progress";
      case 1212:
         return "All services requested to be ceased are no longer active";
      case 1213:
         return "Switch Order Reference is already in use";
      case 1214:
         return "There is an open cease order which is past point of no return and cannot be cancelled.";
      case 1215:
         return "There is an open modify order which is past point of no return and cannot be cancelled.";
      case 1301:
         return "Invalid or missing switch order reference";
      case 1302:
         return "Switch order reference is no longer available";
      case 1303:
         return "Invalid or missing planned switch date";
      case 1304:
         return "Switch order has already been completed";
      case 1305:
         return "Switch order has already been cancelled";
      case 1306:
         return "Switch order was never raised";
      case 1401:
         return "Invalid or missing switch order reference";
      case 1402:
         return "Switch order reference is no longer available";
      case 1403:
         return "Invalid or missing activation date";
      case 1404:
         return "Switch order has already been completed";
      case 1405:
         return "Switch order has already been cancelled";
      case 1406:
         return "Switch order was never raised";
      case 1501:
         return "Invalid or missing switch order reference";
      case 1502:
         return "Switch order reference is no longer available";
      case 1504:
         return "Switch order has already been completed";
      case 1505:
         return "Switch order has already been cancelled";
      case 1506:
         return "Switch order was never raised";
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
