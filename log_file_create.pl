#!/usr/bin/perl
open(my $fh, ">", "log.txt")
   or die "cannot open < ~/log.txt: $!";
my $str1 = "INFO :- BAD BLOCK DETECTED\n";
my $len1 = length($str1);
my $str2 = "ERROR :- BAD USER DETECTED\n";
my $len2 = length($str2);
my $str3 = "WARNING :- BAD FC LINK IS DETECTED\n";
my $len3 = length($str3);
my $str4 = "SEVERE :- BAD BLOCK DETECTED IN THE FILE SYSTEM\n";
my $len4 = length($str4);
my $str5 = "CRITICAL :- BAD VOLINFO BLOCK DETECTED IN THE SNAPSHOT OF THE FILE SYSTEM\n";
my $len5 = length($str5);
my $sum1=$len1+$len2+$len3+$len4+$len5;
print "---------$sum1----------\n";
my $sum=0;
my $argument = shift;
while ($sum<$argument)
 {
   print $fh $str1;
   print $fh $str2;
   print $fh $str3;
   print $fh $str4;
   print $fh $str5;
   $sum+=$len1 + $len2 + $len3 + $len4 + $len5;
 }
close ($fh);




