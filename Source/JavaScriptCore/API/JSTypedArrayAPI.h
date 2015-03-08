/*
 * Copyright (C) 2015 Swise Corporation All rights reserved.
 * Copyright (C) 2014 Dominic Szablewski <http://www.phoboslab.org>
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

#ifndef JSTypedArray_h
#define JSTypedArray_h

#include <JavaScriptCore/JSValueRef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
@enum JSType
@abstract     A constant identifying the Typed Array type of a JSValue.
@constant     kJSTypedArrayTypeNone                 Not a Typed Array.
@constant     kJSTypedArrayTypeInt8Array            Int8Array
@constant     kJSTypedArrayTypeInt16Array           Int16Array
@constant     kJSTypedArrayTypeInt32Array           Int32Array
@constant     kJSTypedArrayTypeUint8Array           Int8Array
@constant     kJSTypedArrayTypeUint8ClampedArray    Int8ClampedArray
@constant     kJSTypedArrayTypeUint16Array          Uint16Array
@constant     kJSTypedArrayTypeUint32Array          Uint32Array
@constant     kJSTypedArrayTypeFloat32Array         Float32Array
@constant     kJSTypedArrayTypeFloat64Array         Float64Array
@constant     kJSTypedArrayTypeArrayBuffer          ArrayBuffer
*/
typedef enum {
    NotTypedArray,
    TypeInt8,
    TypeUint8,
    TypeUint8Clamped,
    TypeInt16,
    TypeUint16,
    TypeInt32,
    TypeUint32,
    TypeFloat32,
    TypeFloat64,
    TypeDataView
} JSTypedArrayType;

/*!
@function
@abstract           Returns a JavaScript value's Typed Array type
@param ctx          The execution context to use.
@param value        The JSValue whose Typed Array type you want to obtain.
@result             A value of type JSTypedArrayType that identifies value's Typed Array type.
*/
JS_EXPORT JSTypedArrayType JSTypedArrayGetType(JSContextRef ctx, JSValueRef value);

/*!
@function
@abstract           Creates a JavaScript Typed Array with the given number of elements
@param ctx          The execution context to use.
@param arrayType    A value of type JSTypedArrayType identifying the type of array you want to create
@param numElements  The number of elements for the array.
@result             A JSObjectRef that is a Typed Array or NULL if there was an error
*/
JS_EXPORT JSObjectRef JSTypedArrayMake(JSContextRef ctx, JSTypedArrayType arrayType, size_t length);

/*!
@function
@abstract           Creates a JavaScript Typed Array with the given number of elements
@param ctx          The execution context to use.
@param arrayType    A value of type JSTypedArrayType identifying the type of array you want to create
@param numElements  The number of elements for the array.
@result             A JSObjectRef that is a Typed Array or NULL if there was an error
*/
JS_EXPORT JSObjectRef JSTypedArrayMakeFromSource(JSContextRef ctx, JSTypedArrayType arrayType, void* data, size_t length);

/*!
@function
@abstract           Returns a pointer to a Typed Array's data in memory
@param ctx          The execution context to use.
@param value        The JSValue whose Typed Array type data pointer you want to obtain.
@param byteLength   A pointer to a size_t in which to store the byte length of the Typed Array
@result             A pointer to the Typed Array's data or NULL if the JSValue is not a Typed Array
*/
JS_EXPORT void * JSTypedArrayGetDataPtr(JSContextRef ctx, JSValueRef value, size_t * byteLength);

#ifdef __cplusplus
}
#endif

#endif /* JSTypedArray_h */