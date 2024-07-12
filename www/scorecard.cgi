#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Score card</h1>
<sql table=tester WHERE="ID=$TESTER">
<if company not company=''><h2><output name=company></h2></if>
</sql>
<p>Counts of messages</p>
<table border=1>
<sql table=scorecard where='tester=$TESTER' order="status,direction,routing" desc>
<tr>
<td><output name=direction></td>
<td><output name=status></td>
<td><output name=routing></td>
<td align right><output name=count></td>
<td align=right><output name=first type=recent></td>
<td align=right><output name=last type=recent></td>
</tr>
</sql>
</table>
<p>Results are cleared if more than 30 days since last test.</p>
'END'
