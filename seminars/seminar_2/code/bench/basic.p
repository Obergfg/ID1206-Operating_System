# Gnuplot script for plotting data in file "basic.dat"

set terminal png
set output "basic.png"
set datafile separator whitespace
set key right center
#set title "Free list development without merge"

set ylabel "free list size"

set multiplot

set size 1,0.5
set origin 0.0,0.5
set yrange [0:400]
set xrange [30:1000]
plot col=1 "basic.dat" using 0:col with lines title columnheader

set origin 0.0,0.0
set yrange [0:4000]
set xrange [30:1000]
set xlabel "number of test iterations"
set title ""
set ylabel "average free block size"
plot col=2 "basic.dat" using 0:col with lines title columnheader linecolor 2

unset multiplot
