#!../login/envcgi /bin/csh -f
source ../script/loggedin

if($?brand) then
        unsetenv clientid
        unsetenv clientsecret
        unsetenv ID
        sqlwrite notsco tester ID="$TESTER"
	../bin/notscotx --tester="$TESTER" residentialSwitchMatchRequest
	echo "Status: 303"
        echo "Location: https://$HTTP_HOST/control.cgi"
        echo ""
        exit 0
endif

echo Content-Type: text/html
echo ""
xmlsql --exec -d notsco head.html - tail.html << 'END'
<h1>Send a Match Request</h1>
<p>You can send an invalid request by leaving mandatory fields blank, or using invalid values.</p>
<sql table=tester where="ID=$TESTER">
<form method=post>
<table border=1>
<tr><td>From RCPID</td><td><input name=fromrcpid size=5 maxlength=4 placeholder="XXXX"></td></tr>
<tr><td>Brand name</td><td><input name=brand placeholder="Your brand name"></td></tr>
<tr><td>Surname</td><td><input name=surname placeholder="Surname"></td></tr>
<tr><td>Account</td><td><input name=account placeholder="Account"></td></tr>
<tr><td>UPRN</td><td><input name=uprn size=13 maxlength=12 placeholder="UPRN"></td></tr>
<tr><td rowspan=5>Address</td><td><input name=address1 placeholder="Line 1"></td></tr>
<tr><td><input name=address2 placeholder="Line 2"></td></tr>
<tr><td><input name=address3 placeholder="Line 3"></td></tr>
<tr><td><input name=address4 placeholder="Line 4"></td></tr>
<tr><td><input name=address5 placeholder="Line 5"></td></tr>
<tr><td>Post Town</td><td><input name=posttown placeholder=Town></td></tr>
<tr><td>Post Code</td><td><input name=postcode size=9 maxlength=8 placeholder="XXNN NXX"></td></tr>
<tr><td>IAS circuit</td><td><input name=circuit placeholder="Circuit"></td></tr>
<tr><td>PortDN</td><td><input name=portdn size=12 maxlength=11 placeholder="Telephone"> If set, sends an NBICS request, else IAS</td></tr>
<tr><td>IdentifyDN</td><td><input name=identifydn size=12 maxlength=11 placeholder="Telephone"></td></tr>
</table>
<input type=submit value="Send">
</form>
</sql>
'END'

