gcc -o clist clist.c -lpthread

name="pcs" 

echo $name > ${name}.dat 

for j in {1..8}
do
    for i in {1..10}
    do
        ./clist 5000 ${j} >> temp.dat
    done

    awk 'BEGIN {total=0} {total+=$1} END {printf("%.2f\n",total/(NR))}' temp.dat >> ${name}.dat
    rm temp.dat
done

