#!/usr/bin/perl

# Reads a file that contains the output from the basic Arduino sketch gainTest
# and pulls out sets of readings at different gain levels, and evaluates them.

use strict;

my %gains = (
	LOW => 1,
	MED => 25,
	HIGH => 428,
	MAX => 9876
	);

my %readings;

my $file = shift @ARGV or die "usage: $0 <file>";
open(F, $file) or die "$!";
my $line;
while ($line = <F>) {
	chomp $line;
	if ($line =~ /(\w+)\s[^,]+,(\d+),(\d+)/) {
		my $gain_level = $1;
		my $seqid = $2;
		my $value = $3;
		$readings{$seqid} = {} unless exists($readings{$seqid});
		$readings{$seqid}{$gain_level} = $value;
	} else {
		print STDERR "Parse error: $line\n";
		next;
	}
}

print "GLOW	GMED	GHIGH	GMAX\n";
foreach my $id (keys %readings) {
	if (exists($readings{$id}{LOW})) { print "$readings{$id}{LOW}\t"; } else { print "\t"}
	if (exists($readings{$id}{MED})) { print "$readings{$id}{MED}\t"; } else { print "\t"}
	if (exists($readings{$id}{HIGH})) { print "$readings{$id}{HIGH}\t"; } else { print "\t"}
	if (exists($readings{$id}{MAX})) { print "$readings{$id}{MAX}"; } else { print ""} print "\n";
}

close(F);