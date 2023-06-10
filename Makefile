find_sort3_algorithm_with_libFuzzer: find_sort3_algorithm_with_libFuzzer.c
	clang -O2 -Wall -mllvm -inline-threshold=2000000 -fsanitize=fuzzer -Wextra find_sort3_algorithm_with_libFuzzer.c -o find_sort3_algorithm_with_libFuzzer

clean:
	rm find_sort3_algorithm_with_libFuzzer
