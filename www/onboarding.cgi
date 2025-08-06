#!/bin/csh -f
set path=(../xmlsql ../SQLlib $path)

#!../login/envcgi /bin/csh -f
#source ../script/loggedin
  
echo Content-Type: text/html
echo ""
  
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Onboarding</h1>
<p>The TOTSCO process involves several stages. Once you get on the pre-production platform you can send and receive messsages to other CPs on the pre-production platform. It is usually a good idea to talk to the CP first, but some CPs are happy for you to send mesages at any time (including A&amp;A).</p>
<p>Once you think you are ready, TOTSCO will pair you with another CP and arrange a call with them and you and go through some simple tests. They actually want you to do a lot of messages as well, but that ccan be a script of a few hundred match requests if necessary.</p>
<p>The idea is that this shows you are ready to go live. It is not a comprehensive test by any means.</p>
<h2>Platforms</h2>
<p>You need to either be able to switch your system between platforms or handle more than one platform. Handling more than one platform is a good idea. You can make your dev system talk to the TOTSCO pre-production platform both ways. You can make a system which talks to NOTSCO both ways. And finally you have a live system that talks to/from TOTSCO live platform and your live customer database and interactions. Obviously your test systems will want to avoid sending messages to real customers or ceasing real services.</p>
<p>It makes a lot of sense to keep a dev systrem that talks to TOTSCO pre-production, it allows you to carry out tests if something comes up later before changes are made live. Sorry I have to say this!</p>
<p>One trick on your dev/test platform is to direct messages based on the CP identity, a simple trick is to set up NOTSCO to be <tt>TEST</tt>. Then your dev system can have code to route any message to <tt>TEST</tt> to NOTSO and other messages to TOTSCO pre-production platform. It makes testing using pre-production and NOTSCO simple.</tt>
<h2>Are you ready?</h2>
<p>You need to be able to send and receive all of the types of message. This means the <tt>SwitchMatch</tt> messages (<tt>Request</tt>, <tt>Confirmation</tt>, and <tt>Failure</tt>), and the same for <tt>SwitchOrder</tt>, <tt>SwitchOrderUpdate</tt>, <tt>SwitchOrderCancellation</tt>, and <tt>SwitchOrderTrigger</tt>. You also need to handle a <tt>messageDeliveryFailure</tt>.</p>
<p>This is a lot to test on the call, 16 message types, sent and received to your buddy CP, so 30 messages total. If your system is ready it can be done quite quickly.</p>
<h2>NOTSCO can help</h2>
<p>The NOTSCO system handles and generates all of these message types, and you can do tests. The <a href="scorecard.cgi">Scorecard</a> can confirm what you have managed.</p>
<h2>A&amp;A can help.</h2>
<p>Yes, we can be a buddy CP, but some rules, sorry.</p>
<ul>
<li>Use NOTSCO, even if you did not in your development, it will help find issues you may have missed. Fill the scrore card for all the message types.</li>
<li>Once you are ready and have done that testing, then contact us or ask TOTSCO to contact us.</li>
<li>We'll want to go through some tests on pre-production platform, to/from you, without errors our NOTSCO system will detect.</li>
<li>Basically, we're not trying to spend a lot of time hand holding. If you can get through NOSTCO, the call with TOSTCO should be quick and easy.</li>
<li>Once the call is done, we can do the several hundred messages on a script if TOTSCO insist on it.</li>
</ul>
'END'
