#! /usr/bin/perl

use strict;

my $lang = shift;
$lang = "py" unless $lang;
my $count = shift;
$count = 50*1000 unless $count;
my $loops = shift;
$loops = 200 unless $loops;

print "count: $count loops: $loops\n";
if ($lang =~ /^all/i){
	&pyAssign;
	&cppAssign;
	&javaAssign;
	&plAssign;
	&mfAssign;
} elsif ($lang =~ /^py/i){
	&pyAssign;
} elsif  ($lang =~ /^j/i){
	&javaAssign;
} elsif  ($lang =~ /^c/i){
	&cppAssign;
} elsif  ($lang =~ /^p.*l/i){
	&plAssign;
} elsif  ($lang =~ /^m/i){
	&mfAssign;
} else {
	die "unknown language: $lang. Use py pl java cpp";
}

exit 0;


sub pyAssign {
	print "pyAssign...\n";
	open (my $OUT, ">", "pybench.py") || die "pybench.py: $!";
	&pyPrefix($OUT);
	my $ix = 1;
	while($ix < $count){
		print $OUT sprintf("    variable%d = variable%d + 1\n", $ix, $ix - 1);
		$ix++;
	}
	print $OUT sprintf("    return variable%d\n", $ix - 1);
	&pySuffix($OUT);
	close $OUT;
}
sub pyPrefix{
	my $OUT = shift;
	print $OUT <<EOS;
import time
def doIt(defValue):
    variable0 = defValue
EOS
}
sub pySuffix{
	my $OUT = shift;
	print $OUT <<EOS;
if __name__ == '__main__':
    start = time.time()
    ix = $loops
    rc = 0
    while ix > 0 :
        ix -= 1
        rc += doIt(ix)
    end = time.time()
    print("rc", rc, " time of doIt: ", (end - start))
EOS
}

sub plAssign {
	print "plAssign...\n";
	open (my $OUT, ">", "plbench.pl") || die "plbench.pl: $!";
	&plPrefix($OUT);
	my $ix = 1;
	while($ix < $count){
		print $OUT sprintf("    my \$variable%d = \$variable%d + 1;\n", $ix, $ix - 1);
		$ix++;
	}
	&plSuffix($OUT);
	close $OUT;
}
sub plPrefix{
	my $OUT = shift;
	print $OUT <<'EOS';
use strict;
sub doIt{
    my $variable0 = shift;
EOS
}
sub plSuffix{
	my $OUT = shift;
	print $OUT <<EOS;
}
my \$start = time();
my \$rc = 0;
my \$ix = $loops;
while (\$ix--){
	\$rc += &doIt(\$ix);
}
my \$end = time();
print "rc: ", \$rc, " time of doIt: ", \$end - \$start, "\n";
EOS
}

sub javaAssign {
	print "javaAssign...\n";
	open (my $OUT, ">", "Bench.java") || die "Bench.java: $!";
	&javaPrefix($OUT);
	my $ix = 1;
	while($ix < $count){
		print $OUT sprintf("        int variable%d = variable%d + 1;\n", $ix, $ix - 1);
		$ix++;
	}
	print $OUT sprintf("        return variable%d;\n", $ix - 1);
	&javaSuffix($OUT);
	close $OUT;
}
sub javaPrefix{
	my $OUT = shift;
	print $OUT <<'EOS';
public class Bench {
    int doIt(int defValue){
    int variable0 = defValue;
EOS
}
sub javaSuffix{
	my $OUT = shift;
	print $OUT <<EOS;
    }
    public static void main(String[] args) {
        long start = System.currentTimeMillis();
        Bench bench = new Bench();
        int rc = 0;
        int ix = $loops;
        while(ix-- > 0)
			rc += bench.doIt(ix);
        long end = System.currentTimeMillis();
        System.out.println(String.format("rc: %d time of doIt(): %.3f", 
                rc, (end - start) / 1000.0));

    }
}
EOS
}

sub cppAssign {
	print "cppAssign...\n";
	open (my $OUT, ">", "cppbench.cpp") || die "cppbench.cpp: $!";
	&cppPrefix($OUT);
	my $ix = 1;
	while($ix < $count){
		print $OUT sprintf("        int variable%d = variable%d + 1;\n", $ix, $ix - 1);
		$ix++;
	}
	print $OUT sprintf("        return variable%d;\n", $ix - 1);
	&cppSuffix($OUT);
	close $OUT;
}
sub cppPrefix{
	my $OUT = shift;
	print $OUT <<'EOS';
#include <stdio.h>
#include <time.h>
class Bench {
public:
    int doIt(int defValue){
    int variable0 = defValue;
EOS
}
sub cppSuffix{
	my $OUT = shift;
	print $OUT <<EOS;
    }
};
int main(int argc, char**argv) {
    time_t start = time(NULL);
    Bench bench;
    int ix = $loops;
    int rc = 0;
    while(ix-- > 0){
		rc += bench.doIt(ix);
	}
    time_t end = time(NULL);
    printf("rc: %d time of doIt(): %d", rc, (end - start));
}
EOS
}

sub mfAssign {
	print "mfAssign...\n";
	open (my $OUT, ">", "mfbench.mf") || die "mfbench.mf: $!";
	&mfPrefix($OUT);
	my $ix = 1;
	while($ix < $count){
		print $OUT sprintf("        Int variable%d = variable%d + 1;\n", $ix, $ix - 1);
		$ix++;
	}
	print $OUT sprintf("        variable%d;\n", $ix - 1);
	&mfSuffix($OUT);
	close $OUT;
}
sub mfPrefix{
	my $OUT = shift;
	print $OUT <<'EOS';
func Int doIt(Int defValue) :
    Int variable0 = defValue;
EOS
}
sub mfSuffix{
	my $OUT = shift;
	print $OUT <<EOS;
endf
func Int main(List argv) :
    //time_t start = time(NULL);
    Int ix = $loops;
    Int rc = 0;
    while ix-- > 0 do
        rc += bench.doIt(ix);
    od
    //time_t end = time(NULL);
    //printf("rc: %d time of doIt(): %d", rc, (end - start));
endf
EOS
}


