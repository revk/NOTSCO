#!../login/envcgi /bin/csh -f
source ../script/loggedin
  
echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Log message</h1>
<sql select="min(ID) AS I" table=log WHERE="tester=$TESTER AND ID>$ID"><if I><p><a href="log.cgi?ID=$I">Next</a></p></if></sql>
<sql table=log WHERE="tester=$TESTER AND ID=$ID">
<table>
<tr><td>When</td><td><output name=ts></td></tr>
<if ip><tr><td>IP</td><td><output name=ip></td></tr></if>
<tr><td>Status</td><td><output name=status></td></tr>
</table>
<table border=1>
<tr><td colspan=2><output name=description></td></tr>
<tr>
<if ip><td>Rx</td></if>
<td>Tx</td>
<if not ip><td>Rx</td></if>
</tr>
<tr>
<if ip><td style='white-space:pre;font-weight:bold;'><output rxerror></td></if>
<td style='white-space:pre;font-weight:bold;'><output txerror></td>
<if not ip><td style='white-space:pre;font-weight:bold;'><output rxerror></td></if>
</tr>
<tr>
<if ip><td><pre><output name=rx></pre></td></if>
<td><pre><output name=tx></pre></td>
<if not ip><td><pre><output name=rx></pre></td></if>
</table>
</sql>
<sql select="max(ID) AS I" table=log WHERE="tester=$TESTER AND ID<$ID"><if I><p><a href="log.cgi?ID=$I">Previous</a></p></if></sql>
'END'
