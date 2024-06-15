#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?PATH_INFO) goto fail
setenv CLIENTID "$PATH_INFO:t"
setenv TESTER `sql notsco 'SELECT ID FROM tester WHERE clientid="$CLIENTID"'`
if("$TESTER" == "") goto fail

echo "Status: 303"
echo "Set-Cookie: NOTSCO=$CLIENTID; HttpOnly; Secure; SameSite=Strict; Max-Age=3600; Domain=$HTTP_HOST; Path=/"
echo "Location: https://$HTTP_HOST/control.cgi"
echo ""
exit 0

fail:
echo "Status: 303"
echo "Location: https://$HTTP_HOST/"
echo ""
