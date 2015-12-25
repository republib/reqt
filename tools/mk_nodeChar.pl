#! /usr/bin/perl

use strict;
my (@index, @chars);

push @index, "const int ReCryptFileSystem::m_indexOfNodeChar[] = {";
push @chars, "const char ReCryptFileSystem::m_nodeChars[] = {";

for (0..31){
	push @index, "-1,";
}
my $ix = 0;
for (32..127){
	my $cc = chr($_);
	if ($cc !~ m!["*:?<>;\\/]!){
		push @index, $ix++ . ",";
		if ($cc eq "'"){
			push @chars, "'\\'',"; 
		} else {
			push @chars, "'$cc',"; 
		}
	} else {
		push @index, "-1,";
	}
}
print join("\n", @index), "\n";
print "};\n";
print join("\n", @chars), "\n";
print "};\n";
