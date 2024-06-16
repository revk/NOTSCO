#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Getting Started</h1>
<p>The platform allows messages to be sent and received as if to/from an other CP via an OTS hub. You can send messages, and you can set up the response to a match request in advance.</p>
<p>The messages are all logged, along with details of analysis of the messages and any errors or anomalies.</p>
<p>The first step is <a href="/connection.cgi">Connection and authentication</a> which allows you set up the domain names, client ID and client secret for messages each way, and your details.</p>
<p>Once you have done that you can start sending messages each way.</p>
<p>More information on the protocol can be found at the <a href="https://totsco.org.uk/documents-centre/">TOTSCO Documents Centre</a>.</p>
<h2>Retention/privacy</h2>
<ul>
<li>This site is not intended to not store personal data.</li>
<li>Message logs are cleared at the end of each day.</li>
<li>Switch order references are cleared after 60 days.</li>
<li>All data is cleared if you delete your account. You are welcome to sign up again if you wish.</li>
<li>Logins are cleared if not used for 6 months.</li>
</ul>
'END'
