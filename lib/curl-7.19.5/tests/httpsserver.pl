#!/usr/bin/env perl
#
# $Id: httpsserver.pl,v 1.1 2009/08/09 13:17:33 jason Exp $
# This is the HTTPS and FTPS server designed for the curl test suite.
#
# It is actually just a layer that runs stunnel properly.

use strict;
use Cwd;

my $stunnel = "stunnel";

#
# -p pemfile
# -P pid dir
# -d listen port
# -r target port
# -s stunnel path

my $verbose=0; # set to 1 for debugging

my $port = 8991;        # just our default, weird enough
my $target_port = 8999; # default test http-server port

my $path = getcwd();

my $srcdir=$path;

my $proto='https';

while(@ARGV) {
    if($ARGV[0] eq "-v") {
        $verbose=1;
    }
    if($ARGV[0] eq "-w") {
        return 0; # return success, means we have stunnel working!
    }
    elsif($ARGV[0] eq "-p") {
        $proto=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] eq "-r") {
        $target_port=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] eq "-s") {
        $stunnel=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] eq "-d") {
        $srcdir=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] =~ /^(\d+)$/) {
        $port = $1;
    }
    shift @ARGV;
};

my $conffile="$path/stunnel.conf";	# stunnel configuration data
my $certfile="$srcdir/stunnel.pem";	# stunnel server certificate
my $pidfile="$path/.$proto.pid";	# stunnel process pid file

# find out version info for the given stunnel binary
my $ver_major;
my $ver_minor;
foreach my $veropt (('-version', '-V')) {
    foreach my $verstr (qx($stunnel $veropt 2>&1)) {
        if($verstr =~ /^stunnel (\d+)\.(\d+) on /) {
            $ver_major = $1;
            $ver_minor = $2;
            last;
        }
    }
    last if($ver_major);
}

my $cmd;
if(!$ver_major) {
    print STDERR "no stunnel or unknown version\n";
}
elsif($ver_major < 4) {
    # stunnel version less than 4.00
    $cmd  = "$stunnel -p $certfile -P $pidfile -d $port -r $target_port ";
    $cmd .= "2>/dev/null";
}
else {
    # stunnel version 4.00 or later
    $cmd  = "$stunnel $conffile ";
    $cmd .= "2>/dev/null";
    # stunnel configuration file
    open(STUNCONF, ">$conffile") || exit 1;
    print STUNCONF "
	CApath = $path
	cert = $certfile
	pid = $pidfile
	debug = 0
	output = /dev/null
	foreground = yes
	
	[curltest]
	accept = $port
	connect = $target_port
	";
    close STUNCONF;
}

if($verbose) {
    print uc($proto)." server: $cmd\n";
}

my $rc = system($cmd);

$rc >>= 8;
if($rc) {
    print STDERR "stunnel exited with $rc!\n";
}

unlink $conffile;

exit $rc;
