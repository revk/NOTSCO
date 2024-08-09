#!/bin/csh -f
set path=(../xmlsql ../SQLlib $path)

echo "Content-Type: text/html"
echo ""

xmlsql head.html - tail.html << 'END'
<h1>NOTSCO: One Touch Switching test platform</h1>
<p>This site provides an unofficial, independent, free, test environment for One Touch Switching development.</p>
<ul>
<li>Send customised match requests, and receive responses.</li>
<li>Set up customised match responses, including error responses.</li>
<li>Send and receive switching order messages.</li>
<li>Send deliberately bad, adversarial, or edge case, messages for error checking.</li>
<li>See logs (for the day), with detailed syntax checking report, of messages sent and received, with over 150 separate tests for message syntax and content.</li>
<li>Paste JSON to this site to check individual messages, e.g. those received or sent elsewhere which you wish to check.</li>
<li>Updated with new tests and checks and new message/response options as they come to light, and when TOTSCO issue bulletins or clarify matters.</li>
<li>Can be used for testign at all levels, from basic API and message passing to top level order journey and integration tests, as well as incrmental development testing.</li>
</ul>
<p>If you are not a UK communications provider with consumers as customers, this site is not for you.</p>
<h2>Why would someone use this?</h2>
<p>Under OFCOM General Conditions, One Touch Switching is mandatory for all UK fixed broadband and fixed telephony providers with consumer customers, even if just retail providers of someone else&apos;s service.</p>
<p>One Touch Switching is  managed by <a href='https://totsco.org.uk'>TOTSCO Ltd</a>. But at this point their simulator seems not to be fit for purpose, and does not follow the OTS specification even for simple things like the URL paths used (see <a href="https://www.me.uk/2024/06/working-with-totsco.html">blog post</a>). This site is designed to provide an easy to use, useful, self service platform, with no need to book a test slot. It means it can be used to test <i>as you go</i> during development if you want. It should be useful to any CP that is developing One Touch Switching (before you go on to TOTSCO testing). It may also be useful to CPs that have already developed their One Touch Switching that want to check against an external test system.</p>
<p>We have tried to follow the spec, let us know if you think there is any error. Please bear in mind the specifications are <i>woolly</i> at best. We have tried to reference the specification as much as possible, but some is based on examples, clarifications, bulkletins, and messages actually seen from TOTSCO. We&apos;re happy to take on suggestions in improving our checking.</p>
<p>This service is provided free of charge, and sponsored by <a href="https://firebrick.co.uk">FireBrick</a>. It is not affiliated with, or endorsed by, TOTSCO Ltd.</p>
<p>To start using this service, enter your email address at your CP, and you will be emailed a link.</p>
<form method=post action=signup.cgi>
<input name=email autofocus placeholder="Business email address" size=80 type=email><input type=submit value="Send link">
</form>
<p>For suggestions and support, raise issues on <a href="https://github.com/revk/NOTSCO">GitHub</a></p>
<p>The NOTSCO bot is tooting stats mesages every day at <a href='https://social.aa.net.uk/@notsco'>https://social.aa.net.uk/@notsco</a>.</p>
'END'
