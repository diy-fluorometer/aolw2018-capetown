#!/usr/bin/perl

use strict;
use IO::Socket;

sub usage() {
	return "usage: $0 <host> <port> <numreadings> <runname>";
}

my $host = shift @ARGV or die usage();
my $port = shift @ARGV or die usage();
my $n = shift @ARGV or die usage();
my $name = shift @ARGV or die usage();

my $sock = new IO::Socket::INET(
	PeerAddr => $host,
	PeerPort => $port,
	Proto => "tcp"
);

if (!$sock) {
	die "Could not open socket to $host:$port";
}

print $sock "GET /run/start?n=$n&name=$name HTTP/1.1\n\n";
close($sock);

sleep(5);

$sock = new IO::Socket::INET(
	PeerAddr => $host,
	PeerPort => $port,
	Proto => "tcp"
);

if (!$sock) {
	die "Could not open socket to $host:$port";
}

print $sock "GET /run/fetch HTTP/1.1\n\n";

my @response = <$sock>;

print @response;

close($sock);
