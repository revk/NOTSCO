#!../login/envcgi /bin/csh -f
source ../script/loggedin
setenv CLIENTID `sql notsco 'SELECT clientid FROM tester WHERE ID="$TESTER"'`

echo "Set-Cookie: NOTSCO=$CLIENTID; HttpOnly; Secure; SameSite=Strict; Max-Age=0; Domain=$HTTP_HOST; Path=/"
echo "Content-Type: text/html"
echo "Refresh: 0;URL=https://$HTTP_HOST/"
echo ""
echo "Go <a href='/'>here</a>"
exit 0

fail:
echo "Status: 303"
echo "Location: https://$HTTP_HOST/"
echo ""
