#!../login/envcgi /bin/csh -f
source ../script/loggedin
  
echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Log message</h1>
<p>
<a href="control.cgi">Home</a>
<sql select="max(ID) AS I" table=log WHERE="tester=$TESTER AND ID<$ID"><if I><a href="log.cgi?ID=$I">Previous</a></if></sql>
<sql select="min(ID) AS I" table=log WHERE="tester=$TESTER AND ID>$ID"><if I><a href="log.cgi?ID=$I">Next</a></if></sql>
</p>
<sql table=log WHERE="tester=$TESTER AND ID=$ID">
<table>
<tr><td>When</td><td><output name=ts></td></tr>
<if ip><tr><td>IP</td><td><output name=ip></td></tr></if>
<if status not status=0><tr><td>Status</td><td><output name=status></td></tr></if>
<if ms not ms=0><tr><td>Response time</td><td><output name=ms>ms</td></tr></if>
</table>
<table border=1>
<tr><td colspan=2><output name=description></td></tr>
<tr>
<if rx ip><td>Rx</td></if>
<if tx><td>Tx</td></if>
<if rx not ip><td>Rx</td></if>
</tr>
<if txerror or rxerror>
<tr>
<if rx ip><td valign=top style='white-space:pre;font-weight:bold;'><output name=rxerror></td></if>
<if tx><td valign=top style='white-space:pre;font-weight:bold;'><output name=txerror></td></if>
<if rx not ip><td valign=top style='white-space:pre;font-weight:bold;'><output name=rxerror></td></if>
</tr>
</if>
<tr>
<if rx ip><td valign=top><pre><output name=rx></pre></td></if>
<if tx><td valign=top><pre><output name=tx></pre></td></if>
<if rx not ip><td valign=top><pre><output name=rx></pre></td></if>
</table>
</sql>
'END'
