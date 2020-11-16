autotune="/home/dycz0fx/program/autotune"
config="$autotune/config"
output="$HOME/.openmpi"
result="${output}/autotune"

source $config

if [ ! -d ${output} ] 
then
    mkdir $output
fi

if [ ! -d ${result} ] 
then
    mkdir $result
fi

for n in "${nodes[@]}"
do
    np=$n
    echo "n $n"
    for umod in "${umods[@]}"
    do
        echo "umod $umod"
        eval "mpirun -np $np --map-by ppr:1:node --timeout $timeout --bind-to core --report-bindings --mca coll_${umod}_priority 40 $autotune/bench_ib --min_msg $min_msg --min_seg $min_seg --max_seg $max_seg --iters $iters > $result/bench_ib.${n}_${umod}"

    done
done

for c in "${ppns[@]}"
do
    np=$c
    for lmod in "${lmods[@]}"
    do
        echo "lmod $lmod"
        eval "mpirun -np $np --map-by ppr:$c:node --timeout $timeout --bind-to core --report-bindings --mca coll_${lmod}_priority 40 $autotune/bench_sb --min_msg $min_msg --min_seg $min_seg --max_seg $max_seg --iters $iters > $result/bench_sb.${c}_${lmod}"
    done
done


for n in "${nodes[@]}"
do
    for c in "${ppns[@]}"
    do
        np=$((n*c))
        echo "n $n"
        echo "c $c"
        echo "np $np"
        for umod in "${umods[@]}"
        do
            for lmod in "${lmods[@]}"
            do
                echo "umod $umod"
                echo "lmod $lmod"                
                eval "mpirun -np $np --map-by ppr:$c:node --timeout $timeout --bind-to core --report-bindings --mca coll_${lmod}_priority 50 --mca coll_${umod}_priority 40 $autotune/bench_ibsb --min_msg $min_msg --min_seg $min_seg --max_seg $max_seg --iters $iters > $result/bench_ibsb.${n}_${c}_${umod}_${lmod}"
            done
        done
    done
done