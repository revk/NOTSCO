// TOTSCO error codes
e(1101, "Missing or incomplete address")
e(1102, "Name not provided")
e(1103, "Account not found")
e(1104, "Account found but is closed or historic")
e(1105, "Account found but at least one serviceIdentifier not found on the account.")
e(1106, "Address not found")
e(1107, "No customers found with service at that location")
e(1108, "One or more customers found, but no match on surname")
e(1109, "Multiple customers found matching on surname")
e(1110, "Customer found, but they have multiple matching services at the same address.")
e(1111, "A switch is currently in progress")
e(1112, "Data Integrity issue detected by LRCP.")
e(1113, "Account number format not valid for residentialMatchRequest")
e(1114, "Address does not match despite two other strong points of contact, one of which is account number")
e(1115, "Address does not match and no account number is included despite two other strong points of contact")
e(1116, "Service Identifier not found.")
e(1117, "Services not included or invalid")
e(1118, "No DN included as service identifier for an NBICS 'port' or 'identify'")
e(1119, "Account found, but no IAS or NBICS services were found under it")
e(1120, "The requested service was not found against the matched customer / account / subscription")
e(1121, "Name does not match and address is only a close match")
e(1122, "Name does not match, address and service identifier match, but account number is not included")
e(1201, "Invalid or missing switch order reference")
e(1202, "Switch order reference has expired")
e(1203, "Invalid or missing planned switch date")
e(1204, "Switch order has already been completed")
e(1205, "Switch order has already been cancelled")
e(1211, "A switch is currently in progress")
e(1212, "All services requested to be ceased are no longer active")
e(1213, "Switch Order Reference is already in use")
e(1214, "There is an open cease order which is past point of no return and cannot be cancelled.")
e(1215, "There is an open modify order which is past point of no return and cannot be cancelled.")
e(1301, "Invalid or missing switch order reference")
e(1302, "Switch order reference is no longer available")
e(1303, "Invalid or missing planned switch date")
e(1304, "Switch order has already been completed")
e(1305, "Switch order has already been cancelled")
e(1306, "Switch order was never raised")
e(1401, "Invalid or missing switch order reference")
e(1402, "Switch order reference is no longer available")
e(1403, "Invalid or missing activation date")
e(1404, "Switch order has already been completed")
e(1405, "Switch order has already been cancelled")
e(1406, "Switch order was never raised")
e(1501, "Invalid or missing switch order reference")
e(1502, "Switch order reference is no longer available")
e(1504, "Switch order has already been completed")
e(1505, "Switch order has already been cancelled")
e(1506, "Switch order was never raised")
e(9005, "Unable to deliver the message to the destination, no valid route.")
e(9006, "Unable to deliver the message to the destination, rejected, invalid message format.")
e(9007, "Recipient rejected message.")
e(9008, "Unable to deliver the message to the destination, timed out.")
e(9013, "Unable to deliver message to the destination - Invalid API Key")
e(9014, "Unable to deliver message to the destination - API Key expired")
e(9015, "Unable to deliver message to the destination - Digital certificate invalid")
e(9016, "Unable to deliver message to the destination - Digital certificate expired")
#undef e
