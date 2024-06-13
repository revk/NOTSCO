#!../login/envcgi /bin/csh -f
source ../script/loggedin

if($?matchresponse) then
        unsetenv clientid
        unsetenv clientsecret
        unsetenv ID
        sqlwrite notsco tester ID="$TESTER"
	echo "Status: 303"
        echo "Location: https://$HTTP_HOST/control.cgi"
        echo ""
        exit 0
endif

echo Content-Type: text/html
echo ""
xmlsql --exec -d notsco head.html - tail.html << 'END'
<h1>Set next Match Request response</h1>
<p>Any match request received will generate a reply using these details.</p>
<sql table=tester where="ID=$TESTER">
<form method=post>
<table border=1>
<tr><td>Response delay</td><td><input name=delay size=4 maxlength=3 placeholder=secs></td></tr>
<tr><td>Match response</td><td>
<select name=matchresponse>
<option value=None>No response</option>
<option value=Match>Respond with match</option>
<option value=Match+Alt>Respond with match and alternatives</option>
<option value=NoMatch>Respond No Match (error)</option>
<option value=DeliveryFail>Respond delivery fail</option>
</select>
</td></tr>
<tr><td>Error code</td><td><select name=matcherror><exec ../bin/notscotx --error-choice></select> (for No Match)</td></tr>
<tr><td>NBICS Network Operator</td><td><input name=networkoperator size=4 maxlength=5 placeholder=ANNN></td></tr>
<tr><td>NBICS CUPID</td><td><input name=cupid size=3 maxlength=4 placeholder=NNN></td></tr>
<tr><td>Access Line ID</td><td><input name=alid size=12 placeholder=ALID></td></tr>
<tr><td>ONT Reference</td><td><input name=ontref size=12 placeholder="ONT Ref"></td></tr>
<tr><td>ONT port</td><td><input name=ontport size=2 placeholder=N></td></tr>
<tr><td>DN</td><td><input name=dn size=20 placeholder=Number></td></tr>
<tr><td>Partial DN</td><td><input name=partialdn size=3 maxlength=2 placeholder=NN></td></tr>
</table>
<input type=submit value="Save">
</form>
</sql>
'END'

