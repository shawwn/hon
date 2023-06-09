#!/bin/bash

# (c) CopyRight EdelWeb for EdelKey and OpenEvidence, 2000-2004, 2009
# Author: Peter Sylvester

# "libre" for integration with curl 

OPENSSL=openssl
if [ -f /usr/local/ssl/bin/openssl ] ; then
OPENSSL=/usr/local/ssl/bin/openssl
fi

USAGE="echo Usage is genroot.sh \<name\>"

HOME=`pwd`
cd $HOME

KEYSIZE=2048
DURATION=6000

PREFIX=$1
if [ ".$PREFIX" = . ] ; then
	echo No configuration prefix
	NOTOK=1
else
    if [ ! -f $PREFIX-ca.prm ] ; then
	echo No configuration file $PREFIX-ca.prm
	NOTOK=1
    fi
fi

if [ ".$NOTOK" != . ] ; then
    echo "Sorry, I can't do that for you."
    $USAGE
    exit
fi

GETSERIAL="\$t = time ;\$d =  \$t . substr(\$t+$$ ,-4,4)-1;print \$d"
SERIAL=`/usr/bin/env perl -e "$GETSERIAL"`

echo SERIAL=$SERIAL PREFIX=$PREFIX DURATION=$DURATION KEYSIZE=$KEYSIZE 

echo "openssl req -config $PREFIX-ca.prm -newkey rsa:$KEYSIZE -keyout $PREFIX-ca.key -out $PREFIX-ca.csr"
$OPENSSL req -config $PREFIX-ca.prm -newkey rsa:$KEYSIZE -keyout $PREFIX-ca.key -out $PREFIX-ca.csr

echo "openssl x509 -set_serial $SERIAL -extfile $PREFIX-ca.prm -days $DURATION -req -signkey $PREFIX-ca.key -in $PREFIX-ca.csr -out $PREFIX-$SERIAL.ca-cacert -sha1 "

$OPENSSL x509  -set_serial $SERIAL -extfile $PREFIX-ca.prm -days $DURATION -req -signkey $PREFIX-ca.key -in $PREFIX-ca.csr -out $PREFIX-$SERIAL-ca.cacert -sha1 

echo "openssl x509 -text -hash -out $PREFIX-ca.cacert -in $PREFIX-$SERIAL-ca.cacert -nameopt multiline"
$OPENSSL x509 -text -hash -out $PREFIX-ca.cacert -in $PREFIX-$SERIAL-ca.cacert -nameopt multiline

echo "openssl x509 -in $PREFIX-ca.cacert -outform der -out $PREFIX-ca.der "
$OPENSSL x509 -in $PREFIX-ca.cacert -outform der -out $PREFIX-ca.der 

echo "openssl x509 -in $PREFIX-ca.cacert -text -out $PREFIX-ca.crt -nameopt multiline"

$OPENSSL x509 -in $PREFIX-ca.cacert -text -out $PREFIX-ca.crt -nameopt multiline

echo "openssl x509 -noout -text -in $PREFIX-ca.cacert -nameopt multiline"
$OPENSSL x509 -noout -text -in $PREFIX-ca.cacert -nameopt multiline

#$OPENSSL rsa -in ../keys/$PREFIX-ca.key -text -noout -pubout
