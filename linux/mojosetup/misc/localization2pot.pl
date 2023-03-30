#!/usr/bin/perl -w

use warnings;
use strict;
use Encode qw( decode_utf8 );

# Fixes unicode dumping to stdio...hopefully you have a utf-8 terminal by now.
binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");
binmode(STDERR, ":utf8");

my $now = `date '+%Y-%m-%d %H:%M:%S%z'`;
chomp($now);

my $svnver = `svnversion . 2>/dev/null`;
chomp($svnver);
$svnver = 'svn-' . (($svnver eq '') ? '???' : $svnver);

my $exportdate = '';
my $generator = '';
while (<STDIN>) {
    chomp;
    if (s/\A\-\-\s*(X-Launchpad-Export-Date: .*?)\Z/"$1\\n"/) {
        $exportdate = $_;
        next;
    }
    if (s/\A\-\-\s*(X-Generator: .*?)\Z/"$1\\n"/) {
        $generator = $_;
        next;
    }
    last if (/MojoSetup.localization = {/);
}


print <<__EOF__;
# MojoSetup; a portable, flexible installation application.
#    http://icculus.org/mojosetup/
#
# Please see the file LICENSE.txt in the source's root directory.
#
# This file generated by localization2pot.pl, version $svnver ...
#
#
# NOTE: If you care about Unicode or ASCII chars above 127, this file _MUST_
#  be UTF-8 encoded! If you think you're using a certain high-ascii codepage,
#  you're wrong!
#
# Whenever you see a %x sequence, that is replaced with a string at runtime.
#  So if you see, "Hello, %0, my name is %1.", then this might become
#  "Hello, Alice, my name is Bob." at runtime. If your culture would find
#  introducing yourself second to be rude, you might translate this to:
#  "My name is %1, hello %0." If you need a literal '%' char, write "%%":
#  "Operation is %0%% complete" might give "Operation is 3% complete."
#  All strings, from your locale or otherwise, are checked for formatter
#  correctness at startup. This is to prevent the installer working fine
#  in all reasonable tests, then finding out that one guy in Ghana has a
#  crashing installer because his localization forgot to add a %1 somewhere.
#
# Occasionally you might see a "\\n" ... that's a newline character. "\\t" is
#  a tab character, and "\\\\" turns into a single "\\" character.
#
# Questions about the intent of a specific string can go to Ryan C. Gordon
#  (icculus\@icculus.org).

#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: mojosetup $svnver\\n"
"Report-Msgid-Bugs-To: icculus\@icculus.org\\n"
"POT-Creation-Date: $now\\n"
"PO-Revision-Date: $now\\n"
"Last-Translator: Ryan C. Gordon <icculus\@icculus.org>\\n"
"Language-Team: Ryan C. Gordon <icculus\@icculus.org>\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
$exportdate
$generator

__EOF__


my $looking_for_end = 0;

while (<STDIN>) {
    chomp;
    s/\A\s+//;
    s/\s+\Z//;
    next if ($_ eq '');
    if (/\A};\Z/) {
        last if (not $looking_for_end);
        $looking_for_end = 0;
        next;
    } else {
        next if $looking_for_end;
    }

    if (s/\A\[(\".*?\")\] = {\Z/msgid $1\nmsgstr ""\n/) {
        print("$_\n");
        $looking_for_end = 1;
    } elsif (s/\A--\s+(.*?)\Z/#. $1/) {
        print("$_\n");
    } else {
        die("unhandled string: '$_'\n");
    }
}

print "# end of localization.pot ...\n\n";

# end of localization2pot.pl ...

