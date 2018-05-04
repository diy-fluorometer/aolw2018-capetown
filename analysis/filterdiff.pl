#!/usr/bin/perl

use strict;
use Data::Dumper;

my $f1 = shift @ARGV or die;
my $f2 = shift @ARGV or die;
my $out = shift @ARGV or die;

open(F1, $f1) or die "$!";
open(F2, $f2) or die "$!";
open(OUT, ">$out") or die "$!";

my %ds1;
my %ds2;

my $line;

<F1>;
while ($line = <F1>) {
	chomp $line;
	next if ($line =~ /^#/);
	my ($name, $seqid, $value, $gain) = split(/,/, $line);
	$ds1{$name} = [] if not exists($ds1{$name});
	$ds1{$name}[$seqid] = {} if not defined($ds1{$name}[$seqid]);
	$ds1{$name}[$seqid]{$gain} = $value;
}

close(F1);

<F2>;
while ($line = <F2>) {
	chomp $line;
		next if ($line =~ /^#/);

	my ($name, $seqid, $value, $gain) = split(/,/, $line);
	$ds2{$name} = [] if not exists($ds2{$name});
	$ds2{$name}[$seqid] = {} if not defined($ds2{$name}[$seqid]);
	$ds2{$name}[$seqid]{$gain} = $value;
}

close(F2);

# print header
print OUT "name,seqid,value,gain\n";
print OUT "# diff of corresponding values in: \n" .
	"# $f1\n" .
	"# $f2\n";
#print Dumper(%ds1);
#print Dumper(%ds2);
#exit;

foreach my $name (keys %ds1) {
	for (my $id = 0; $id < @{$ds1{$name}}; $id++) {
		foreach my $gain (keys %{$ds1{$name}[$id]}) {
			if (exists($ds2{$name}) and defined($ds2{$name}[$id]) and exists($ds2{$name}[$id]{$gain})) {
				print OUT "$name,$id,", $ds1{$name}[$id]{$gain} - $ds2{$name}[$id]{$gain}, ",$gain\n";
				print $ds1{$name}[$id]{$gain}, " - ", $ds2{$name}[$id]{$gain}, " = ", $ds1{$name}[$id]{$gain} - $ds2{$name}[$id]{$gain}, "\n";
			}
		}
	}
}

close(OUT);