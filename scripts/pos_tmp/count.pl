#!/bin/perl

use strict;
use warnings;

my $s_count = 0;
my $t_count = 0;
my $before_blank = 1;

while(<>){
        if(/\w/){
                $s_count ++ if $before_blank;
                $before_blank = 0;
                $t_count ++;
        }
        else{
                $before_blank = 1;
        }
}

print "s:$s_count,t:$t_count\n";

