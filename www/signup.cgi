#!../login/envcgi /bin/csh -f
set path=($path ../xmlsql ../SQLlib)
if(! $?email) then
	echo "Location: /"
	echo ""
	exit 0
endif
echo "Location: /control.cgi"
echo ""
