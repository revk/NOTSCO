#!../login/envcgi /bin/csh -f
source ../script/loggedin

echo Content-Type: text/html
echo ""

xmlsql -d notsco head.html - tail.html << 'END'
<h1>Send bad messages</h1>
<p>In addition to these specific bad messages, you can also send invalid messages such as a match request by leaviung madatory fields blank.</p>

'END'
