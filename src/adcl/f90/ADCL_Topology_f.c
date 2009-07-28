/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2009           HLRS. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"
#include "ADCL_fprototypes.h"

#ifndef _SX
#pragma weak adcl_topology_create_  = adcl_topology_create
#pragma weak adcl_topology_create__ = adcl_topology_create
#pragma weak ADCL_TOPOLOGY_CREATE   = adcl_topology_create

#pragma weak adcl_topology_create_extended_neighborhood_  = adcl_topology_create_extended_neighborhood
#pragma weak adcl_topology_create_extended_neighborhood__ = adcl_topology_create_extended_neighborhood
#pragma weak ADCL_TOPOLOGY_CREATE_EXTENDED_NEIGHBORHOOD   = adcl_topology_create_extended_neighborhood

#pragma weak adcl_topology_create_generic_   = adcl_topology_create_generic
#pragma weak adcl_topology_create_generic__  = adcl_topology_create_generic
#pragma weak ADCL_TOPOLOGY_CREATE_GENERIC    = adcl_topology_create_generic

#pragma weak adcl_topology_dump_  = adcl_topology_dump
#pragma weak adcl_topology_dump__ = adcl_topology_dump
#pragma weak ADCL_TOPOLOGY_DUMP   = adcl_topology_dump

#pragma weak adcl_topology_free_  = adcl_topology_free
#pragma weak adcl_topology_free__ = adcl_topology_free
#pragma weak ADCL_TOPOLOGY_FREE   = adcl_topology_free
#endif

#ifdef _SX
void adcl_topology_create_generic_ ( int *ndims, int *nneigh, int *lneighb, int *rneighb, int *flip, int *coords, 
				    int *direction, int *comm, int *topo, int *ierror )
#else
void adcl_topology_create_generic ( int *ndims, int *nneigh, int *lneighb, int *rneighb, int *flip, int *coords, 
				    int *direction, int *comm, int *topo, int *ierror )
#endif
{
    ADCL_topology_t *ctopo;
    MPI_Comm ccomm;
    
    if ( ( NULL == ndims )    || 
	 ( NULL == nneigh )   ||
	 ( NULL == lneighb )  ||
	 ( NULL == rneighb  ) ||
	 ( NULL == flip     ) ||
	 ( NULL == coords )   ||
	 ( NULL == comm )     ||
	 ( NULL == topo ) )   {
	*ierror = ADCL_INVALID_ARG;
	return;
    }

    ccomm = MPI_Comm_f2c (*comm);
    if ( ccomm == MPI_COMM_NULL ) {
	*ierror = ADCL_INVALID_COMM;
	return;
    }
    
    if ( 0 == *ndims ) {
	*ierror = ADCL_topology_create_generic ( *ndims, *nneigh, NULL, NULL, NULL, 
						 NULL, *direction, ccomm,
						 &ctopo );
    }
    else {
        *ierror = ADCL_topology_create_generic ( *ndims, *nneigh, lneighb, rneighb, flip, 
						 coords, *direction, ccomm,
						 &ctopo );
    }

    if ( *ierror == ADCL_SUCCESS ) {
	*topo = ctopo->t_findex;
    }

    return;
}

#ifdef _SX
void adcl_topology_create_  ( int* cart_comm, int *topo, int *ierror )
#else
void adcl_topology_create   ( int* cart_comm, int *topo, int *ierror )
#endif
{
    int topo_type;
    ADCL_topology_t *ctopo;
    MPI_Comm ccomm;

    if ( ( NULL == cart_comm ) ||
	 ( NULL == topo ) )    {
	*ierror = ADCL_INVALID_ARG;
	return;
    }

    ccomm = MPI_Comm_f2c (*cart_comm);
    if ( ccomm == MPI_COMM_NULL ) {
	*ierror = ADCL_INVALID_COMM;
	return;
    }
    MPI_Topo_test ( ccomm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
        *ierror = ADCL_INVALID_COMM;
        return;
    }

    *ierror = ADCL_topology_create ( ccomm, &ctopo );
    if ( *ierror == ADCL_SUCCESS ) {
	*topo = ctopo->t_findex;
    } 
    
    return;
}

#ifdef _SX
void adcl_topology_create_extended_neighborhood_  ( int* cart_comm, int *topo, int *ierror )
#else
void adcl_topology_create_extended_neighborhood   ( int* cart_comm, int *topo, int *ierror )
#endif
{
    int topo_type, ndims;
    ADCL_topology_t *ctopo;
    MPI_Comm ccomm;

    if ( ( NULL == cart_comm ) ||
	 ( NULL == topo ) )    {
	*ierror = ADCL_INVALID_ARG;
	return;
    }

    ccomm = MPI_Comm_f2c (*cart_comm);
    if ( ccomm == MPI_COMM_NULL ) {
	*ierror = ADCL_INVALID_COMM;
	return;
    }
    MPI_Topo_test ( ccomm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
        *ierror = ADCL_INVALID_COMM;
        return;
    }

    MPI_Cartdim_get ( ccomm, &ndims );
    if ( 1 > ndims || 3 < ndims ) {
        *ierror = ADCL_INVALID_TOPOLOGY;
        return;
    }
 
    *ierror = ADCL_topology_create_extended_neighborhood ( ccomm, &ctopo );
    if ( *ierror == ADCL_SUCCESS ) {
	*topo = ctopo->t_findex;
    } 
    
    return;
}

#ifdef _SX
void adcl_topology_dump_ ( int* topo, int *ierror)
#else
void adcl_topology_dump ( int* topo, int *ierror)
#endif
{
    ADCL_topology_t *ctopo;

    if ( 0 > topo ) {
        *ierror = ADCL_INVALID_TOPOLOGY;
        return;
    }

    ctopo = (ADCL_topology_t *) ADCL_array_get_ptr_by_pos (ADCL_topology_farray, *topo );

    *ierror = ADCL_topology_dump ( ctopo );

    return;
}



#ifdef _SX
void adcl_topology_free_ ( int *topo, int *ierror )
#else
void adcl_topology_free ( int *topo, int *ierror )
#endif
{
    ADCL_topology_t *ctopo;

    ctopo = (ADCL_topology_t *) ADCL_array_get_ptr_by_pos (ADCL_topology_farray, 
							   *topo );
    *ierror = ADCL_topology_free ( &ctopo );

    *topo = ADCL_FTOPOLOGY_NULL;

    return;
}

