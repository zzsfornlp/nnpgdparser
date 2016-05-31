#!/bin/perl

use warnings;
use strict;

while(<>){
	my @l = split;
	if(scalar @l > 0){
		print "$l[0]\t$l[1]\t$l[1]\t$l[4]\t$l[4]\t_\t$l[8]\t$l[9]\n";
	}
	else{
		print "\n";
	}
}
