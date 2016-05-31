#!/bin/perl

use strict;
use warnings;

my $s_count = 0;
my $t_count = 0;
my $before_blank = 1;

my @ll;
my $curr = 0;
while(<>){
        if(not /^$/){
                $s_count ++ if $before_blank;
                $before_blank = 0;
                $t_count ++;
		$curr ++;
        }
        else{
		$ll[$curr] ++;
                $before_blank = 1;
		$curr = 0;
        }
}

print "s:$s_count,t:$t_count\n";
for(my $i=0;$i < scalar @ll;$i++){
	if($ll[$i]){
		print "$i:$ll[$i]\n";
	}
	else{
		print "$i:0\n";
	}
}

