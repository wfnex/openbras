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

**********************************************************************/

#ifndef _HTTP_PARSE_H_
#define _HTTP_PARSE_H_
#include "aceinclude.h"
#include "BaseDefines.h"

typedef enum
{
   HTTP_PARSE_FIELD_URL     = 0,    /*获取客户端请求的资源*/
   HTTP_PARSE_FIELD_HOST    = 1,    /*获取http报文中host字段*/
   HTTP_PARSE_FIELD_REQTYPE = 2,    /*获取http报文中请求类GET POST*/ 
   HTTP_PARSE_FIELD_MAX     = 0xFF,   
}HTTP_FIELD_NAME;

typedef enum
{
   HTTP_PARSE_SUCCESS       = 0,  /*成功*/
   HTTP_PARSE_PARAM_ERROR   = 1,  /*参数错误*/
   HTTP_PARSE_NOT_FOUND     = 2,  /*未查找到查找的字段*/
}HTTP_PARSE_RET;

/*****************************************************************************
* 函 数 名  : http_get_field_value
* 功能描述  : 获取http报文中指定的参数
* 输入参数  :  
               pHttpPkt   : http 报文
               dwPktLen   : 报文的长度
               ucFieldFlag: 获取报文字段
* 输出参数  : 
               ppFieldValue:获取参数的起始地址
               pdwValueLen :参数的长度
* 返 回 值  : 
* 调用函数  : 
* 被调函数  : 
* 
* 修改历史  :
* 日    期  :
* 作    者  : linlizeng
* 修改内容  : 
*
*****************************************************************************/
HTTP_PARSE_RET  http_get_field_value(char *pHttpPkt,
                                          uint32_t          dwPktLen,
                                          HTTP_FIELD_NAME ucFieldFlag,
                                          char          **ppFieldValue,
                                          uint32_t         *pdwValueLen);
#endif

