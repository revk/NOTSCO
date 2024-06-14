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
<li><a href="/gettingstarted.cgi">Getting Started</a></li>
<li><a href="/connection.cgi">Connection and authentication</a><if rcpid not rcpid=''> (<output name=rcpid>)</if><if not apihost or apihost=''> <b>NOT SET UP</b></if></li>
<li><a href="/matchresponse.cgi">Set up next match response</a> (<output name=matchresponse>)</li>
<li><a href="/sendmatch.cgi">Send a match request</a><if postcode not postcode=''> (<output name=postcode>)</if></li>
<li><a href="/sendorder.cgi">Send order/update/trigger/cancel</a></li>
<li><a href="/sendbad.cgi">Send bad messages</a></li>
</ul>
<if not apihost or apihost=''><p>You need to set up your connection/authentication details before you can send messages.</p></if>
<if not rcpid or rcpid=''><p>You have not set an RCPID yet.</p></if>
<if not tokenhost or tokenhost=''><p>You have not set a token host yet.</p></if>
<if not farclientid or farclientid=''><p>You have not set a Client ID yet.</p></if>
<if not farclientsecret or farclientsecret=''><p>You have not set a Client Secret yet.</p></if>
<hr>
<h2>Today's messages</h2>
<table>
<sql table=log where="tester=$TESTER" order=ID DESC><set found=1>
<tr>
<td valign=top><output name=ts type=%T href="/log.cgi?ID=$ID"></td>
<td valign=top><output name=status></td>
<td valign=top><output name=ip></td>
<td valign=top style='white-space:nowrap;'><output name=description></td>
<if txerror or rxerror>
<td style='white-space:pre;font-weight:bold;'><if ip rxerror><output name=rxerror><if txerror><hr></if></if><if txerror><output name=txerror><if not ip rxerror><hr></if></if><if not ip rxerror><output name=rxerror></if></td>
</if>
</tr>

</sql>
</table>
<if not found><p>No mesage logs found. Remember, messages are cleared at the end of each day.</p></if>
</sql>
'END'
