#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void sort(int *, int *, int, int);

int main(int argc, char *argv[])
{
    int *x, *y, *y_final, *x_sub;
    int i, j, my_num, my_place;
    int rank, size, chunk_size;

	// Initialize MPI
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	if (argc != 2) {
      printf ("Usage : %s <array_size>\n", argv[0]);
      return 1;
    }

	// Allocate memory for the arrays
	int n = strtol(argv[1], NULL, 10);
    x = ( int * ) calloc ( n, sizeof ( int ) );
    y = ( int * ) calloc ( n, sizeof ( int ) );
    if (rank == 0) y_final = ( int * ) calloc ( n, sizeof ( int ) );

    // Send the array x to every process by broadcast
    if (rank == 0){
      for (i=0; i<n; i++)
		    x[i] = n - i;
    }
    MPI_Bcast(x, n, MPI_INT, 0, MPI_COMM_WORLD);

	// Count-sort for each process in its own y array
    for (j=0; j<n; j++) {
        if (j%size != rank) continue; //check ==
        my_num = x[j];
        my_place = 0;
        for (i=0; i<n; i++)
            if ( my_num > x[i] ) 
                my_place++;
        y[my_place] = my_num;
    }

    // Reduce the sorted sub-arrays into one
    MPI_Reduce(y, y_final, n, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if (rank == 0){
		for (i=0; i<n; i++) 
			printf("%d\n", y_final[i]);
    }

	MPI_Finalize();
	return 0;

}