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
	int n;
	int k;
	int sum = 0;
	int tag = 0;
	int nm_array[2];
	int NR_WORKERS = 2;
	MPI_Comm parentcomm;
	MPI_Comm workercomm2;
	MPI_Status status ;	/* return status for receive */

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
	MPI_Comm_get_parent( &parentcomm );

	/* worker process code */

	printf("Worker %d - Spawned\n", myrank);

	// Receive array dimension and threshold k
	MPI_Bcast( nm_array, NR_WORKERS, MPI_INT, MPI_ROOT, parentcomm );
	n = nm_array[0];
	k = nm_array[1];

	array = malloc(n*sizeof(int));
	sum_array = malloc(sizeof(int));

	// Receive array
	MPI_Scatter(array, n, MPI_INT, array, n, MPI_INT, MPI_ROOT, parentcomm);
	printf("Final Worker %d: Receiving: ", myrank);
	print_array(array, n, myrank);

	if(n <= k) {
		// Compute sum of array
		sum = compute_sum(array, n);
		printf("Final Worker %d: Sum = %d\n", myrank, sum);

		// Send sum to parent
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, 0, parentcomm);
	}
	else {
		printf("Worker %d: Receiving: ", myrank);
		print_array(array, n, myrank);
		printf("Worker %d: Spawning other workers\n", myrank);

		// Spawn workers
		MPI_Comm_spawn( "distributed_sum_child", MPI_ARGV_NULL, NR_WORKERS, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &workercomm2, MPI_ERRCODES_IGNORE );

		nm_array[0] = n/2;

		// Send matrix dimensions to workers
		MPI_Bcast(nm_array, NR_WORKERS, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm2);

		sub_array = malloc(n*sizeof(int));

		// Send matrix line to worker
		MPI_Scatter(array, n/2, MPI_INT, sub_array, n/2, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm2);

		sum_array = malloc(NR_WORKERS * sizeof(int));

		// Receive partial sums from workers
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm2);

		// Compute final sum
		sum = compute_sum(sum_array, NR_WORKERS);
		printf("Worker %d: Cumulative Sum = %d\n", myrank, sum);

		// Send sum to parent
		MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, 0, parentcomm);

		MPI_Comm_free( &workercomm2 );
	}

	/* worker process code */
	MPI_Comm_free( &parentcomm );

	MPI_Finalize( );

	return 0;
}
