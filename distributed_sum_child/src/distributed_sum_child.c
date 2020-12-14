/*
 ============================================================================
 Name        : distributed_sum_child.c
 Author      : Alexandru Grigoras
 Version     : 1.0
 Copyright   : Copyright Alexandru Grigoras
 Description : Compute Distributed Sum using MPI
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

void print_array(int *array, int n, int myrank)
{
	int i;

	for (i = 0; i < n; i++)
	{
		printf("%d ", array[i]);
	}
	printf("\n");
}

int compute_sum(int *array, int n)
{
	int i, sum = 0;

	for (i = 0; i < n; i++)
	{
		sum += array[i];
	}

	return sum;
}

int main( int argc, char *argv[] ) {
	int myrank;
	int *array, *sum_array, *sub_array;
	int m;
	int n;
	int k;
	int sum = 0;
	int send_array[4];
	int NR_WORKERS = 2;
	int root = 0;
	MPI_Comm parentcomm;
	MPI_Comm workercomm;

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
	MPI_Comm_get_parent( &parentcomm );

	/* worker process code */

	printf("Worker (%d, %d): Spawned\n", getpid(), myrank);

	// Receive from parent
	MPI_Bcast( send_array, 4, MPI_INT, 0, parentcomm );
	printf("Worker (%d, %d): Spawned and received!\n", getpid(), myrank);
	m = send_array[0];
	n = send_array[1];
	k = send_array[2];
	root = send_array[3];

	array = malloc(n * sizeof(int));
	sum_array = malloc(sizeof(int));

	MPI_Scatter(array, n, MPI_INT, array, n, MPI_INT, 0, parentcomm);
	printf("Worker (%d, %d) with ROOT %d: Receiving: m = %d n = %d, k = %d and nr: ", getpid(), myrank, root, m, n, k);
	print_array(array, n, myrank);

	if(n <= k) // leaf worker
	{
		printf("Leaf Worker (%d, %d)\n", getpid(), myrank);
		// Compute sum of array
		sum = compute_sum(array, n);
		printf("Leaf Worker (%d, %d): Sum = %d\n", getpid(), myrank, sum);

		// Send sum to parent
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, 0, parentcomm);
	}
	else // internal worker
	{
		printf("Worker (%d, %d): Spawning other workers\n", getpid(), myrank);

		// Spawn workers
		MPI_Comm_spawn("distributed_sum_child", MPI_ARGV_NULL, NR_WORKERS, MPI_INFO_NULL, 0, MPI_COMM_SELF, &workercomm, MPI_ERRCODES_IGNORE);

		printf("Worker (%d, %d): Finished Spawning\n", getpid(), myrank);

		send_array[1] = n/2;
		send_array[3] = getpid();

		// Send matrix dimensions to workers
		MPI_Bcast(send_array, 4, MPI_INT, MPI_ROOT, workercomm);

		sub_array = malloc((n/2) * sizeof(int));

		// Send matrix line to workers
		MPI_Scatter(array, n/2, MPI_INT, sub_array, n/2, MPI_INT, MPI_ROOT, workercomm);

		sum_array = malloc(NR_WORKERS * sizeof(int));

		// Receive partial sums from workers
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, MPI_ROOT, workercomm);

		// Compute final sum
		sum = compute_sum(sum_array, NR_WORKERS);
		printf("Worker (%d, %d): Cumulative Sum = %d\n", getpid(), myrank, sum);

		// Send sum to parent
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, 0, parentcomm);

		MPI_Comm_free( &workercomm );
	}

	/* end worker process code */

	MPI_Comm_free( &parentcomm );

	MPI_Finalize( );

	return 0;
}
