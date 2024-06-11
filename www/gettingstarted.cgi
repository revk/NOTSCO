#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Getting Started</h1>
<p>The platform allows messages to be sent and received as if to/from an other CP via an OTS hub. You can send messages, and you can set up the response to a match request in advance.</p>
<p>The messages are all logged, along with details of analysis of the messages and any errors or anomalies. These logs are clearde at the end of each day.</p>
<p>The first step if <a href="/connection.cgi">Connection and authentication</a> which allows you set up the domain names, client ID and client secret for messages ech way, and your details.</p>
<p>Once you have done that you can start sending messages each way.</p>
<p>TODO more links to TOTSCO documentation...</p>
'END'
