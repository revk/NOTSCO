#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>One Touch Swicthing test platform</h1>
<p>This is your main control pages, allowing you to configure settings and responses, send messages, and see logs.</p>
<ul>
<li><a href="/gettingstarted.cgi">Getting Started</a></li>
<li><a href="/connection.cgi">Connection and authentication</a></li>
<li><a href="/matchresponse.cgi">Set up next match response</a></li>
<li><a href="/sendmatch.cgi">Send a match request</a></li>
<li><a href="/sendorder.cgi">Send order/update/trigger/cancel</a></li>
<li><a href="/sendbad.cgi">Send bad messages</a></li>
</ul>
<hr>
<h2>Recent messages</h2>
<p>Messages are cleared at the end of each day</p>
<table>

</table>
'END'
