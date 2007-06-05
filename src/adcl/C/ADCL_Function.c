/*
 * Copyright (c) 2006-2007      University of Houston. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ADCL.h"
#include "ADCL_internal.h"

int ADCL_Function_create ( ADCL_work_fnct_ptr *iptr, ADCL_Attrset attrset,
                           int *array_of_values, char *name, ADCL_Function *fnct )
{
    int i, k ,found = 0;
    if ( NULL == iptr ) {
        return ADCL_INVALID_WORK_FUNCTION_PTR;
    }

    if ( attrset != ADCL_ATTRSET_NULL ) {
        if ( attrset->as_id < 0 ) {
            return ADCL_INVALID_ATTRSET;
        }
        if ( NULL == array_of_values ) {
            return ADCL_INVALID_ARG;
        }
    }
    /*
    ** We check here whether each of the values is
    ** within the registerd range of the according attributes
    */
    if ( ADCL_ATTRSET_NULL != attrset ) {
        for (i=0; i<attrset->as_maxnum; i++) {
            for (k=0; k<attrset->as_attrs_numval[i] ; k++) {
                if ( array_of_values[i] == attrset->as_attrs[i]->a_values[k] ) {
                    found = 1;
                }
            }
            if (found == 0) {
                return ADCL_INVALID_ARG;
            }
            else {
                found = 0;
            }
        }
     }
    /* Note: name can be NULL, thus not checking that */
    return ADCL_function_create_async ( iptr, NULL, attrset, array_of_values,
                                        name, fnct);
}

int ADCL_Function_create_async ( ADCL_work_fnct_ptr *iptr, ADCL_work_fnct_ptr *wptr,
                 ADCL_Attrset attrset, int *array_of_values,
                 char *name, ADCL_Function *fnct )
{
    if ( NULL == iptr || NULL == wptr ) {
        return ADCL_INVALID_WORK_FUNCTION_PTR;
    }

    if ( attrset != ADCL_ATTRSET_NULL ) {
        if ( attrset->as_id < 0 ) {
            return ADCL_INVALID_ATTRSET;
        }

        if ( NULL == array_of_values ) {
            return ADCL_INVALID_ARG;
        }
    }
    /*
    ** Theoretically, we should check here whether each of the values is
    ** within the registerd range of the according attributes
    */

    /* Note: name can be NULL, thus not checking that */

    return ADCL_function_create_async ( iptr, wptr, attrset, array_of_values,
                    name, fnct );
}

int ADCL_Function_free ( ADCL_Function *fnct )
{
    if ( NULL == fnct ) {
        return ADCL_INVALID_ARG;
    }
    if ( (*fnct)->f_id < 0 ) {
        return ADCL_INVALID_FUNCTION;
    }

    return ADCL_function_free (fnct);
}

int ADCL_Fnctset_create ( int maxnum, ADCL_Function *fncts,
                          char *name, ADCL_Fnctset *fctset )
{
    int i;

    if ( 0 >= maxnum ) {
        return ADCL_INVALID_ARG;
    }
    if ( NULL == fctset ) {
        return ADCL_INVALID_ARG;
    }
    for ( i=0; i< maxnum; i++ ) {
        if ( fncts[i]->f_id < 0 ) {
            return ADCL_INVALID_FUNCTION;
        }
    }

    return ADCL_fnctset_create ( maxnum, fncts, name, fctset );
}

int ADCL_Fnctset_free ( ADCL_Fnctset *fctset )
{
    if ( NULL == fctset ) {
        return ADCL_INVALID_ARG;
    }

    if ( (*fctset)->fs_id < 0 ) {
        return ADCL_INVALID_FNCTSET;
    }

    return ADCL_fnctset_free ( fctset );
}



