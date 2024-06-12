#!/bin/csh -f
set path=($path ../xmlsql ../SQLlib)

echo "Content-Type: text/html"
echo ""

xmlsql head.html - tail.html << 'END'
<h1>One Touch Switching test platform</h1>
<p>This site provides a test environment for One Touch Switching development. It allows messages to be sent and received as if to/from another CP, setting up the response to a match request in advance, and even sending deliberately invalid messages to test error checking. The message logs include detailed logs of request and response, and analysis of errors and issues detected.</p>
<h2>Why would someone use this?</h2>
<p>One Touch Switching is managed by <a href='https://totsco.org.uk'>TOTSCO Ltd</a>. But at this point their simulator seems not to be fit for purpose, and does not follow the OTS specification even for simple things like the URL paths used (see <a href="https://www.me.uk/2024/06/working-with-totsco.html">blog post</a>). This site is designed to provide an easy to use, useful, self service platform, with no need to book a test slot. It means it can be used to test <i>as you go</i> during development if you want. It should be useful to any CP that is developing One Touch Switching (before you go on to TOTSCO testing). It may also be useful to CPs that have already developed their One Touch Switching that want to check against an external test system. Obviously we think this meets the spec, but will be happy to update if we have any errors or the spec changes.</p>
<p>This service is provided free of charge, and sponsored by <a href="https://firebrick.co.uk">FireBrick</a>. It is not affiliated with, or endorsed by, TOTSCO Ltd.</p>
<p>To start using this service, enter your email address at your CP, and you will be emailed a link.</p>
<form method=post action=signup.cgi>
<input name=email autofocus placeholder="Business email address" size=80 type=email><input type=submit value="Send link">
</form>
<p>For suggestions and support, raise issues on <a href="https://github.com/revk/NOTSCO">GitHub</a></p>
'END'
