#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?PATH_INFO) goto fail
setenv CLIENTID "$PATH_INFO:t"
setenv TESTER `sql notsco 'SELECT ID FROM tester WHERE clientid="$CLIENTID"'`
if("$TESTER" == "") goto fail

echo "Set-Cookie: NOTSCO=$CLIENTID; HttpOnly; Secure; SameSite=Strict; Max-Age=3600; Domain=$HTTP_HOST; Path=/"
echo "Content-Type: text/html"
echo "Refresh: 0;URL=/control.cgi"
echo ""
echo "Go <a href='/control.cgi'>here</a>"
exit 0

fail:
echo "Location: /"
echo ""
