#!/usr/bin/perl
# img_trans.pl
# Transformiert die Bilder eines Verzeichnisses auf eine festgelegte Breite oder Hoehe

use strict;

sub Help {
	print <<EOS;
Aufruf: img_trans.pl [<opts>] source_1 ... target
<opt>
 -v          Ausfuehrliche Ausgabe.
 -lw<width>  Max. Breite im Querformat
 -lh<height> Max. Hoehe im Querformat
 -pw<width>  Max. Breite im Hochformat
 -ph<height> Max. Breite im Hochformat
 -sw<width>  Max. Breite bei quadratischen Bildern.
 -sh<height> Max. Breite bei quadratischen Bildern.
 -q<quality> Qualitaet in %
 -e<ext>     Dateityp. VE: jpg Nur relevant, wenn Quelle ein Verzeichnis.
source_X     Quelle: Datei oder Verzeichnis.
target       Ziel: Datei (dann nur genau Quelle) oder Verzeichnis.
Bsp:
img_trans png -phw100 -lh100 -e jpg /home/pic /home/pic/preview
EOS
	print "+++ ", $_[0], "\n";
	exit (1);
}

my $verbose = 0;
my $lwidth = 0;
my $lheight = 0;
my $pwidth = 0;
my $pheight = 0;
my $sheight = 0;
my $swidth = 0;
my $ext = "jpg";
my $quality = 0;

while ($ARGV[0] =~ /^-(.)(.*)/) {
  my ($opt, $val) = ($1, $2);
  if ($opt eq "v") {
	  $verbose = 1;
  } elsif ($opt eq "e"){
    $ext = $val;
  } elsif ($opt eq "q"){
    $quality = $val;
  } elsif ( ($opt eq "l" || $opt eq "p" || $opt eq "s") && $val =~ /([hw])(\d*)$/)  {
    my ($type, $val) = ($1, $2);
    if ($opt eq "l"){
      if ($type eq "w"){
	$lwidth = $val;
      } else {
	$lheight = $val;
      }
    } elsif ($opt eq "s") {
      if ($type eq "w"){
	$swidth = $val;
      } else {
	$sheight = $val;
      }
    } else {
      if ($type eq "w"){
	$pwidth = $val;
      } else {
	$pheight = $val;
      }
    }
  } else {
	  &Help ("unbekannte Option: $ARGV[0]");
  }
  shift;
} # while
&Help ("Keine Konvertierungsangabe gefunden (-pw -ph -lw -lh)")
	unless $lheight != 0 || $lwidth != 0 || $pheight != 0 || $pwidth != 0;

&Help ("Zu wenig Argumente") if $#ARGV < 1;
my $source = shift;
my $target = pop(@ARGV);

&Help ("Quelle existiert nicht: $source") unless -e $source;

if ($#ARGV < 0){
  if (-d $source){
    &Help ("Zielverzeichnis $target nicht ok (vorhanden?)") unless -d $target;
    &oneDir($source, $target);
  } else {
    my $trg = $target;
    $trg = &mkTarget($source, $target) if -d $trg;
    &oneFile($source, $trg);
  }
} else {
  &Help ("Zielverzeichnis $target nicht ok (vorhanden?)") unless -d $target;
  while ($source){
    if (-d $source){
      &oneDir($source, $target);
    } else {
      my $trg = &mkTarget($source, $target);
      &oneFile($source, $trg);
    }
    $source = shift;
  }
}
exit 0;
sub mkTarget{
  my $src = shift;
  my $trg = shift;

  $trg .= "/" if $trg !~ m!/$!;
  if ($src =~ m!/([^/]+)$!){
    $trg .= $1;
  } else {
    $trg .= $source;
  }
  return $trg;
}
sub oneDir{
  my $source = shift;
  my $target = shift;
  $source = substr($source, 0, length($source) - 2) if $source =~ m!/$!;
  $target .= "/" unless $target =~ m!/$!;
  opendir (DIR, $source) || die "$source: $!";
  my @files = readdir(DIR);
  close (DIR);

  my ($fn_src, $fn_trg, $info, $w, $h, $w2, $h2, $cmd);
  foreach (@files) {
	next if /^.{1,2}$/ || -d || ! /[.]$ext$/i;
	$fn_src = $source . "/" . $_;
	$fn_trg = $target . $_;
	&oneFile($fn_src, $fn_trg);
  } # foreach
}
sub oneFile{
  my $fn_src = shift;
  my $fn_trg = shift;

  my $info = `identify '$fn_src'`;
  if ($info eq ""){
	  die "+++ identify liefert nichts! Ist imagemagick installiert?"
  } elsif ($info !~ / (\d+)x(\d+) /) {
	  print "+++ kann Format nicht erkennen: $info\n";
  } else {
    my ($w, $h) = ($1, $2);
    my ($w2, $h2);
    if (abs($w - $h) < 2 || abs($w - $h) < $w * 5 / 100){
      # Quadratisch:
      if ($swidth > 0 && $swidth < $w){
	$w2 = $swidth;
	$h2 = int ($swidth * $h / $w + 0.5);
      } elsif ($sheight > 0 && $sheight < $h){
	$h2 = $sheight;
	$w2 = int ($sheight * $w / $h + 0.5);
      }
    } elsif ($w > $h){
      # Querformat
      if ($lwidth > 0 && $lwidth < $w){
	$w2 = $lwidth;
	$h2 = int ($lwidth * $h / $w + 0.5);
      } elsif ($lheight > 0 && $lheight < $h){
	$h2 = $lheight;
	$w2 = int ($lheight * $w / $h + 0.5);
      }
    } else {
      # Hochformat
      if ($pwidth > 0 && $pwidth < $w){
	$w2 = $pwidth;
	$h2 = int ($pwidth * $h / $w + 0.5);
      } elsif ($pheight > 0 && $lheight < $h){
	$h2 = $pheight;
	$w2 = int ($pheight * $w / $h + 0.5);
      }
    }
    my $cmd;
    if ($h2 > 0){
      my $q;
      $q = "-quality $quality" if $quality > 0;
      my @stat = stat($fn_src);
      my $size = $stat[7];
      $cmd = "convert '$fn_src' $q -resize ${w2}x$h2 '$fn_trg'";
      print $cmd, "\n" if $verbose;
      system ($cmd);
      @stat = stat($fn_trg);
      my $size2 = $stat[7];
      print sprintf ("%.3f kByte -> %.3f kByte\n", $size / 1000.0, $size2 / 1000.0) if $verbose;
    } else {
      print "$fn_src: Keine Konversion, da Breite=$w und Hoehe=$h\n" if $verbose;
      $cmd = "cp '$fn_src' '$fn_trg'";
      print "$cmd\n" if $verbose;
      system($cmd);
    }
  }
}
exit 1;
