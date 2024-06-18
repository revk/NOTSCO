#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>One Touch Switching test platform</h1>
<sql table=tester WHERE="ID=$TESTER">
<if company not company=''><h2><output name=company></h2></if>
<p>This is your main control page, allowing you to configure settings and responses, send messages, and see logs.</p>
<ul>
<if not apiurl or apiurl='' or not rcpid or rcpid='' or not tokenurl or tokenurl='' or not farclientid or farclientid='' or not farclientsecret or farclientsecret=''>
<li><a href="/gettingstarted.cgi">Getting Started</a></li>
<if not apiurl or apiurl=''><li>You need to set up your connection/authentication details before you can send messages.</li></if>
<if not rcpid or rcpid=''><li>You have not set an RCPID yet.</li></if>
<if not tokenurl or tokenurl=''><li>You have not set a token API yet.</li></if>
<if not farclientid or farclientid=''><li>You have not set a Client ID yet.</li></if>
<if not farclientsecret or farclientsecret=''><li>You have not set a Client Secret yet.</li></if>
</if>
<li><a href="/connection.cgi">Connection and authentication</a><if rcpid not rcpid=''> (<output name=rcpid>)</if><if not apiurl or apiurl=''> <b>NOT SET UP</b></if></li>
<li><a href="/response.cgi">Set up responses</a></li>
<li><a href="/sendmatch.cgi">Send a match request</a><if postcode not postcode=''> (<output name=postcode>)</if></li>
<li><a href="/sendorder.cgi">Send order/update/trigger/cancel</a></li>
<li><a href="/sendbad.cgi">Send bad messages</a></li>
</ul>
<hr>
<h2>Today's messages</h2>
<p><a href="/control.cgi">Reload</a> Note messages are deleted at the end of each day. Select message for more detailed log.</p>
<table border=1>
<sql table=log where="tester=$TESTER" order=ID DESC>
<if not found><set found=1><tr><th>Time</th><th>Status</th><th>IP/time</th><th>Message</th><th>Notes/errors</th></tr></if>
<tr>
<td valign=top><output name=ts type=%T href="/log.cgi?ID=$ID"></td>
<td valign=top><output name=status 0=''></td>
<td valign=top align=right><if ip><output name=ip></if><if else ms not ms=0><output name=ms>ms</if></td>
<td valign=top style='white-space:nowrap;'><output name=description></td>
<td style='white-space:pre;font-weight:bold;'><if txerror or rxerror><if ip rxerror><output name=rxerror><if txerror><hr></if></if><if txerror><output name=txerror><if not ip rxerror><hr></if></if><if not ip rxerror><output name=rxerror></if></if></td>
</tr>

</sql>
</table>
<if not found><p>No mesage logs found. Remember, messages are cleared at the end of each day.</p></if>
</sql>
'END'
