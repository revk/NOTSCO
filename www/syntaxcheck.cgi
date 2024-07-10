#!../login/envcgi /bin/csh -fx
source ../script/loggedin

if($?JSON) then
        setenv CHECK `printenv JSON | ../bin/notsco`
endif

echo Content-Type: text/html
echo ""
xmlsql --exec head.html - tail.html << 'END'
<h1>Syntax check JSON</h1>
<p>Paste JSON of a message to run through syntax check.</p>
<form method=post>
<table>
<tr>
<td valign=top><textarea name=JSON rows=40 cols=40 placeholder='{"envelope":{...'></textarea></td>
<if JSON><td valign=top style='font-weight:bold;'><exec ../bin/notsco JSON></td></if>
</tr>
</table>
<input type=SUBMIT name=SEND value="Check">
</form>
'END'
