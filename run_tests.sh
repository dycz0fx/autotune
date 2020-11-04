folder="/home/dycz0fx/programs/test/autotune"
result="/home/dycz0fx/programs/test/autotune/results"

module load papi
export PATH=/home/dycz0fx/opt/ompi/release/bin:$PATH
export LD_LIBRARY_PATH=/home/dycz0fx/opt/ompi/release/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=/home/dycz0fx/opt/ompi/release/include:$C_INCLUDE_PATH

nodes=(2 4 8 16)
ppns=(6 12)
#imod: 0 - libnbc, 1 - adapt
imods=(0, 1)
#smod: 0 - sm, 1 - solo
smods=(0)
#adapt_algs_small: 1 - binomial, 3 - binary
adapt_algs_small=(1 3)
#adapt_algs_big: 3 - binary, 4 - chain
adapt_algs_big=(3 4)
adapt_segs=(4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304)

for n in "${nodes[@]}"
do
    for c in "${ppns[@]}"
    do
        np=$((n*c))
        echo "n $n"
        echo "c $c"
        echo "np $np"
        for imod in "${imods[@]}"
        do
            for smod in "${smods[@]}"
            do
                if [ $imod -eq 0 ] && [ $smod -eq 0 ]; then
                    echo "imod $imod"
                    echo "smod $smod"
                    mpirun -np $np -npernode $c --timeout 200 --bind-to core --report-bindings --mca coll_sm_priority 50 $folder/bench_ibsb --min 4 --max 4194304 --seg 4096 --iters 10 >> $result/ib_ibsb_sb.${n}_${c}_${imod}_${smod}
                fi
                if [ $imod -eq 0 ] && [ $smod -eq 1 ]; then
                    echo "imod $imod"
                    echo "smod $smod"
                    mpirun -np $np -npernode $c --timeout 200 --bind-to core --report-bindings --mca coll_shared_priority 50 $folder/bench_ibsb --min 4 --max 4194304 --seg 4096 --iters 10 >> $result/ib_ibsb_sb.${n}_${c}_${imod}_${smod}
                fi
                if [ $imod -eq 1 ] && [ $smod -eq 0 ]; then
                    echo "imod $imod"
                    echo "smod $smod"
                    for alg in "${algs_big[@]}"
                    do
                        for bseg in "${segs[@]}"
                        do
                            for rseg in "${segs[@]}"
                            do
                                echo "alg $alg"
                                echo "bseg $bseg"
                                echo "rseg $rseg"
                                mpirun -np $np -npernode $c --timeout 200 --bind-to core --report-bindings --mca pml cm --mca coll_sm_priority 50 --mca coll_adapt_priority 40 --mca coll_adapt_bcast_algorithm $alg --mca coll_adapt_bcast_segment_size $bseg --mca coll_adapt_reduce_algorithm $alg --mca coll_adapt_reduce_segment_size $rseg  $folder/bench_ibsb --min 4095 --max 4194304 --iters 10 >> $result/ib_ibsb_sb.${n}_${c}_${imod}_${smod}_${alg}_${bseg}_${rseg}
                            done
                        done
                    done
                fi
                if [ $imod -eq 1 ] && [ $smod -eq 1 ]; then
                    echo "imod $imod"
                    echo "smod $smod"
                    for alg in "${algs_big[@]}"
                    do
                        for bseg in "${segs[@]}"
                        do
                            for rseg in "${segs[@]}"
                            do
                                echo "alg $alg"
                                echo "bseg $bseg"
                                echo "rseg $rseg"
                                mpirun -np $np -npernode $c --timeout 200 --bind-to core --report-bindings --mca pml cm --mca coll_shared_priority 50 --mca coll_adapt_priority 40 --mca coll_adapt_bcast_algorithm $alg --mca coll_adapt_bcast_segment_size $bseg --mca coll_adapt_reduce_algorithm $alg --mca coll_adapt_reduce_segment_size $rseg  $folder/bench_ibsb --min 4095 --max 4194304 --iters 10 >> $result/ib_ibsb_sb.${n}_${c}_${imod}_${smod}_${alg}_${bseg}_${rseg}
                            done
                        done
                    done
                fi
            done
        done
    done
done

now=$(date +"%T")
echo "End time : $now"