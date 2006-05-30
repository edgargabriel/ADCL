#include <stdio.h>

#include "ADCL.h"
#include "mpi.h"


/* Dimensions of the data matrix per process */
#define DIM0  2
#define DIM1  4

/* how many halo-cells for each neighbor ? */
#define HWIDTH 1


int main ( int argc, char ** argv ) 
{
    /* General variables */
    int i, j, k;
    int rank, size;
    
    /* Definition of the 2-D vector */
    int dims[2]={DIM0+2*HWIDTH, DIM1+2*HWIDTH};
    double matrix[DIM0+2*HWIDTH][DIM1+2*HWIDTH];
    double offset1, offset0;
    ADCL_vector vec;

    /* Variables for the process topology information */
    int cdims[]={0,0};
    int coord[2];
    int periods[]={0,0};
    MPI_Comm cart_comm;
    ADCL_request request;


    /* Initiate the MPI environment */
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    /* Initiate the ADCL library */
    ADCL_Init ();
    
    /* Register a two dimensional vector with ADCL */
    ADCL_vector_register ( 2,  dims, 1, 1, MPI_DOUBLE, matrix, &vec );

    /* Describe the neighborhood relations */
    MPI_Dims_create ( size, 2, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 2, cdims, periods, 0, &cart_comm);
    MPI_Cart_coords ( cart_comm, rank, 2, coord );

    /* Match the data type description and the process topology 
       to each other */
    ADCL_request_create ( vec, cart_comm, &request );


    /* Initiate matrix to zero including halo-cells */
    for (i=0; i<DIM0+2; i++ ) {
        for ( j=0; j<DIM1+2; j++ ) {
            matrix[i][j] = 0.0;
        }
    }

    /* Set now the values */
    offset1 = coord[1] * DIM1;
    offset0 = coord[0] * DIM0 * (DIM1 * cdims[1]);
    for (i=1; i<DIM0+1; i++ ) {
        for ( j=1; j<DIM1+1; j++ ){
            matrix[i][j] = (i-1)*DIM1*cdims[1]+offset0+offset1+(j-1);
        }
    }

#if VERBOSE
    if ( rank == 0 ) {
        printf("The original input matrix is :\n");
    }
    for ( k=0; k< size; k++ ){
        if ( rank == k ) {
            for ( i=0; i<DIM0+2; i++ ) {
                printf("%d: ", rank );
                for ( j=0; j<DIM1+2; j++ ) {
                    printf("%lf ", matrix[i][j]);
                }
                printf("\n");
            }
            printf("\n");
        }
        MPI_Barrier ( cart_comm );
    }
#endif

    /* Now this is the real communication */
    ADCL_change_sb_aao ( request );

   /* Dump the resulting matrix */
    if (rank == 0 ) {
        printf("After the communication step\n");
    }
    for ( k=0 ; k < size; k++ ) {
        if (rank == k ) {
            for ( i=0; i<DIM0+2; i++ ) {
                printf("%d: ", rank );
                for ( j=0; j<DIM1+2; j++ ) {
                    printf("%lf ", matrix[i][j]);
                }
                printf("\n");
            }
                printf("\n");
        }
        MPI_Barrier ( cart_comm );
    }


    ADCL_request_free ( &request );
    ADCL_vector_deregister ( &vec );
    MPI_Comm_free ( &cart_comm );
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}
