/***********************************************************************
 * Copyright (C) 2013, Nanjing WFN Technology Co., Ltd 
 * Description: 
 * Others:
 * Version:          V1.0
 * Author:           Yi Jian
 * Date:             2013-09-12
 *
 * History 1:
 *     Date:          
 *     Version:       
 *     Author:       
 *     Modification: 
**********************************************************************/

#ifndef BNGPERROR_H
#define BNGPERROR_H
#include "BaseDefines.h"

#define BNGPLAT_ERROR_MODULE_BASE      10000
#define BNGPLAT_ERROR_MODULE_NETWORK   20000
#define BNGPLAT_ERROR_MODULE_DB        30000
#define BNGPLAT_ERROR_MODULE_OPENFLOW_ENGINE 40000
#define BNGPLAT_FAILED(rv) (rv != 0)
#define BNGPLAT_SUCCEEDED(rv) (rv == 0)

/*###################################################################################*/
#define BNGPLAT_OK                              0
#define BNGPLAT_ERROR_FAILURE                   (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 1)
#define BNGPLAT_ERROR_NOT_INITIALIZED           (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 2)
#define BNGPLAT_ERROR_ALREADY_INITIALIZED       (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 3)
#define BNGPLAT_ERROR_NOT_IMPLEMENTED           (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 4)
#define BNGPLAT_ERROR_NULL_POINTER              (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 5)
#define BNGPLAT_ERROR_UNEXPECTED                (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 6)
#define BNGPLAT_ERROR_OUT_OF_MEMORY             (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 7)
#define BNGPLAT_ERROR_INVALID_ARG               (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 8)
#define BNGPLAT_ERROR_NOT_AVAILABLE             (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 9)
#define BNGPLAT_ERROR_WOULD_BLOCK               (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 10)
#define BNGPLAT_ERROR_NOT_FOUND                 (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 11)
#define BNGPLAT_ERROR_FOUND                     (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 12)
#define BNGPLAT_ERROR_PARTIAL_DATA              (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 13)
#define BNGPLAT_ERROR_TIMEOUT                   (BNGPResult)(BNGPLAT_ERROR_MODULE_BASE + 14)
#endif // BNGPERROR_H
