#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>One Touch Switching test platform</h1>
<p>This is your main control pages, allowing you to configure settings and responses, send messages, and see logs.</p>
<sql table=tester ID=$TESTER>
<ul>
<li><a href="/gettingstarted.cgi">Getting Started</a></li>
<li><a href="/connection.cgi">Connection and authentication</a></li>
<if apihost not apihost=''>
<li><a href="/matchresponse.cgi">Set up next match response</a></li>
<li><a href="/sendmatch.cgi">Send a match request</a></li>
<li><a href="/sendorder.cgi">Send order/update/trigger/cancel</a></li>
<li><a href="/sendbad.cgi">Send bad messages</a></li>
</if>
</ul>
<if not apihost or apihost=''><p>You need to set up your connection/authentication details before you can send messages.</p></if>
<if not rcpid or rcpid=''><p>You have not set an RCPID yet.</p></if>
<if not tokenhost or tokenhost=''><p>You have not set a token host yet.</p></if>
<if not farclientid or farclientid=''><p>You have not set a Client ID yet.</p></if>
<if not farclientsecret or farclientsecret=''><p>You have not set a Client Secret yet.</p></if>
<hr>
<h2>Recent messages</h2>
<p>Messages are cleared at the end of each day</p>
<table>
</table>
</sql>
'END'
