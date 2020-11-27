# Gnuplot script for plotting data in file "merge.dat"

set terminal png
set output "merge.png"
set datafile separator whitespace
#set title "Free list development with merge"
set key right bottom
set ylabel "free list size"

set multiplot

set size 1,0.5
set origin 0.0,0.5
set xrange [30:1000]
set yrange[0:40]
plot col=1 "merge.dat" using 0:col with lines title columnheader

set origin 0.0,0.0
set xrange [30:1000]
set yrange[0:4000]
set key right top
set title ""
set xlabel "number of test iterations"
set ylabel "average free block size"
plot col=2 "merge.dat" using 0:col with lines title columnheader linecolor 2

unset multiplot
