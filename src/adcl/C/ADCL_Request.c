#include "ADCL.h"
#include "ADCL_internal.h"

int ADCL_Request_create ( ADCL_Vector vec, MPI_Comm cart_comm, 
			  ADCL_Request *req )
{
    int topo_type;
    
    /* Right now we can only handle cartesian topologies! */
    MPI_Topo_test ( cart_comm, &topo_type );
    if ( MPI_UNDEFINED==topo_type || MPI_GRAPH==topo_type ) {
	return ADCL_INVALID_COMM;
    }
    
    return ADCL_request_create (vec, cart_comm, req, MPI_ORDER_C );
}

int ADCL_Request_free ( ADCL_Request *req )
{
    ADCL_request_t *preq = *req;

    if ( NULL == req ) {
	return ADCL_INVALID_REQUEST;
    }
    if ( preq->r_id < 0 ) {
	return ADCL_INVALID_REQUEST;
    }
    if ( preq->r_comm_state != ADCL_COMM_AVAIL ) {
	return ADCL_INVALID_REQUEST;
    }

    return ADCL_request_free ( req );
}

int ADCL_Request_start ( ADCL_Request req ) 
{
    int ret=ADCL_SUCCESS;
    int db;
    TIME_TYPE t1, t2;

    /* Check validity of the request  */

    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    if ( ADCL_SUCCESS != ret ) {
	return ret;
    }
    
    if ( db ) {
	ret = ADCL_request_wait ( req );
    }
    t2 = TIME;
    ADCL_request_update ( req, t1, t2 );

    return ret;
}

int ADCL_Request_init ( ADCL_Request req ) 
{
    TIME_TYPE t1, t2;
    int ret=ADCL_SUCCESS;
    int db;

    /* Check validity of the request  */
    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    t2 = TIME;
    req->r_time = t2-t1;

    return ret;
}


int ADCL_Request_wait ( ADCL_Request req ) 
{
    int ret=ADCL_SUCCESS;
    TIME_TYPE t1, t2;

    /* Check validity of the request  */
    t1 = TIME;
    ret = ADCL_request_wait ( req );
    t2 = TIME;
    ADCL_request_update ( req, t1, (t2+req->r_time));

    return ret;
}

int ADCL_Request_start_overlap ( ADCL_Request req, ADCL_work_fctn_ptr* midfctn,
				 ADCL_work_fctn_ptr *endfctn, 
				 ADCL_work_fctn_ptr *totalfctn,
				 void *arg1, void* arg2, void *arg3 )

{
    TIME_TYPE t1, t2;
    int ret=ADCL_SUCCESS;
    int db;

    /* Check validity of the request  */

    t1 = TIME;
    ret = ADCL_request_init ( req, &db );
    if ( ADCL_SUCCESS != ret ) {
	return ret;
    }
    
    if ( db ) {
	if ( ADCL_NULL_FCTN_PTR != midfctn ) {
	    midfctn ( arg1, arg2, arg3 );
	}
	ret = ADCL_request_wait ( req );
	if ( ADCL_NULL_FCTN_PTR != endfctn ) {
	    endfctn ( arg1, arg2, arg3 );
	}
    }
    else {
	if ( ADCL_NULL_FCTN_PTR != endfctn ) {
	    totalfctn(arg1, arg2, arg3 );
	}
    }
    t2 = TIME;
    ADCL_request_update ( req, t1, t2 );
    
    return ret;
}