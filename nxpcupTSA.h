#include "freemaster.h"
#include "freemaster_tsa.h"

/* Define an empty FreeMASTER TSA table */
FMSTR_TSA_TABLE_BEGIN(tsa_table)
FMSTR_TSA_TABLE_END()

FMSTR_TSA_TABLE_LIST_BEGIN()
    FMSTR_TSA_TABLE(tsa_table)
FMSTR_TSA_TABLE_LIST_END()