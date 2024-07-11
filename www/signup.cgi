#!../login/envcgi /bin/csh -f
set path=(../xmlsql ../SQLlib $path)
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
setenv NEW ""
if("$LAST" == NULL) setenv NEW "First use of this link must be within 7 days."
/usr/sbin/sendmail -B8BITMIME -f "info@$HTTP_HOST" "$email" << END
From: NOSTCO <info@$HTTP_HOST>
To: CP <$email>
Subject: One Touch Switching test platform

You have requested a link for One Touch Switching test platform.

The link to access the test system is:-

	https://$HTTP_HOST/login.cgi/$CLIENTID

Keep this link to allow you access when needed.

$NEW

If you did not request this, please disregard this email.

-- 
NOTSCO Admin
END
echo "Emailed $email"
