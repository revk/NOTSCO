# Suggested bulletin

# Usage of correlationID

The API specification defines an `envelope` which includes a `source` and `destination`, each of which can have a `correlationID`.

The specificaiton says :-

*A string of characters that the message originator will
recognise and allow matching of a reply to a request
message.*

*In a source element, the correlationID must always be
provided, the format can be anything the originator
chooses to support their messaging process but should
be sufficiently unique to allow correlation of response
with request over a reasonable period.*

*In a destination element, the correlationID would only be
populated when the message is being sent in response
to a message previously sent to you. In that case the
correlationID will be the value that was sent by the
original sender of the message – i.e., it is being reflected
to them.*

This bulletin aims to clarify the expected use of `correlationID`.

## Requests and responses

Messages exchanged between CPs fall in to two categories.

- **Requests** (having a `routingID` ending `Request`) are sent, when required, from one CP to another. They include messages like `residentialSwitchMatchRequest`, or `residentialSwitchOrderTriggerRequest`
- **Responses** (all other messages) are only sent as a response to a request.
- In addition, a response can be sent back for any message sent to the hub, `messageDeliveryFailure`

## correlationID

- Every message sent to the hub **must** have a `source.correlationID`.
- A *request* **must not** have a `destination.correlationID`.
- Any *response* **must** have a `destination.correlationID` which is a copy of the `source.correlationID` to which the message is a response.

*Note that a `messageDeliveryFailure` sent by the hub does not have a `source.correlationID` so you must allow for receipt of such messages.*

### Common examples of usage

One of the problems with the current API specification is that it suggests the `source.correlationID` is entirely at the whim of, and for the benefit of the sender.
This means some possible uses are :-

- Allocating an ID for an order flow, and using it for multiple `residentialSwitchMatchRequest`, e.g. if address is  changed or an account number added. Then even using for the `residentialSwitchOrderRequest`. This allows the sender to match replies to their ordering flow, i.e. using a simple ID is *sufficiently unique* for the sender's message flow.
- Using the Switch Order Reference on all switch action messages, so replies can be tied to the specific switch order.
- Using the Switch Order Reference on replies to switch action messages so a `messageDeliveryFailure` can be tied to a specific switch order.
- Using a fixed value on Switch Order action messages or responses as all such messages, including failures, carry the `switchOrderReference` so the `correlationID` does not matter (assuming the sender does not care about `messageDeliveryFailure`)
- Using a fixed value on response message, as a response does not need to be correlated (assuming the sender does not care about `messageDeliveryFailure`)

Unfortunately, even though these seem to be within the current API specification, they may have interactions with de-duplication usage used by other CPs as per TOTSCO Bulletin 66.

For that reason, the following clarification of usage of `correlationID` and recommended best practice...

# Recommended usage of correlationID

Whilst the current API specification simply says that the `source.correlationID` is used simply to *support the sender's messaging process*,
and simply *should* be *sufficiently* unique to match a request to a response, the actual usage needs to be clarified.

There are three main uses for the `correlationID`.

- For the sender of a message to be able to match that request with a response to the request.
- For the sender of a response message to match with a `messageDeliveryFailure` returned by the hub.
- For the recipient of a message to recognise a duplication message (as per bulletin 66).

What this means is that **every message sent** needs to have a **unique correlationID**, per message. Otherwise the recipient could assume a message is a duplicate.

It is recommended that the sender of a message allocates a new unique UUID as the `source.correlationID` for every message.
