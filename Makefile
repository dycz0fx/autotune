CC = mpicc
FLAGS = -O2

all:	bench_bcast	    \
		bench_ib	    \
		bench_sb        \
		bench_ibsb

clean: 	
		rm -rf			\
		bench_bcast	    \
		bench_ib	    \
		bench_sb        \
		bench_ibsb

bench_bcast: bench_bcast.c
	$(CC) $(FLAGS) bench_bcast.c -o bench_bcast

bench_ib: bench_ib.c
	$(CC) $(FLAGS) bench_ib.c -o bench_ib

bench_sb: bench_sb.c
	$(CC) $(FLAGS) bench_sb.c -o bench_sb

bench_ibsb: bench_ibsb.c
	$(CC) $(FLAGS) bench_ibsb.c -o bench_ibsb
