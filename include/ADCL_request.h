/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * Copyright (c) 2007           Cisco, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef __ADCL_REQUEST_H__
#define __ADCL_REQUEST_H__

#ifdef ADCL_DUMMY_MPI
#include "ADCL_dummy_mpi.h"
#else
#include "mpi.h"
#endif

#ifdef ADCL_LIBNBC
#include "nbc.h"
#endif

struct ADCL_vector_t;
struct ADCL_function_t;
struct ADCL_emethod_t;
struct ADCL_topology_t;

#ifdef ADCL_LIBNBC
typedef struct {
  int alg;
  int segsize;
  int fanout;
  NBC_Schedule *schedule;
} ADCL_Ibcast_args;
#endif

#define ADCL_COMM_ACTIVE 1024
#define ADCL_COMM_AVAIL  1025

/* The most general ADCL object used to start and identify a communication 'entity' */
struct ADCL_request_s{
    int                        r_id; /* unique identifier */
    int                    r_findex; /* index for the fortran interface */
    int                r_comm_state; /* communication state of the object  */

    ADCL_vector_t         **r_svecs; /* ptr to the vectors describing send data items */
    ADCL_vector_t         **r_rvecs; /* ptr to the vectors describing recv data items */
    ADCL_emethod_t       *r_emethod; /* ptr to the emethod describing everything */
    ADCL_function_t     *r_function; /* ADCL function currently being used. This pointer
                                        will also contain the 'winner' function once
                                        the evaluation part is over. */

    /* Elements used for the communication */
    int                   *r_spsize; /* size of each individual temporary sbuf used
                                        for pack/unpack */
    int                   *r_rpsize; /* size of each individual temporary rbuf used
                                        for pack/unpack */
    MPI_Win                   r_win; /* window used for one-sided operations */
    MPI_Group               r_group; /* Group used for some window operations */
    int                     r_ndats; /* size of arrays r_sdats and r_rdats */
    MPI_Datatype           *r_sdats; /* array of MPI datatypes used for sending */
    MPI_Datatype           *r_rdats; /* array of MPI datatypes used for receiving */
    int                    *r_scnts;  /* array of number of MPI datatypes to be send */ 
    int                    *r_rcnts;  /* array of number of MPI datatypes to be received */ 
    MPI_Request            *r_sreqs; /* array of send requests used for nb ops */
    MPI_Request            *r_rreqs; /* array of recv requests used for nb ops */
    char                   **r_rbuf; /* temp recv buffer used for pack/unpack */
    char                   **r_sbuf; /* temp send buffer used for pack/unpack */

    /* Elements required for the selection logic */
    int                     r_erlast; /* last method used */
    int                     r_erflag; /* flag to be passed to the state machine */
    TIME_TYPE                 r_time; /* temporary buffer to store the exeuction
				         time for dual-block operations */
 
#ifdef ADCL_LIBNBC
   NBC_Handle	            r_handle;
   int                      r_progress;
   ADCL_Ibcast_args         r_Ibcast_args;
#endif
};

extern ADCL_array_t * ADCL_request_farray;

int ADCL_request_create_generic_rooted ( ADCL_vector_t **svecs,
                                  ADCL_vector_t **rvecs,
                                  ADCL_topology_t *topo,
                                  ADCL_fnctset_t *fnctset,
                                  ADCL_request_t **req, int root, int order );

int ADCL_request_free ( ADCL_request_t **req );
int ADCL_request_init ( ADCL_request_t *req, int *db );
int ADCL_request_wait ( ADCL_request_t *req );

int ADCL_request_update ( ADCL_request_t *req,
                          TIME_TYPE t1, TIME_TYPE t2 );

int ADCL_request_reg_hist_criteria ( ADCL_request_t *req, ADCL_hist_criteria_t *hist_criteria );

int ADCL_request_get_comm ( ADCL_request_t *req, MPI_Comm *comm, int *rank, int *size);

int ADCL_request_get_curr_function ( ADCL_request_t *req, char **function_name,
                                     char ***attrs_names, int *attrs_num,
                                     char ***attrs_values_names, int **attrs_values_num );
int ADCL_request_get_winner_stat ( ADCL_request_t *req, double *filtered_avg,
                                   double *unfiltered_avg, double *outliers_num );
int ADCL_request_get_functions_with_average ( ADCL_request_t *req, 
                                              double filtered_avg, 
                                              int *num_functions,
                                              char ***function_name, 
                                              char ***attrs_names, 
                                              int *attrs_num, 
                                              char ****attrs_values_names, 
                                              int ***attrs_values_num );

int ADCL_request_save_status ( ADCL_request_t *req, int *tested_num,
                               double **unfiltered_avg,
                               double **filtered_avg,
                               double **outliers, int *winner_so_far );

int ADCL_request_restore_status ( ADCL_request_t *req, int tested_num,
                                  double *unfiltered_avg,
                                  double *filtered_avg,
                                  double *outliers );

int ADCL_request_get_fsname ( ADCL_request_t *req, char **fsname );

int ADCL_request_get_tndims ( ADCL_request_t *req, int *tndims );

int ADCL_request_get_state ( ADCL_request_t *req, int *state );

#endif /* __ADCL_REQUEST_H__ */

