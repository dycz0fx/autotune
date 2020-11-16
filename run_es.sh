autotune="/home/dycz0fx/program/autotune"
config="$autotune/config"
output="$HOME/.openmpi"
result="${output}/autotune"

source $config

segs=($min_seg)
while [ ${segs[-1]} -lt $max_seg ]
do
    segs+=($((${segs[-1]}*2)))
done

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
    for c in "${ppns[@]}"
    do
        np=$((n*c))
        echo "n $n"
        echo "c $c"
        echo "np $np"
        for umod_id in "${!umods[@]}"
        do
            for lmod_id in "${!lmods[@]}"
            do
                for seg in "${segs[@]}"
                do
                    umod=${umods[$umod_id]}
                    lmod=${lmods[$lmod_id]}
                    echo "umod $umod"
                    echo "lmod $lmod"
                    echo "seg $seg"
                    eval "mpirun -np $np --map-by ppr:$c:node --timeout $timeout --bind-to core --report-bindings --mca coll_han_priority 50 --mca coll_han_bcast_up_module $umod_id --mca coll_han_bcast_low_module $lmod_id --mca coll_han_bcast_segsize $seg $autotune/bench_bcast --min_msg $min_msg --max_msg $max_msg --iters $iters > $result/bench_bcast.${n}_${c}_${umod}_${lmod}_${seg}"
                done
            done
        done
    done
done

