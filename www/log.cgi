#!../login/envcgi /bin/csh -f
source ../script/loggedin
  
echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Log message</h1>
<sql select="min(ID) AS I" table=log WHERE="tester=$TESTER AND ID>$ID"><if I><p><a href="log.cgi?ID=$I">Next</a></p></if></sql>
<table>
<sql table=log WHERE="tester=$TESTER AND ID=$ID">
<tr><td>When</td><td><output name=ts></td></tr>
<tr><td>Description</td><td><output name=description></td></tr>
<if ip><tr><td>IP</td><td><output name=ip></td></tr></if>
<tr><td>Status</td><td><output name=status></td></tr>
<if ip>
<tr><td colspan=2><hr></td></tr>
<tr><td>Rx</td><td><tt><output name=rx></tt></td></tr>
<if rxerror><tr><td>Errors</td><td style='white-space:pre;font-weight:bold;'><output name=rxerror></td></tr></if>
</if>
<tr><td colspan=2><hr></td></tr>
<if not tx=null><tr><td>Tx</td><td><tt><output name=tx></tt></td></tr></if>
<if txerror><tr><td>Errors</td><td style='white-space:pre;font-weight:bold;'><output name=txerror></td></tr></if>
<if not ip>
<tr><td colspan=2><hr></td></tr>
<if not rx=null><tr><td>Rx</td><td><tt><output name=rx></tt></td></tr></if>
<if rxerror><tr><td>Errors</td><td style='white-space:pre;font-weight:bold;'><output name=rxerror></td></tr></if>
</if>
</sql>
</table>
<sql select="max(ID) AS I" table=log WHERE="tester=$TESTER AND ID<$ID"><if I><p><a href="log.cgi?ID=$I">Previous</a></p></if></sql>
'END'
