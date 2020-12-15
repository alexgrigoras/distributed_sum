# Distributed sum

## Description
Distributed application of computing the sum of a matrix:
- Start with one root process (p0) and a matrix of integers of dimension m lines by n columns.
- The matrix is dynamically generated using random numbers.
- The m and n parameters are provided as program arguments.
- The goal is to obtain for each line i (i=1..m) the sum (si) of its elements.
- The root process starts by spawning m child processes (p1...pm). For each child process, it sends the
corresponding line in the matrix (li) as an array of n elements. A child process, computes the sum for
the input array and send it back to the parent process.
- Depending on the size of the received array, a child process does the following:
* if the array size is less than k (ex: 1000) elements, the child process computes the sum by itself
and sends the result back to its parent;
* otherwise, the child process splits the array in half, spawns two children of its own (thus
becoming itself a parent), sends each half to the corresponding child and waits for the results.
Upon receiving the partial sum results, it sums them and sends the result back to its parent.

## Implementation
Implemented in C using MPI.

Run using the command (on Linux):
```bash
mpirun -np 1 distributed_sum_master <m> <n> <k>
```

## License
MIT License.
