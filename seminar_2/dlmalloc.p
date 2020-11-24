# Gnuplot script for plotting data in file "dlmalloc.dat"

set terminal png
set output "dlmalloc.png"

set datafile separator whitespace
set key right center
set title "Free list development without merge"



set xlabel "number of dalloc and dfree requests "
set ylabel "free list size"

#plot "dlmalloc.dat" u 1:2 w linespoints title "free list"

set multiplot

set size 1,0.5
set origin 0.0,0.5
set yrange [0:400]
set xrange [30:1000]
plot col=1 "dlmalloc.dat" using 0:col with lines title columnheader

set origin 0.0,0.0
set yrange [0:4000]
set xrange [30:1000]

set title ""
set ylabel "average block size"
plot col=2 "dlmalloc.dat" using 0:col with lines title columnheader linecolor 2

unset multiplot

#plot for [col=1:2] "dlmalloc.dat" using 0:col with lines title columnheader