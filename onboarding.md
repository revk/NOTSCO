# Onboarding with TOTSCO

The official process have three steps...

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

We'll expect you to have done NOTSCO tests first please. Do messages both ways cleanly. This should make buddy CP testing a breeze.

As I say, the requirement is every message type each way. So this is the plan.
We run all the messages through the NOTSCO syntax checker, so we expect all messages to be *clean* through that as well.
We can do this in advance and tell TOTSCO it is done, but they may want a call to go through it, before they move to production.

## Testing to A&A

- Send MatchRequest for 10 Downing Street, London, SW1A 2AA, with name Sunak, get match failure
- Send MatchRequest for 10 Downing Street, London, SW1A 2AA, with name Starmer, get match confirmation
- Send Switch order, get confirmation
- Send Switch order again, get failure
- Send Switch update, get confirmation
- Send Switch update (dated last year), get failure
- Send Switch cancel, get confirmation
- Send Switch cancel again, get failure
- Send MatchRequest for 10 Downing Street, London, SW1A 2AA, with name Starmer, get match confirmation
- Send Switch order, get confirmation
- Send Trigger, get confirmation
- Send Trigger again, get failues
- Send messasge to RCPID XXXX, get MessageDeliveryFailure

Provide us with an address and surname to do exactly the same back to you.

As part of that, if TOTSCO ask, we can do 1000 match requests as well.
