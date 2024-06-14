#!../login/envcgi /bin/csh -fx
source ../script/loggedin

if($?SEND) then
	if("$sor" == "") then
		setenv MSG "Set switch order reference"
			goto show
	endif
        ../bin/notscotx --tester="$TESTER" "$SEND" --sor="$sor" --nearid="$nearid" --farid="$farid" --dated="$dated" --rcpid="$rcpid"
        echo "Status: 303"
        echo "Location: https://$HTTP_HOST/control.cgi"
        echo ""
        exit 0
endif

if(! $?dated) then
	setenv dated `date +%F -d"14 days"`
endif

show:
echo Content-Type: text/html
echo ""
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Send order/update/tigger/cease</h1>
<if MSG><p><b><output name=MSG></b></p></if>
<form method=post name=F>
<table border=1>
<tr><td>RCPID</td><td><input name=rcpid size=5 maxlength=4 placeholder="XXXX"></td></tr>
<tr><td>Switch order reference</td><td><input name=sor size=37 maxlength=36 placeholder="XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"></td></tr>
<tr><td>Our correlation ID</td><td><input name=nearid size=40 placeholder="ID"></td></tr>
<tr><td>Their correlation ID</td><td><input name=farid size=40 placeholder="ID"></td></tr>
<tr><td>Dated</td><td><input name=dated size=11 maxlength=10 placeholder="YYYY-MM-DD"></td></tr>
</table>
<p>
<input type=submit name=SEND value="residentialSwitchOrderRequest"><br>
<input type=submit name=SEND value="residentialSwitchOrderUpdateRequest"><br>
<input type=submit name=SEND value="residentialSwitchOrderTriggerRequest"><br>
<input type=submit name=SEND value="residentialSwitchOrderCancellationRequest">
</p>
</form>
<table border=1>
<p>Select SOR to send</p>
<sql table=sor where="tester=$TESTER and issuedby='THEM'" order="created" DESC><set found=1>
<tr>
<td><button onclick="F.rcpid.value='$rcpid';F.sor.value='$sor';F.nearid.value='$nearid';F.farid.value='$farid';">Select</button></td>
<td><output name=created type=recent></td>
<td><output name=rcpid></td>
<td><output name=sor></td>
<td><output name=dated></td>
<td><output name=status></td>
</tr>
</sql>
</table>
<if not found><p>You need to send a match order request first and ger SOR</p></if>

'END'
