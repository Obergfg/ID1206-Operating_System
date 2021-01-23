set terminal png
set output "write2.png"
unset key

set xrange[-30:500]
set yrange[0:10000000]
set xtics 64

set xlabel "index in block"
set ylabel "time in ns"

set boxwidth 10
plot 'foo.dat' u 1:3:2:6:5 with candlesticks