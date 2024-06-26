#!../login/envcgi /bin/csh -f
source ../script/loggedin

if($?SEND) then
        ../bin/notscotx --tester="$TESTER" "$SEND"
        echo "Status: 303"
        echo "Location: https://$HTTP_HOST/control.cgi"
        echo ""
        exit 0
endif

echo Content-Type: text/html
echo ""
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Send bad messages</h1>
<p>In addition to these specific bad messages, you can also send invalid messages such as a match request by leaving madatory fields blank or sending invalid values.</p>
<form method=post>
<table>
<tr><td><input type=submit name=SEND value=BadEnvelope1></td><td>Send a message with bad source type.</td></tr>
<tr><td><input type=submit name=SEND value=BadEnvelope2></td><td>Send a message with bad destination type.</td></tr>
<tr><td><input type=submit name=SEND value=BadEnvelope3></td><td>Send a message with bad destination identity.</td></tr>
<tr><td><input type=submit name=SEND value=BadEnvelope4></td><td>Send a message with missing destination identity.</td></tr>
<tr><td><input type=submit name=SEND value=BadEnvelope5></td><td>Send a message with missing destination type.</td></tr>
<tr><td><input type=submit name=SEND value=BadEnvelope6></td><td>Send a message with source type.</td></tr>
<tr><td><input type=submit name=SEND value=BadRouting></td><td>Send a message bad routing.</td></tr>
</table>
</form>
'END'
