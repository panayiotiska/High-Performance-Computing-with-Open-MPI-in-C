#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void sort(int *, int *, int, int);

int main(int argc, char *argv[])
{
    int *x, *y_final, *y_locations, *y_sub;
    int i, j, my_num, my_place;
    int rank, size, chunk_size, next;
    int START, STOP, EXTRA;

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
    if (rank == 0) {
        y_final = ( int * ) calloc ( n, sizeof ( int ) );
        y_locations = ( int * ) calloc ( n, sizeof ( int ) );
    }

    // Send the array x to every process by broadcast
    if (rank == 0){
      for (i=0; i<n; i++)
		    x[i] = n - i; //fill the array with integers from n to zero (sorted backwards)
    }
    MPI_Bcast(x, n, MPI_INT, 0, MPI_COMM_WORLD);

    // Divide the array in equal-sized chunks
    chunk_size = n/size;
    y_sub = ( int * ) calloc (chunk_size, sizeof(int));
    
	// Initialize SPMD variables
    START = chunk_size * rank;
    STOP = START + chunk_size;
    EXTRA = n % size;
	
	// Every process inserts the locations of its chunk to the y_sub array
	next = 0;
    for (j=START; j<STOP; j++) {
        my_num = x[j];
        my_place = 0;
        for (i=0; i<n; i++)
            if ( my_num > x[i] ) 
                my_place++;
        y_sub[next] = my_place;
        next ++;
        if (next > chunk_size) break;
    }
	
    // Gather the sorted sub-arrays into one
    MPI_Gather(y_sub, chunk_size, MPI_INT, y_locations, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	// Fill the final array using the locations gathered
    if (rank == 0){
        for (i=0; i<n; i++){
            y_final[y_locations[i]] = x[i];
        }
		
		// Print the final sorted array
        for (i=0; i<n; i++){
            printf("%d\n", y_final[i]);
        }
    }

	MPI_Finalize();
	return 0;

}