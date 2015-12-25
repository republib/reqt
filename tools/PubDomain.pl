#! /usr/bin/perl
#
# Replaces the file header comment of *.cpp + *.hpp files with
# a public domain license.
# all files of the current directory and its subdirectory will be modified.
#

use strict;

my $s_delim = "/";
my $s_template = "/*
 * %NODE%
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
";

&oneDir(".");
exit 0;


sub oneFile(){
	my $filename = shift;
	my $node = shift;
	
	my $comment = $s_template;
	$comment =~ s/%NODE%/$node/;
	my @expected = split(/\n/, $comment);
	
	open(my $INP, "<", $filename) || die "$filename: $!";
	my @lines = <$INP>;
	close $INP;
	my $ix;
	my $equal = 1;
	for ($ix = 0; $equal && $ix <= $#expected; $ix++){
		$equal = $expected[$ix] . "\n" eq $lines[$ix];
	}
	if (! $equal){
		my $org = $lines[$ix-1];
		chomp $org;
		my $exp = $expected[$ix-1];
		chomp $exp;
		my $col = 0;
		while (substr($org, $col, 1) eq substr($exp, $col, 1)){
			 $col++;
		}
		$col++;
		print "$filename differs in line $ix-$col:\n'$org'\n'$exp'\n",
		'-' x $col, "^\n";
		# Remove the first comment:
		if ($lines[0] =~ m!^/\*!){
			my $stop = 0;
			while (! $stop && $#lines >= 0){
				$stop = index($lines[0], "*/") >= 0;
				shift @lines;
			}
		}
		open(my $OUT, ">", $filename) || die "$filename: $!";
		print $OUT $comment;
		print $OUT @lines;
	}
}
sub oneDir{
  my $dir = shift;
  
  opendir (DIR, $dir) || die;
  my @files = readdir (DIR);
  closedir(DIR);
  
  my $node;
  foreach $node(@files){
    my $fn = $dir . $s_delim . $node;
    next unless $node =~ /\.(cpp|hpp|java)$/;
    &oneFile($fn, $node);
  }
  
  foreach $node(@files){
    my $fn = $dir . $s_delim . $node;
    next unless -d $fn;
    next if $node =~ /^[.]{1,2}$/;
    &oneDir($fn);
  }
}


