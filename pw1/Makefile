sort: sort.o sort-64.s sort-32.s sort.i sort-64-clang.s sort-32-clang.s
	gcc sort.o -o sort
sort-64.s: sort.c 
	gcc -S sort.c -o sort-64.s
sort-32.s: sort.c
	gcc -S -m32 sort.c -o sort-32.s
sort-64-clang.s: sort.c 
	clang -S sort.c -o sort-64-clang.s
sort-32-clang.s: sort.c
	clang -S -m32 sort.c -o sort-32-clang.s
sort.i: sort.c
	gcc -E sort.c -o sort.i
sort.o: sort.c
	gcc -c sort.c -o sort.o
