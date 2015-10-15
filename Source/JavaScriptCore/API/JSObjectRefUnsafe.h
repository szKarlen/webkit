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

#ifndef JSObjectRefUnsafe_h
#define JSObjectRefUnsafe_h

#include <JavaScriptCore/JSObjectRef.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stddef.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

union JSData
{
	bool b;
	int i;
	double d;
	JSStringRef str;
	void* data;
};

JS_EXPORT ::JSType JSValueGetDataWithType(JSContextRef ctx, JSValueRef value, JSData* data);

JS_EXPORT ::JSType JSValueGetDataWithTypeUnsafe(JSContextRef ctx, JSValueRef value, JSData* data);

JS_EXPORT JSObjectRef JSObjectMakeUnsafe(JSContextRef ctx, JSClassRef jsClass, void* data);

JS_EXPORT JSValueRef JSObjectGetPropertyDirect(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

JS_EXPORT JSValueRef JSObjectGetPropertyUnsafe(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);

JS_EXPORT bool JSObjectIsDate(JSContextRef ctx, JSObjectRef object);

JS_EXPORT bool JSObjectIsDateUnsafe(JSContextRef ctx, JSObjectRef object);

JS_EXPORT bool JSObjectIsFunctionUnsafe(JSContextRef ctx, JSObjectRef object);

JS_EXPORT JSValueRef JSObjectCallAsFunctionUnsafe(JSContextRef ctx, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);


/*!
@function
@abstract Gets the names of an object's enumerable properties.
@param ctx The execution context to use.
@param object The object whose property names you want to get.
@result A JSPropertyNameArray containing the names object's enumerable properties. Ownership follows the Create Rule.
*/
JS_EXPORT JSPropertyNameArrayRef JSObjectCopyPropertyNamesUnsafe(JSContextRef ctx, JSObjectRef object);

#ifdef __cplusplus
}
#endif

#endif /* JSObjectRef_h */
