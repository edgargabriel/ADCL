/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include <stdio.h>
#include <string.h>

#include "ADCL.h"
#include "ADCL_internal.h"
#include "mpi.h"

static void alltoallv_test(int cnt, int dims, int nc, int rank, int size, ADCL_Topology topo); 

extern void dump_vector_1D ( double *data, int rank, int dim);
extern void set_data_1D ( double *data, int rank, int dim); 
extern int check_data_1D ( double *data, int* rcounts, int *rdispl, int rank, int size);
//static void set_data_2D ( double* data[][], int rank, int dims[2]);

int main ( int argc, char ** argv ) 
{
    int cnt, dims, err; 
    int rank, size;
    int cdims=0;
    int periods=0;
    int nc; 

    ADCL_Topology topo;
    MPI_Comm cart_comm;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    ADCL_Init ();

    MPI_Dims_create ( size, 1, &cdims );
    MPI_Cart_create ( MPI_COMM_WORLD, 1, &cdims, &periods, 0, &cart_comm);

    err = ADCL_Topology_create ( cart_comm, &topo );
    if ( ADCL_SUCCESS != err) goto exit;   

    cnt = 30;
    dims = 13;

    dims=4; nc=2;
    /* AllGather with Vector_allocate */ 
    alltoallv_test(cnt, dims, nc, rank, size, topo);

exit:
    if ( ADCL_TOPOLOGY_NULL != topo)   ADCL_Topology_free ( &topo );
    MPI_Comm_free ( &cart_comm );
    ADCL_Finalize ();
    MPI_Finalize ();
    return 0;
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
void alltoallv_test(int cnt, int dims, int nc, int rank, int size, 
		     ADCL_Topology topo)
{
    double *sdata, *rdata;
    int dim, i; 
    int* cnts, *displ;
    int err, errc; 
    ADCL_Request request;
    ADCL_Timer timer;
 
    cnts = (int*) calloc ( size, sizeof(int) ); 
    displ = (int*) calloc ( size, sizeof(int) );
    for ( i=0;i<size;i++){
       cnts[i] = dims*nc;
       displ[i] = dims*nc * i; 
    }


    dim = dims*size*nc;

    sdata = (double *) calloc(dim, sizeof(double));
    rdata = (double *) calloc(dim, sizeof(double));

    err = ADCL_Alltoallv_init ( sdata, cnts, displ, MPI_DOUBLE, rdata, cnts, displ, MPI_DOUBLE, MPI_COMM_WORLD, &request);
    if ( ADCL_SUCCESS != err) goto exit;
    err = ADCL_Timer_create (1, &request, &timer);
    if ( ADCL_SUCCESS != err) goto exit;

    for (i=0; i<cnt; i++){

        ADCL_Timer_start(timer);

	set_data_1D ( sdata, rank, dim );
	set_data_1D ( rdata, -1,   dim);

#ifdef VERBOSE
       dump_vector_1D ( sdata, rank, dim);
       dump_vector_1D ( rdata, rank, dim);
#endif

       err = ADCL_Request_start( request );
       if ( ADCL_SUCCESS != err) goto exit;   

       errc = check_data_1D ( rdata, cnts, displ, rank, size);
//       if (errc) goto exit;   

       ADCL_Timer_stop(timer);
	     
    }

    MPI_Barrier ( MPI_COMM_WORLD);

exit:
    if ( ADCL_SUCCESS != err) { printf("ADCL error nr. %d\n", err); } 

    if ( NULL != cnts) free(cnts);
    if ( NULL != displ) free(displ);
    if ( ADCL_TIMER_NULL != request) ADCL_Timer_free ( &timer );
    if ( ADCL_REQUEST_NULL != request) ADCL_Request_free ( &request );

    return;
}
