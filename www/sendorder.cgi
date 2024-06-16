#!../login/envcgi /bin/csh -fx
source ../script/loggedin

if(! $?dated) then
	setenv dated `date +%F -d"14 days"`
endif

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

show:
echo Content-Type: text/html
echo ""
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Send order/update/tigger/cease</h1>
<p>Select the SOR, or enter manually. You can send an invalid response by leaving mandatory fields blank, or using invalid values.</p>
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
<input type=submit name=SEND value="residentialSwitchOrderRequest">
<input type=submit name=SEND value="residentialSwitchOrderUpdateRequest">
<input type=submit name=SEND value="residentialSwitchOrderTriggerRequest">
<input type=submit name=SEND value="residentialSwitchOrderCancellationRequest">
</p>
</form>
<p>Sent match orders, select to fill in the above details.</p>
<table border=1>
<if not found><set found=1><tr><th>Select</th><th>Created</th><th>RCPID</th><th>Switch order reference</th><th>Date</th><th>Status</th></tr></if>
<sql table=sor where="tester=$TESTER and issuedby='THEM'" order="created" DESC><set found=1>
<tr>
<td><button onclick="F.rcpid.value='$rcpid';F.sor.value='$sor';F.nearid.value='$nearid';F.farid.value='$farid';">Select</button></td>
<td><output name=created type=recent></td>
<td><output name=rcpid></td>
<td><tt><output name=sor></tt></td>
<td><output name=dated></td>
<td><output name=status></td>
<if status=new><td><a href="/sendorder.cgi?sor=$+sor&rcpid=$+rcpid&nearid=$+nearid&farid=$+farid&SEND=residentialSwitchOrderRequest">Order</a></td></if>
<if status=confirmed or status=updated><td><a href="/sendorder.cgi?sor=$+sor&rcpid=$+rcpid&nearid=$+nearid&farid=$+farid&SEND=residentialSwitchOrderUpdateRequest">Update</a></td></if>
<if status=confirmed or status=updated><td><a href="/sendorder.cgi?sor=$+sor&rcpid=$+rcpid&nearid=$+nearid&farid=$+farid&SEND=residentialSwitchOrderTriggerRequest">Trigger</a></td></if>
<if status=confirmed or status=updated><td><a href="/sendorder.cgi?sor=$+sor&rcpid=$+rcpid&nearid=$+nearid&farid=$+farid&SEND=residentialSwitchOrderCancellationRequest">Cancel</a></td></if>
</tr>
</sql>
</table>
<if not found><p>You have not sent any match orders yet.</p></if>
<hr>
<p>Received match orders, for your information.</p>
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

'END'
