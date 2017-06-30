/***********************************************************************
	Copyright (c) 2017, The OpenBRAS project authors. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	  * Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

	  * Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in
		the documentation and/or other materials provided with the
		distribution.

	  * Neither the name of OpenBRAS nor the names of its contributors may
		be used to endorse or promote products derived from this software
		without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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
