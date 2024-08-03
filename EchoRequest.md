# Proposed new OTS message (echo)

## This document proposes two new One Touch Switching messages

- `echoRequest`
- `echoConfirmation`

## Purpose

The purpose of these messsages is to provide a benign end to end connectivity test between CPs at the OTS level.

## echoRequest

The sending of an `echoRequest` should normally be manually insticated by the sending CP, either because of testing requirements with TOSTCO, on-ramp, etc, of for diagnosing any issues with connectivity.

A CP SHOULD NOT set up any form of automated regular generation of `echoRequest` messages without the express agreement of the receiving CP.

TOTSCO may want to consider an option of sending regular `echoRequest` messages as a service to a CP, with a means of alerting the CP if a corresponding `echoConfirmation` is not received in a timely manor.

### Payload

The only payload is `generated` which is a timestamp, UK local time, `YYYY-MM-DD HH:MM:SS` when the `echoRequest` was generated.

### Processing of an echoRequest

The `echoRequest` should be processed by the receiving CP using the same message flows, queues, and processing systems as they would process a *match request*. This is important is the purpose is to confirm end to end message processing. As such the processing of an `echoReqest` should not *bypass* the normal queuing and processing systems. 

The recipient of an `echoRequest` should automatically generate and send an `echoConfirmation` message, and send this via the same normal processing and queuing as would a *match confirmation*.

## echoConfirmation

The only payload is `processed` which is a timestamp, UK local time, `YYYY-MM-DD HH:MM:SS` when the `echoRequest` was processed and the `echoConfirmation` was generated.

## echoFailure

`echoFailure` is not defined, but reserved for future use, as such no echo specific error codes are defined at this time.

## SLA

There is no SLA, but as the processing should use the same systems and queues a as a *match request* the sender of an `echoRequest` can assume a connectivity issue exists if they do not receive a corresponding `echoConfirmation` within 60 seconds.
