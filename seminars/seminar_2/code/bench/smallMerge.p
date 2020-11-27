# Gnuplot script for plotting data in file "smallMerge.dat"

set terminal png
set output "smallMerge.png"
set datafile separator whitespace
#set title "Free list development of dlmalloc with small headers and merge"
set key right top
set ylabel "free list size"

set multiplot

set size 1,0.5
set origin 0.0,0.5
set xrange [30:1000]
set yrange[0:40]
plot col=1 "smallMerge.dat" using 0:col with lines title columnheader

set origin 0.0,0.0
set xrange [30:1000]
set yrange[0:4000]
set key right bottom
set title ""
set xlabel "number of test iterations"
set ylabel "average free block size"
plot col=2 "smallMerge.dat" using 0:col with lines title columnheader linecolor 2

unset multiplot
