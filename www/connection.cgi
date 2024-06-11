#!../login/envcgi /bin/csh -f
source ../script/loggedin

if($?rcpid) then

endif

echo Content-Type: text/html
echo ""
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Connection and authentication</h1>
<p>The simulator uses OAUTH2 and https. If you need other authentication please raise an issue, and we will look in to that.</p>
<sql table=tester ID=$TESTER>
<form method=post>
<table border=1>
<tr><td colspan=4><b>Your details</b></td></tr>
<tr><td>Company name</td><td colspan=3><tt><input name=company size=60 placeholder="Company name"></tt></td></tr>
<tr><td>RCPID</td><td colspan=3><tt><input name=rcpid size=4 maxlength=4 placeholder=XXXX></tt></td></tr>
<tr><td colspan=4><b>Messages and access to the simulator</b></td></tr>
<tr><td>Token</td><td align=right><tt>https://</tt></td><td><tt>otshub-token.<output name=HTTP_HOST></tt></td><td><tt>/oauth2/token</tt></td></tr>
<tr><td>API</td><td align=right><tt>https://</tt></td><td><tt>otshub.<output name=HTTP_HOST></tt></td><td><tt>/directory/v1/entry</tt></td></tr>
<tr><td>API</td><td align=right><tt>https://</tt></td><td><tt>otshub.<output name=HTTP_HOST></tt></td><td><tt>/letter/v1/post</tt></td></tr>
<tr><td>Client ID</td><td colspan=3><tt><output name=clientid></tt></td></tr>
<tr><td>Client Secret</td><td colspan=3><tt><output name=clientsecret></tt></td></tr>
<tr><td colspan=4><b>Messages from the simulator</b></td></tr>
<tr><td>Token</td><td align=right><tt>https://</tt></td><td><tt><input name=tokenhost size=40 placeholder=Hostname></tt></td><td><tt>/oauth2/token</tt></td></tr>
<tr><td>API</td><td align=right><tt>https://</tt></td><td><tt><input name=apihost size=40 placeholder=Hostname></tt></td><td><tt>/letterbox/v1/post</tt></td></tr>
<tr><td>Client ID</td><td colspan=3><tt><input name=farclientid size=60 placeholder=ID></tt></td></tr>
<tr><td>Client Secret</td><td colspan=3><tt><input name=farclientsecret size=60 placeholder=Secret></tt></td></tr>
</table>
<input type=submit value="Save">
</form>
</sql>

'END'
