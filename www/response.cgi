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
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Set next Match Request response</h1>
<p>Any match request received will generate a reply using these details. You can send an invalid response by leaving mandatory fields blank, or using invalid values.</p>
<sql table=tester where="ID=$TESTER">
<form method=post>
<table border=1>
<tr><th colspan=2>Match response</th></tr>
<tr><td>Response delay</td><td><input name=delay size=4 maxlength=3 placeholder=secs></td></tr>
<tr><td>Response</td><td><select name=matchresponse>
<option value=0>No reply</option>
<option value=1>Match Response</option>
<option value=2>Match Response + alternative</option>
<option value=3>Match Response + two alternatives</option>
<include src=notscoerrors.html>
</select></td></tr>
<tr><td>Sent to (email/sms)</td><td><input name=sentto placeholder="Email/Telephone"></td><td rowspan=2>(1st class post if bothblank)</td></tr>
<tr><td>Sent to (email/sms)</td><td><input name=sentto2 placeholder="Email/Telephone"></td></tr>
<tr><td>IAS Network Operator</td><td><input name=networkoperator size=4 maxlength=5 placeholder="ANNN"></td></tr>
<tr><td>NBICS CUPID</td><td><input name=cupid size=3 maxlength=4 placeholder="NNN"></td></tr>
<tr><td>Access Line ID</td><td><input name=alid size=12 placeholder="ALID"></td></tr>
<tr><td>ONT Reference</td><td><input name=ontref size=12 placeholder="ONT Ref"></td></tr>
<tr><td>ONT port</td><td><input name=ontport size=2 placeholder="N"></td></tr>
<tr><td>DN</td><td><input name=dn size=20 placeholder="Telephone"></td></tr>
<tr><td>Partial DN</td><td><input name=partialdn size=3 maxlength=2 placeholder="NN"></td></tr>
<tr><th colspan=2>Order response</th></tr>
<tr><td>Order/Update/Trigger/Cancel response</td><td><select name=orderresponse>
<option value=0>No reply</option>
<option value=1>Normal reply</option>
<include src=notscoerrors.html>
</select></td></tr>
</table>
<input type=submit value="Save">
</form>
<p>Received match orders.</p>
<set found>
<table border=1>
<sql table=sor where="tester=$TESTER and issuedby='US'" order="created" DESC>
<if not found><set found=1><tr><th>Created</th><th>RCPID</th><th>Switch order reference</th><th>Date</th><th>Status</th></tr></if>
<tr>
<td><output name=created type=recent></td>
<td><output name=rcpid></td>
<td><tt><output name=sor></tt></td>
<td><output name=dated></td>
<td><output name=status></td>
</tr>
</sql>
</table>
<if not found><p>You have not received any match orders.</p></if>
<p>Match orders are retained for 60 days to allow expired references to be tested.</p>
</sql>
'END'

