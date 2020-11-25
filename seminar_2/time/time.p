# Gnuplot script for plotting data in files "merge.dat" and "small.dat"

set terminal png
set output "time.png"
set datafile separator whitespace
set key right top
set title "Execution time comparison between the basic and smaller header"
set xlabel "execution number"
set ylabel "execution time(S)"

plot col=1 "merge.dat" using 0:col title columnheader, \
     col=1 "small.dat" using 0:col title columnheader

