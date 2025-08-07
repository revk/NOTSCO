# Onboarding with TOTSCO

The official process has these steps...

- SIT
- Buddy CP
- Sign declaration
- PIT

## SIT

This is a "simulator", sort of. The objective is to test connectivity.

However, there is a test specification that involves a lot of different sorts of messages.
The challenge is that any CP that has a proper working system will not be able to generate or handle the invalid messages involved.

A&A were able to progress with no more testing having shown a message each way.
The irony is SIT does not test the 3 second SLA so failed to identify a connectivity issue we had, the one thing is is meant to check.

So you probably need to argue with TOTSCO to get passed this. If you have done NOTSCO, connectivty should be no problem. Ask them to confirm that A&A did not have to do these tests.

## Buddy CP

The next stage is testing on pre-production with a "buddy CP" - this is not testing against the specifications (poor that they are).

The TOTSCO requirement is one of each message type each way, and 1000 messages (which makes no sense, but I hear they are relaxing).

See below for more on this.

## PIT

This is to be on production / live - one message each way is all that is needed.

# Buddy CP with A&A

A&A is prepared to do some buddy CP testing with CPs, generally. Ask nicely.

See https://notsco.co.uk/onboarding.cgi
