#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?email) then
	echo "Location: /"
	echo ""
	exit 0
endif
setenv TESTER `sql notsco 'SELECT ID FROM tester WHERE email="$email"'`
if("$TESTER" == "") then
	setenv TESTER `sql -i notsco 'INSERT INTO tester SET ID=0,email="$email",clientid=hex(random_bytes(10)),clientsecret=hex(random_bytes(32))'`
endif
setenv CLIENTID `sql notsco 'SELECT clientid FROM tester WHERE ID="$TESTER"'`
/usr/sbin/sendmail -B8BITMIME -f "info@$HTTP_HOST" "$email" << END
From: <info@$HTTP_HOST> "NOTSCO"
To: <$email> "CP"
Subject: One Touch Switching test platform

You have requested a link for One Touch Switching test platform.

The link to access the test system is:-

	https://$HTTP_HOST/login.cgi/$CLIENTID

Keep this link to allow you access when needed.

If you did not request this, please disregard this email.

-- 
NOTSCO Admin
END
echo "Content-Type: text/plain"
echo "Refresh: 5;URL=/"
echo ""
echo "Emailed $email"
