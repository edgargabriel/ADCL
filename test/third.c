#include <stdio.h>

#include "ADCL.h"
#include "mpi.h"

#define NIT 100

int main ( int argc, char ** argv ) 
{
    int i, rank, size;
    int dims[3]={66,34,34};
    int cdims[]={0,0,0};
    int periods[]={0,0,0};
    int ***data1, ***data2, ***data3;
    ADCL_Topology topo;
    ADCL_Vector vec1, vec2, vec3;    
    ADCL_Request request1, request2, request3;
    
    MPI_Comm cart_comm;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();
    ADCL_Vector_allocate ( 3,  dims, 0, 1, MPI_DOUBLE, &data1, &vec1 );
    ADCL_Vector_allocate ( 3,  dims, 0, 1, MPI_DOUBLE, &data2, &vec2 );
    ADCL_Vector_allocate ( 3,  dims, 0, 1, MPI_DOUBLE, &data3, &vec3 );

    MPI_Dims_create ( size, 3, cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 3, cdims, periods, 0, &cart_comm);
    ADCL_Topology_create_bycomm ( cart_comm, &topo );

    ADCL_Request_create ( vec1, topo, &request1 );
    ADCL_Request_create ( vec2, topo, &request2 );
    ADCL_Request_create ( vec3, topo, &request3 );

    for ( i=0; i<NIT; i++ ) {
	ADCL_Request_start( request1 );
	ADCL_Request_start( request2 );
	ADCL_Request_start( request3 );
    }

    ADCL_Request_free ( &request1 );
    ADCL_Request_free ( &request2 );
    ADCL_Request_free ( &request3 );
    ADCL_Vector_free ( &vec1 );
    ADCL_Vector_free ( &vec2 );
    ADCL_Vector_free ( &vec3 );
    ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );
    
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}
