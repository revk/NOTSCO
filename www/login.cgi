#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?PATH_INFO) goto fail
setenv CLIENTID "$PATH_INFO:t"
setenv TESTER `sql notsco 'SELECT ID FROM tester WHERE clientid="$CLIENTID"'`
if("$TESTER" == "") goto fail

setenv LAST `sql notsco 'SELECT lastlogin FROM tester WHERE ID="$TESTER"'`
sql notsco 'UPDATE tester SET lastlogin=NOW() WHERE ID="$TESTER"'

echo "Status: 303"
echo "Set-Cookie: NOTSCO=$CLIENTID; HttpOnly; Secure; SameSite=Strict; Max-Age=3600; Domain=$HTTP_HOST; Path=/"
if("$LAST" == NULL) then
	echo "Location: https://$HTTP_HOST/gettingstarted.cgi"
else
	echo "Location: https://$HTTP_HOST/control.cgi"
endif
echo ""
exit 0

fail:
echo "Status: 303"
echo "Location: https://$HTTP_HOST/"
echo ""
