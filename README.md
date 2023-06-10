# Optimizing sort3 with libFuzzer

This project is inspired by [Google DeepMind's](https://www.deepmind.com/blog/alphadev-discovers-faster-sorting-algorithms)
work on using AI to invent new algorithms for sorting integer arrays with 3 elements.

This project takes a similar approach, but instead of AI it uses [libFuzzer](https://www.llvm.org/docs/LibFuzzer.html).
The fuzzer input is treated as a sequence of instructions in a simple virtual instruction set.
If those instructions are able to successfully sort the 3 values in registers r0, r1, and r2, then
the program errors out, thereby notifying libFuzzer that a solution was found.

To build and run the fuzz target:

```bash
make
mkdir -p corpus
./find_sort3_algorithm_with_libFuzzer corpus -jobs=$(nproc) -workers=$(nproc) -max_len=11
```

When the fuzzer finds a solution, it creates a file with a name like `crash-1234`.
An example, named `crash-2ab6fce354320707f457d7a85f5a22b501e2ddbf`, is included.
You can confirm that the solution is valid like this:

```bash
./find_sort3_algorithm_with_libFuzzer crash-2ab6fce354320707f457d7a85f5a22b501e2ddbf
```
