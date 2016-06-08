#!/bin/bash
gnuplot << EOF
set datafile separator ','

set key right bottom
set xlabel "Time in microseconds"
set ylabel "CDF"
set yrange [0:1]
set logscale x

set term postscript eps enhanced color 
set output "$2"
f(x)=0.95
set xrange [100:1000000]
plot "$1" using 1:2 notitle with linespoints, \
 f(x) w l lw 1 lt 2 title "95 pct"
EOF
