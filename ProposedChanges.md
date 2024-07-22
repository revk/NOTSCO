# Suggestions for updates to TOTSCO specifications.

The TOTSCO specifications seem to be very poorly written, including notable omissions, and contradictions, and ambiguities.
The specifications come in several layers: the API specification; The OTS message specification; and Industry Processes. There are also examples. These do not always align. Some aspects from Industry process would be suitable for inclusion in the OTS message specification, notably things like acceptable ranges for dates.

Notably "address matching" needs to be in one place and really clearly defined.

I have outlined a number of proposed changes for consideration.

MRs welcome, let's colaborate on this before sending to TOTSCO shall we?

## API

The API specification should ensure all examples match the specification and align with the separate examples documentation. At present they do not, notably:

- Example request messages incorrectly included a `envelope.destination.correlationID`, leading to the confusion that they might be considered a reply to a previous reply (e.g. `residentialSwitchMatchConfirmation`) in some way. This could lead to extra unnecessary work by CPs.
- Many examples do not match the defined field list - one or other needs correcting.

### 2.1.2

The strict URI format is defined for use by TOTSCO, RCPs and MAPs, but it is not in fact required by TOSTCO for RCPs/MAPs. TOTSCO allow any URI for CP/MAP to be specified. TOTSCO do follow this URI scheme for the hub, but not for the simulator. This should be clarified as more flexibility over the URI format will help some CPs.

### 2.1.8

- This defines `errorCode`, `errorText`, `code`, `message`, and `Description`. The fields include `Description` but the examples all use `description` (lower case).
- The examples include an undefined field `nextAccessTime`, which appears to use an insane message format for a date/time (should be RFC 3339 please).
- The examples include an undefined field `type`. This should be defined.
- The fields define `errorCode` and `code` as *Integer*, but the examples are all strings.
- `messageDeliveryFailure` fields define `Code`, `Text`, `Severity`, but the examples have `code`, `text`, and `severity` (lower case).
- `messageDeliveryFailure` defined `Code` as *Integer*, but the example is a string.
- The examples then show various combinations. It would help to clarify the exact combinations that are valid, which seems to be errorCode+errorText, or code+message+description only. It is not clearly explained why the two styles.

### 2.2.1

The directory (at least on pre-production) does not follow the specification as it requires `identity=all`, even though the specification says it is *optional*.

### 3.2.1.1.1

Make the specification actually reference the RFC (6749) and explain the options for using `Authorization` header, and for `client_id`/`client_secret` as per §2.3.1 of that RFC. It needs to say which TOTSCO use to authenticate with the CP (which I believe is `Authorization` header). At present it only mentions using `Authorization` header.

### JSON

It seems to me that it would help to have a proper definition of message content, not just saying JSON, e.g. 

- As documented, OAUTH2 token requests shall use `Content-Type: application/x-www-form-urlencoded` POST, and directory API uses a URL encoded GET query string. Both provide a JSON response (`Content-Type: application/json`).
- Successful post messages must return a status `202` and an empty response, (`Content-Length:0`) which can be `Content-Type: application/json`.
- All other messages (request and response at an HTTP level) shall be `Content-Type: application/json`, and shall be a single JSON object (as per RFC 8259).

It also seems prudent (after correcting 2.1.8 *Integer* definitions) to state:

- All simple values in the JSON object shall be a JSON string type. Other JSON types (numeric, null, boolean) shall not be used.

It also seems prudent (given the examples that do this), to say:

- Where a field is defined as being optional, it should be omitted. However, the recipient of a message should consider such a field that contains an empty string (`””`) is the same as being omitted.

### RCPID

The data type RCPID should be defined. It seems to be :-

- An RCPID is assigned by TOTSCO, and is 4 (latin) letters, or the fixed string `“TOTSCO”`. RCPIDs will normally start with the letter `R`, but CPs should not assume that is the case.

The explanation (which does not allow `"TOTSCO"`) in 2.2.1, and for various other data types, including RCPID, is no longer needed if properly defined in a definitions section.

### correlationID

The specification is contradictory and unclear. I would suggest the following. Note the proposed definition having a limited length, which is a clear omission from the current specification.

- All messages sent by a CP/MAP to the hub **must** have an `envelope.source.correlationID`. This **must** be unique for each message sent (we recommend using a UUID), such that any reply can be correlation to the original message, and that duplicates can be recognised (see TOTSCO bulletin 66). Note that any message to the hub can have a reply, even if it is only `messageDeliveryFailure`, hence an `envelope.source.correlationID` is **required** even on a message that is, itself, logical a reply message.
- Messages with `routingID` ending `Request` shall omit any `envelope.destination.correlationID`, as they are not a reply to a message that was sent.
- All other messages must include an `envelope.destination.correlationID` that is the `envelope.source.correlationID` to which this message is a reply.
- Note that this means all messages sent by the hub will have an `envelope.source.correlationID` provided by the original CP, except for `messageDeliveryFailure` where it is omitted.
- A *correlationID* is any JSON string, but **must not** be more than 255 characters in length (this is to allow for use in SQL tables as a `tinytext` type). It is recommended that a UUID is used, but this is not a requirement.

## OTS

The OTS specification should ensure all examples match the specification and align with the separate examples documentation. At present they do not.

### Definitions

A clear definitions for data types should be included, covering all data types, including at least:

- NetworkOperator
- CUPID
- Telephone number
- Partial DN
- AccessLineID
- dates and times (see below)
- Postcode (is it case insensitive, does it need the space?)
- UUID (reference RFC), and highlight that it is **case insensitive**. This is especially important if stored in SQL as it may be retrieved in differene case to how stored.
- and any others…

They should say the format, and maximum size of such fields. I would suggest clear consideration as you how such types might be defined in SQL.

### JSON

It seems to me that it would help to have a proper definition of message content, not just saying JSON, e.g. 

- The payload of all messages shall be a single JSON object (as per RFC 8259), tagged with the value of the `envelope.routingID`, as per the API specification.

It also seems prudent to state:

- All simple values in the JSON object shall be a JSON string type. Other JSON types (numeric, null, boolean) shall not be used.
It also seems prudent (given the examples that do this), to say:
- Where a field is defined as being optional, it should be omitted. However, the recipient of a message should consider such a field that contains an empty string (`””`) is the same as being omitted.

### Dates and times

I suggest:

- All date and datetime values shall be UK local time zone, and expressed in RFC 3339 format with no time zone suffix. i.e. `YYYY-MM-DD` or `YYYY-MM-DD HH:MM:SS` only. 

### Planned/activation dates

The specification and industry process do define some rules regarding dates. I think extending these and making them very clear within the OTS specification as well as Industry Process would be in order. This will allow range checking on message receipt for simple error response.

I specifically propose:

- A maximum time limit for switch orders of 6 months. This is something for Industry process to agree, but at present there is no limit - a switch order could set for 10 years and block all other switch orders for that time as losing CP has no way to cancel. Picking up this as an error would be prudent. A new error code may be prudent for this, if accepted as a rule. If any order is likely to exceed the time, a new order could be created, indeed that may be a reason to limit to a short time such as 3 months.
- A limit on how back dated an activation can be, this is likely to be important for CPs billing processes. At present it is not clear that a gaining provider cannot back date before the original match request, even, which would clearly be wrong.
- Clear rules on planned being *today or future* and activation being *today or past*. Additional error codes would help resolve any issues, such as *Date is in the past*, and *Date is in the future*.

#### residentialSwitchOrderRequest:

The `plannedSwitchDate`

- shall not be before the current date.
- shall not be more than 6 months beyond the date the switch order was issued (i.e. when `residentialSwitchMatchConfirmation` was received).

#### residentialSwitchOrderUpdateRequest:

The `plannedSwitchDate`

- shall not be before the current date.
- shall not be more than 6 months beyond the date the switch order was issued (i.e. when the `residentialSwitchMatchConfirmation` was received).

In addition, a `residentialSwitchOrderUpdateRequest` cannot be sent more than 31 days beyond the previously advised `plannedSwitchDate` regardless of `plannedSwitchDate`.

#### residentialSwitchOrderTriggerRequest:

The `activationDate`

- shall not be after the current date.
- shall not be before the date the switch order was issued (i.e. when the `residentialSwitchMatchConfirmation` was received).
- shall not be more than 6 months beyond date the switch order was issued (i.e. when the `residentialSwitchMatchConfirmation` was received).
- shall not be more than 31 days before the current date.

In addition, a `residentialSwitchOrderTriggerRequest` cannot be sent more than 31 days beyond the previously advised `plannedSwitchDate` regardless of `activationDate`.

#### Billing

The `activationDate` is the date the gaining provider starts billing, and as such the losing provide should bill up to the end of the day before.

# Test platform

TOTSCO need a proper test platform. This is especially an issue now, but an issue ongoing for new CPs.

The simulator is useless - it claims to test basic connectivity but does not even pick up slow response (outside 3s SLA). It generates fixed (and hence invalid) responses, and has no way to originate messages to the CP. It is very limited and useless for any ISP that has a proper system and expects valid responses.

Pre-production Integration tests relies on two CPs, quite possibly with both not being fully developed or ready, and each with their own interpretation of the specifications. I have seen this work badly, and well, but it is a total roll of the dice what CP you get and if they can function as a test platform. CPs should not be expected to provide TOTSCO with a test platform.

Pre-production testing also needs co-ordinating with another CP, arranged by TOTSCO, which can take weeks to arrange. A test system should be readily available without delay to all CPs, and continue to be available in to production and ramp up.
What is needed is an interactive test platform.

Operating in the same way as the pre-production platform - indeed, it could be connected to the pre-production platform as a CP, perhaps with an RCPID of TEST. It could even be on the production platform to help address issues found in production systems.

- It should allow response to be pre-set for messages, both match requests and the various switch order requests. This is to include the data in the response, and a choice of response failure messages.
- It should allow requests to be initiated to the CP, both match requests and the switch order messages.
- It should include a comprehensive set of test messages for valid, invalid, and unusual data in various ways, a set of tests to be completed by an ISP to confirm their error checking and data sanitation. E.g. tests to confirm matching with accented characters, and so on.
- All messages should be syntax and semantically checked and all errors reported with reference to specifications so that even subtle errors can be identified and reported. This would provide a lot more detail than simply sending an error code.
- It should provide statistics on tests done, and create a suitable report for completion of testing.

A free open source platform exists, it took only a few days to create, so why have TOTSCO not made such a platform.

# Silly

The *declaration* to confirm testing complete has a section worded:

- "I confirm we have made no internal system changes since declaration"

Sorry, but that is daft, as "at the point of making the declaration" that is always true.

Was it meant to be

- "I confirm we have made no internal system changes since testing was completed"

Maybe?
