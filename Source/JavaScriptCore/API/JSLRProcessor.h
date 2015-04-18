/*
* Copyright (C) 2015 Swise Corporation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1.  Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
* 2.  Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
* 3.  Neither the name of Swise Corporation nor the names of
*     its contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY SWISE CORPORATION AND ITS CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Swise Corporation OR ITS CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JSLRProcessor_h
#define JSLRProcessor_h

#include <JavaScript.h>
#include "JSBase.h"
#include "JSObjectRef.h"
#include "JSValueRefUnsafe.h"

#ifndef JSLR_API
#define	JSLR_API __declspec ( dllexport )
#endif // !JSLR_API

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JSLR_API bool _cdecl JSTryConvertToNumber(JSContextRef ctx, JSValueRef value, double* result);

JSLR_API bool _cdecl JSTryConvertToNumbers(JSContextRef ctx, JSValueRef** value, size_t length, int** indices, double** result);

JSLR_API bool _cdecl JSTryConvertToBool(JSContextRef ctx, JSValueRef value, bool* result);

JSLR_API bool _cdecl JSTryConvertToBooleans(JSContextRef ctx, JSValueRef** value, size_t length, int** indices, bool** result);

JSLR_API bool _cdecl JSTryConvertToString(JSContextRef ctx, JSValueRef value, const char** ptr, size_t* length, JSStringRef* str);

JSLR_API void _cdecl JSGetEmptyDefinition(JSClassDefinition** result);

#ifdef __cplusplus
}
#endif

#endif /* JSLRProcessor_h */