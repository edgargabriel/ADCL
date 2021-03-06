/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */


#include "ADCL_internal.h"

static int pown( int fanout, int num )
{
    int j, p = 1;
    if( num < 0 ) return 0;
    if (1==num) return fanout;
    if (2==fanout) {
        return p<<num;
    }
    else {
        for( j = 0; j < num; j++ ) { p*= fanout; }
    }
    return p;
}

static int calculate_level( int fanout, int rank )
{
    int level, num;
    if( rank < 0 ) return -1;
    for( level = 0, num = 0; num <= rank; level++ ) {
        num += pown(fanout, level);
    }
    return level-1;
}

static int calculate_num_nodes_up_to_level( int fanout, int level )
{
    /* just use geometric progression formula for sum:
       a^0+a^1+...a^(n-1) = (a^n-1)/(a-1) */
    return ((pown(fanout,level) - 1)/(fanout - 1));
}

ADCL_tree_t* ADCL_build_tree(int fanout,ADCL_request_t *req, int root)
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;

    int rank, size;
    int schild, sparent;
    int level; /* location of my rank in the tree structure of size */
    int delta; /* number of nodes on my level */
    int slimit; /* total number of nodes on levels above me */
    int shiftedrank;
    int i;
    ADCL_tree_t* tree;


    if (fanout<1) {
        return NULL;
    }
    if (fanout>MAXTREEFANOUT) {
        return NULL;
    }

    /*
     * Get size and rank of the process in this communicator
     */
    size = topo->t_size;
    rank = topo->t_rank;
tree = (ADCL_tree_t*)malloc(sizeof(ADCL_tree_t));
    if (!tree) {
        return NULL;
    }

    tree->tree_root     = MPI_UNDEFINED;
    tree->tree_nextsize = MPI_UNDEFINED;

    /*
     * Set root
     */
    tree->tree_root = root;

    /*
     * Initialize tree
     */
    tree->tree_fanout   = fanout;
    tree->tree_bmtree   = 0;
    tree->tree_root     = root;
    tree->tree_prev     = -1;
    tree->tree_nextsize = 0;
    for( i = 0; i < fanout; i++ ) {
        tree->tree_next[i] = -1;
    }

    /* return if we have less than 2 processes */
    if( size < 2 ) {
        return tree;
    }

    /*
     * Shift all ranks by root, so that the algorithm can be
     * designed as if root would be always 0
     * shiftedrank should be used in calculating distances
     * and position in tree
     */
    shiftedrank = rank - root;
    if( shiftedrank < 0 ) {
        shiftedrank += size;
    }

    /* calculate my level */
    level = calculate_level( fanout, shiftedrank );
    delta = pown( fanout, level );

    /* find my children */
    for( i = 0; i < fanout; i++ ) {
        schild = shiftedrank + delta * (i+1);
        if( schild < size ) {
            tree->tree_next[i] = (schild+root)%size;
            tree->tree_nextsize = tree->tree_nextsize + 1;
        } else {
            break;
        }
    }

    /* find my parent */
    slimit = calculate_num_nodes_up_to_level( fanout, level );
    sparent = shiftedrank;
if( sparent < fanout ) {
        sparent = 0;
    } else {
        while( sparent >= slimit ) {
            sparent -= delta/fanout;
        }
    }
    tree->tree_prev = (sparent+root)%size;

    return tree;
}

ADCL_tree_t *ADCL_build_in_order_bmtree(ADCL_request_t *req, int root)
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    int childs = 0;
    int rank, vrank;
    int size;
    int mask = 1;
    int remote;
    ADCL_tree_t *bmtree;
    int i;

    /*
     * Get size and rank of the process in this communicator
     */
    size = topo->t_size;
    rank = topo->t_rank;

    vrank = (rank - root + size) % size;

    bmtree = (ADCL_tree_t*)malloc(sizeof(ADCL_tree_t));
    if (!bmtree) {
        return NULL;
    }

    bmtree->tree_bmtree   = 1;
    bmtree->tree_root     = MPI_UNDEFINED;
    bmtree->tree_nextsize = MPI_UNDEFINED;
    for(i=0;i<MAXTREEFANOUT;i++) {
        bmtree->tree_next[i] = -1;
    }

    if (root == rank) {
        bmtree->tree_prev = root;
    }

    while (mask < size) {
        remote = vrank ^ mask;
        if (remote < vrank) {
            bmtree->tree_prev = (remote + root) % size;
            break;
        } else if (remote < size) {
            bmtree->tree_next[childs] = (remote + root) % size;
            childs++;
            if (childs==MAXTREEFANOUT) {
                return NULL;
            }
        }
        mask <<= 1;
    }
    bmtree->tree_nextsize = childs;
    bmtree->tree_root     = root;

    return bmtree;

}

ADCL_tree_t* ADCL_build_in_order_bintree(ADCL_request_t* req)
{
    ADCL_topology_t *topo = req->r_emethod->em_topo;
    int rank, size;
    int myrank, rightsize, delta;
    int parent, lchild, rchild;
    ADCL_tree_t* tree;

    /*
     * Get size and rank of the process in this communicator
     */
    size = topo->t_size;
    rank = topo->t_rank;

    tree = (ADCL_tree_t*)malloc(sizeof(ADCL_tree_t));
    if (!tree) {
        return NULL;
    }

    tree->tree_root     = MPI_UNDEFINED;
    tree->tree_nextsize = MPI_UNDEFINED;

    /*
     * Initialize tree
     */
    tree->tree_fanout   = 2;
    tree->tree_bmtree   = 0;
    tree->tree_root     = size - 1;
    tree->tree_prev     = -1;
    tree->tree_nextsize = 0;
    tree->tree_next[0]  = -1;
    tree->tree_next[1]  = -1;

    /*
     * Build the tree
     */
    myrank = rank;
    parent = size - 1;
    delta = 0;

    while ( 1 ) {
        /* Compute the size of the right subtree */
        rightsize = size >> 1;

        /* Determine the left and right child of this parent  */
        lchild = -1;
        rchild = -1;
        if (size - 1 > 0) {
            lchild = parent - 1;
            if (lchild > 0) {
                rchild = rightsize - 1;
            }
        }

        /* The following cases are possible: myrank can be
           - a parent,
           - belong to the left subtree, or
   - belong to the right subtee
           Each of the cases need to be handled differently.
 */

        if (myrank == parent) {
            /* I am the parent:
               - compute real ranks of my children, and exit the loop. */
            if (lchild >= 0) tree->tree_next[0] = lchild + delta;
            if (rchild >= 0) tree->tree_next[1] = rchild + delta;
            break;
        }
        if (myrank > rchild) {
            /* I belong to the left subtree:
               - If I am the left child, compute real rank of my parent
               - Iterate down through tree:
               compute new size, shift ranks down, and update delta.
            */
            if (myrank == lchild) {
                tree->tree_prev = parent + delta;
            }
            size = size - rightsize - 1;
            delta = delta + rightsize;
            myrank = myrank - rightsize;
            parent = size - 1;

        } else {
            /* I belong to the right subtree:
               - If I am the right child, compute real rank of my parent
               - Iterate down through tree:
               compute new size and parent,
               but the delta and rank do not need to change.
            */
            if (myrank == rchild) {
                tree->tree_prev = parent + delta;
            }
            size = rightsize;
            parent = rchild;
        }
    }

    if (tree->tree_next[0] >= 0) { tree->tree_nextsize = 1; }
    if (tree->tree_next[1] >= 0) { tree->tree_nextsize += 1; }

    return tree;

}
 
ADCL_tree_t* ADCL_build_chain(ADCL_request_t *req,int root, int fanout)
{
    int rank, size;
    int srank; /* shifted rank */
    int i,maxchainlen;
    int mark,head,len;
    ADCL_tree_t *chain;
    ADCL_topology_t *topo = req->r_emethod->em_topo;

    /*
     * Get size and rank of the process in this communicator
     */
    size = topo->t_size;
    rank = topo->t_rank;

    if( fanout < 1 ) {
        fanout = 1;
    }
    if (fanout>MAXTREEFANOUT) {
        fanout = MAXTREEFANOUT;
    }

    /*
     * Allocate space for topology arrays if needed
     */
    chain = (ADCL_tree_t*)malloc( sizeof(ADCL_tree_t) );
    if (!chain) {
        fflush(stdout);
        return NULL;
    }
    chain->tree_root     = MPI_UNDEFINED;
    chain->tree_nextsize = -1;
    for(i=0;i<fanout;i++) chain->tree_next[i] = -1;

    /*
     * Set root & numchain
     */
    chain->tree_root = root;
    if( (size - 1) < fanout ) {
        chain->tree_nextsize = size-1;
        fanout = size-1;
    } else {
        chain->tree_nextsize = fanout;
    }

    /*
     * Shift ranks
     */
    srank = rank - root;
    if (srank < 0) srank += size;

    /*
     * Special case - fanout == 1
     */
if( fanout == 1 ) {
        if( srank == 0 ) chain->tree_prev = -1;
        else chain->tree_prev = (srank-1+root)%size;

        if( (srank + 1) >= size) {
chain->tree_next[0] = -1;
            chain->tree_nextsize = 0;
        } else {
            chain->tree_next[0] = (srank+1+root)%size;
            chain->tree_nextsize = 1;
        }
        return chain;
    }

    /* Let's handle the case where there is just one node in the communicator */
    if( size == 1 ) {
        chain->tree_next[0] = -1;
        chain->tree_nextsize = 0;
        chain->tree_prev = -1;
        return chain;
    }
    /*
     * Calculate maximum chain length
     */
    maxchainlen = (size-1) / fanout;
    if( (size-1) % fanout != 0 ) {
        maxchainlen++;
        mark = (size-1)%fanout;
    } else {
        mark = fanout+1;
    }

    /*
     * Find your own place in the list of shifted ranks
     */
    if( srank != 0 ) {
        int column;
        if( srank-1 < (mark * maxchainlen) ) {
            column = (srank-1)/maxchainlen;
            head = 1+column*maxchainlen;
            len = maxchainlen;
        } else {
            column = mark + (srank-1-mark*maxchainlen)/(maxchainlen-1);
            head = mark*maxchainlen+1+(column-mark)*(maxchainlen-1);
            len = maxchainlen-1;
        }

        if( srank == head ) {
            chain->tree_prev = 0; /*root*/
        } else {
            chain->tree_prev = srank-1; /* rank -1 */
        }
        if( srank == (head + len - 1) ) {
            chain->tree_next[0] = -1;
            chain->tree_nextsize = 0;
        } else {
            if( (srank + 1) < size ) {
                chain->tree_next[0] = srank+1;
                chain->tree_nextsize = 1;
} else {
                chain->tree_next[0] = -1;
                chain->tree_nextsize = 0;
            }
        }
}

    /*
     * Unshift values
     */
    if( rank == root ) {
        chain->tree_prev = -1;
        chain->tree_next[0] = (root+1)%size;
        for( i = 1; i < fanout; i++ ) {
            chain->tree_next[i] = chain->tree_next[i-1] + maxchainlen;
            if( i > mark ) {
                chain->tree_next[i]--;
            }
            chain->tree_next[i] %= size;
        }
        chain->tree_nextsize = fanout;
    } else {
        chain->tree_prev = (chain->tree_prev+root)%size;
        if( chain->tree_next[0] != -1 ) {
            chain->tree_next[0] = (chain->tree_next[0]+root)%size;
        }
    }

    return chain;
}

