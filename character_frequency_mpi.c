#include <stdio.h> 
#include <stdlib.h> 
#include "mpi.h"
#define N 128
#define base 0

int main (int argc, char *argv[]) {
	
	FILE *pFile;
	long file_size;
	char * buffer, *buffer_sub;
	char * filename;
	size_t result;
	int i, j, r, k, next, freq[N], *freq_final, chunk_size, c;
    int rank, size;

	// Initialize MPI
    MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    if (argc != 2) {
	    printf ("Usage : %s <file_name>\n", argv[0]);
		return 1;
    }

	filename = argv[1];
	pFile = fopen ( filename , "rb" );
	if (pFile==NULL) {printf ("File error\n"); return 2;}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	file_size = ftell (pFile);
	rewind (pFile);

	// Create the final frequency array only for root
	if (rank == 0) freq_final = ( int * ) calloc ( N, sizeof ( int ) );

    // Divide the array in equal-sized chunks
    chunk_size = file_size/size;
    buffer_sub = ( char * ) calloc (chunk_size, sizeof(char));

    // Copy a part of the file into the buffer_sub array of every process using modulo operation to avoid conflicts
	// e.x. for the text "a b c d e f g h" -> rank(0)'s chunk = a,e -> rank(1)'s chunk = b,f -> ...
	k = 0;
	next = 0;
    while ((c = fgetc(pFile)) != EOF)
    {
        if (k%size == rank){
        	buffer_sub[next++] = (char) c;
			k++;
		}
		else k++;
    }
    if (k != file_size) {printf ("Reading error, read %d characters\n",k); return 4;} 

	//if (rank == 0) printf("rank(0) will count : %s\n",buffer_sub);
	//if (rank == 1) printf("rank(1) will count : %s\n",buffer_sub);
	//if (rank == 2) printf("rank(2) will count : %s\n",buffer_sub);
	//if (rank == 3) printf("rank(3) will count : %s\n",buffer_sub);

	// Initialize freq with zeros
	for (j=0; j<N; j++){
		freq[j]=0;
	}
	
	// Every process counts the frequency of the characters inside its chunk
	for (i=0; i<chunk_size; i++){
		freq[buffer_sub[i] - base]++;
	}	

	// Reduce the freq subarrays into the final freq array
    MPI_Reduce(freq, freq_final, N, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);	

	// Print the final frequencies
	if (rank == 0){
		for (j=0; j<N; j++){
			printf("%d = %d\n", j+base, freq_final[j]);
		}
	}

	fclose (pFile);
    MPI_Finalize();
	return 0;
}
