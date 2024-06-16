#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?email) setenv email ""
if("$email" == "") then
	echo "Location: /"
	echo ""
	exit 0
endif
echo "Content-Type: text/plain"
echo "Refresh: 5;URL=/"
echo ""
setenv TESTER `sql notsco 'SELECT ID FROM tester WHERE email="$email"'`
if("$TESTER" == "") then
	setenv TESTER `sql -i notsco 'INSERT INTO tester SET ID=0,email="$email",clientid=hex(random_bytes(10)),clientsecret=hex(random_bytes(32))'`
endif
setenv CLIENTID `sql notsco 'SELECT clientid FROM tester WHERE ID="$TESTER" AND (emailed is NULL or emailed<date_sub(now(),interval 10 minute))'`
if("$CLIENTID" == "")	then
	echo "Rate limit, try again in 10 minutes."
	exit 0
endif
setenv LAST `sql notsco 'SELECT lastlogin FROM tester WHERE ID="$TESTER"'`
sql notsco 'UPDATE tester SET emailed=NOW() WHERE email="$email"'
xmlsql << END | /usr/sbin/sendmail -B8BITMIME -f "info@$HTTP_HOST" "$email"
From: <info@$HTTP_HOST> "NOTSCO"
To: <$email> "CP"
Subject: One Touch Switching test platform

You have requested a link for One Touch Switching test platform.

The link to access the test system is:-

	https://$HTTP_HOST/login.cgi/$CLIENTID

Keep this link to allow you access when needed.

<IF LAST=NULL> Use this link within 7 days, or you will have to request a new one.
</IF>
If you did not request this, please disregard this email.

-- 
NOTSCO Admin
END
echo "Emailed $email"
