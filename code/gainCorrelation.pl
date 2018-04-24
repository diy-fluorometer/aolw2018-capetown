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

foreach my $id (keys %readings) {
	if (exists($readings{$id}{MED}) && exists($readings{$id}{LOW})) {
		print "MED / LOW = ", $readings{$id}{MED} / $readings{$id}{LOW}, "\n";
	}
	if (exists($readings{$id}{HIGH}) && exists($readings{$id}{MED})) {
		print "HIGH / MED = ", $readings{$id}{HIGH} / $readings{$id}{MED}, "\n";
	}
	if (exists($readings{$id}{MAX}) && exists($readings{$id}{HIGH})) {
		print "MAX / HIGH = ", $readings{$id}{MAX} / $readings{$id}{HIGH}, "\n";
	}
	if (exists($readings{$id}{HIGH}) && exists($readings{$id}{LOW})) {
		print "HIGH / LOW = ", $readings{$id}{HIGH} / $readings{$id}{LOW}, "\n";
	}
	if (exists($readings{$id}{MAX}) && exists($readings{$id}{LOW})) {
		print "MAX / LOW = ", $readings{$id}{MAX} / $readings{$id}{LOW}, "\n";
	}
	if (exists($readings{$id}{MAX}) && exists($readings{$id}{MED})) {
		print "MAX / MED = ", $readings{$id}{MAX} / $readings{$id}{MED}, "\n";
	}	
}

close(F);