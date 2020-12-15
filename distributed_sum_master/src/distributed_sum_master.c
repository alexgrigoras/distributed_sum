/*
 ============================================================================
 Name        : distributed_sum_master.c
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

void generate_array(int *array, int m, int n)
{
	int i, j;
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
	    {
			array[i*n+j] = rand() % 50;
	    }
	}
}

void print_array(int *array, int n)
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

int main(int argc, char* argv[]){
	int myrank;
	int m, n, k;
	int *array, *sub_array, *sum_array;
	int sum = 0;
	int send_array[4];
	MPI_Comm workercomm;
	
	if ( argc != 4 )
	{
		printf( "Usage: %s <m> <n> <k>\n", argv[0] );
		return 0;
	}
	else
	{
		m = atoi(argv[1]);
		n = atoi(argv[2]);
		k = atoi(argv[3]);
	}

	send_array[0] = m;
	send_array[1] = n;
	send_array[2] = k;
	send_array[3] = getpid();
	array = malloc(m*n*sizeof(int));
	sub_array = malloc(n*sizeof(int));
	sum_array = malloc(m*sizeof(int));

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

	/* master process code */

	// Create matrix as an array
	printf("Master (%d): m = %d n = %d k = %d and nr: ", getpid(), m, n, k);
	generate_array(array, m, n);
	print_array(array, m*n);

	// Spawn workers
	MPI_Comm_spawn( "distributed_sum_child", MPI_ARGV_NULL, m, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &workercomm, MPI_ERRCODES_IGNORE );

	// Send matrix dimensions to workers
	MPI_Bcast(send_array, 4, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm);

	// Send matrix line to worker
	MPI_Scatter(array, n, MPI_INT, sub_array, n, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm);

	// Receive partial sums from workers
	MPI_Gather(&sum, 1, MPI_INT, sum_array, 1, MPI_INT, myrank == 0 ? MPI_ROOT : MPI_PROC_NULL, workercomm);

	// Compute final sum
	sum = compute_sum(sum_array, m);
	printf("Master: Sum = %d\n", sum);

	/* end master process code */

	MPI_Comm_free( &workercomm );

	MPI_Finalize();
	
	return 0;
}
