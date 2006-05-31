#ifndef __ADCL_CHANGE_H__
#define __ADCL_CHANGE_H__

#include "mpi.h"

#if COMMMODE == 1
  #define COMMTEXT  IsendIrecvDerDatatype

  #define ADCL_CHANGE_SB_AAO  ADCL_change_isendirecv_sb_aao
  #define ADCL_CHANGE_SB_PAIR ADCL_change_isendirecv_sb_pair
  #define SEND_START(req,i,tag) MPI_Isend(req->r_vec->v_data, 1,\
         req->r_sdats[i], req->r_neighbors[i], tag, req->r_comm,\
         &req->r_sreqs[i])
  #define RECV_START(req,i,tag) MPI_Irecv(req->r_vec->v_data, 1,\
         req->r_rdats[i], req->r_neighbors[i], tag, req->r_comm,\
         &req->r_rreqs[i])
  #define SEND_WAITALL(req) MPI_Waitall(req->r_nneighbors, req->r_sreqs, \
         MPI_STATUSES_IGNORE )
  #define RECV_WAITALL(req) MPI_Waitall(req->r_nneighbors, req->r_rreqs, \
         MPI_STATUSES_IGNORE)
  #define SEND_WAIT(req,i) MPI_Wait(&req->r_sreqs[i], MPI_STATUS_IGNORE)
  #define RECV_WAIT(req,i) MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE)

#elif COMMMODE == 2
  #define COMMTEXT  SendRecvDerDatatype

  #define ADCL_CHANGE_SB_PAIR ADCL_change_sendrecv_sb_pair
  #define SEND_START(req,i,tag) MPI_Ssend(req->r_vec->v_data, 1, \
         req->r_sdats[i], req->r_neighbors[i], tag, req->r_comm )
  #define RECV_START(req,i,tag) MPI_Recv(req->r_vec->v_data, 1,  \
         req->r_rdats[i], req->r_neighbors[i], tag, req->r_comm, \
         MPI_STATUS_IGNORE )
  #define SEND_WAITALL(req) 
  #define RECV_WAITALL(req) 
  #define SEND_WAIT(req,i) 
  #define RECV_WAIT(req,i) 

#elif COMMMODE == 3
  #define COMMTEXT SendIrecvDerDatatype

  #define ADCL_CHANGE_SB_AAO  ADCL_change_sendirecv_sb_aao
  #define ADCL_CHANGE_SB_PAIR ADCL_change_sendirecv_sb_pair
  #define SEND_START(req,i,tag) MPI_Send(req->r_vec->v_data, 1,\
         req->r_sdats[i], req->r_neighbors[i], tag, req->r_comm)
  #define RECV_START(req,i,tag) MPI_Irecv(req->r_vec->v_data, 1,\
         req->r_rdats[i], req->r_neighbors[i], tag, req->r_comm,\
         &req->r_rreqs[i])
  #define SEND_WAITALL(req) 
  #define RECV_WAITALL(req) MPI_Waitall(req->r_nneighbors, req->r_rreqs, \
         MPI_STATUSES_IGNORE)
  #define SEND_WAIT(req,i) 
  #define RECV_WAIT(req,i) MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE)

#elif COMMMODE == 4
  #define COMMTEXT  IsendIrecvPack

  #define ADCL_CHANGE_SB_AAO  ADCL_change_isendirecv_pack_sb_aao
  #define ADCL_CHANGE_SB_PAIR ADCL_change_isendirecv_pack_sb_pair
  #define SEND_START(req,i,tag) { int _pos=0;                          \
     MPI_Pack ( req->r_vec->v_data, 1, req->r_sdats[i],                \
                req->r_sbuf[i], req->r_spsize[i], &_pos, req->r_comm);\
     MPI_Isend(req->r_sbuf[i], _pos, MPI_PACKED, req->r_neighbors[i],  \
	       tag, req->r_comm, &req->r_sreqs[i]);}

  #define RECV_START(req,i,tag) MPI_Irecv(req->r_rbuf[i], req->r_rpsize[i],\
         MPI_PACKED, req->r_neighbors[i], tag, req->r_comm,&req->r_rreqs[i])
  #define SEND_WAITALL(req) MPI_Waitall(req->r_nneighbors, req->r_sreqs,    \
         MPI_STATUSES_IGNORE )
  #define RECV_WAITALL(req) { int _i, _pos=0;                            \
     MPI_Waitall(req->r_nneighbors, req->r_rreqs, MPI_STATUSES_IGNORE);  \
     for (_i=0; _i< req->r_nneighbors; i++, _pos=0 ) {                   \
         if ( req->r_neighbors[i] == MPI_PROC_NULL ) continue;           \
         MPI_Unpack(req->r_rbuf[i], req->r_rpsize[i], &_pos,             \
              req->r_vec->v_data, 1, req->r_rdats[i], req->r_comm ); } }

  #define SEND_WAIT(req,i) MPI_Wait(&req->r_sreqs[i], MPI_STATUS_IGNORE)
  #define RECV_WAIT(req,i) { int _pos=0;                         \
     MPI_Wait(&req->r_rreqs[i], MPI_STATUS_IGNORE);              \
     MPI_Unpack(req->r_rbuf[i], req->r_rpsize[i], &_pos,         \
         req->r_vec->v_data, 1, req->r_rdats[i], req->r_comm ); }
    
#endif



#endif /* __ADCL_CHANGE_H__ */

