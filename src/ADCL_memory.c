#include "ADCL_internal.h"


void* ADCL_allocate_matrix ( int ndims, int *dims, MPI_Datatype dat ) 
{

    if ( dat == MPI_DOUBLE ) {
	return ADCL_allocate_double_matrix ( ndims, dims );
    }
    else if ( dat == MPI_FLOAT ) {
	return ADCL_allocate_float_matrix ( ndims, dims );
    }
    else if ( dat == MPI_INT ) {
	return ADCL_allocate_int_matrix  (ndims, dims );
    }
    else {
	ADCL_printf("Datatype not supported by ADCL right now!\n");
    }

    return NULL;
}

void ADCL_free_matrix ( int ndims, MPI_Datatype dat, void *mat ) 
{

    if ( dat == MPI_DOUBLE ) {
	ADCL_free_double_matrix ( ndims, mat);
    }
    else if ( dat == MPI_FLOAT ) {
	ADCL_free_float_matrix ( ndims, mat );
    }
    else if ( dat == MPI_INT ) {
	ADCL_free_int_matrix  (ndims, mat );
    }
    else {
	ADCL_printf("Datatype not supported by ADCL right now!\n");
    }

    return;
}
