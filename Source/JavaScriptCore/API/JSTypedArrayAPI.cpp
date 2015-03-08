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

#include "config.h"

#include "JSTypedArrayAPI.h"

#include "JSObjectRef.h"
#include "APICast.h"
#include "InitializeThreading.h"
#include "JSCallbackObject.h"
#include "JSClassRef.h"
#include "JSGlobalObject.h"

#include "JSArrayBuffer.h"
#include "JSFloat32Array.h"
#include "JSFloat64Array.h"
#include "JSInt8Array.h"
#include "JSInt16Array.h"
#include "JSInt32Array.h"
#include "JSUint8ClampedArray.h"
#include "JSUint8Array.h"
#include "JSUint16Array.h"
#include "JSUint32Array.h"

#include "SimpleTypedArrayController.h"

#include "Int32Array.h"

using namespace JSC;

template <typename ArrayType, typename JSArray, size_t byteLength>JSObject * CreateTypedArray(JSC::ExecState* exec, size_t length) {
	auto buffer = ArrayBuffer::create(length, byteLength*length);
	auto int32Array = ArrayType::create(buffer, 0, length);
   
    return JSArray::create(exec->vm(), exec->lexicalGlobalObject()->typedArrayStructure(int32Array->getType()), int32Array);
}

template <typename BufferType>JSObject * CreateArrayBuffer(JSC::ExecState* exec, size_t length) {
    RefPtr<BufferType> buffer = BufferType::create(length, 1);

    return JSArrayBuffer::create(exec->vm(), exec->lexicalGlobalObject()->arrayBufferStructure(), buffer);
}

typedef JSObject*(*CreateTypedArrayFuncPtr)(JSC::ExecState*, size_t);
const CreateTypedArrayFuncPtr CreateTypedArrayFunc[] = {
    NULL,
	CreateTypedArray<Int8Array, JSInt8Array, sizeof(int8_t)>,
	CreateTypedArray<Uint8Array, JSUint8Array, sizeof(uint8_t)>,
	CreateTypedArray<Uint8ClampedArray, JSUint8ClampedArray, sizeof(UInt8)>,
	CreateTypedArray<Int16Array, JSInt16Array, sizeof(int16_t)>,
	CreateTypedArray<Uint16Array, JSUint16Array, sizeof(uint16_t)>,
    CreateTypedArray<Int32Array, JSInt32Array, sizeof(int32_t)>,
	CreateTypedArray<Uint32Array, JSUint32Array, sizeof(uint32_t)>,
	CreateTypedArray<Float32Array, JSFloat32Array, sizeof(float)>,
	CreateTypedArray<Float64Array, JSFloat64Array, sizeof(double)>,
    CreateArrayBuffer<ArrayBuffer>
};

template <typename ArrayType, typename JSArray, size_t byteLength>JSObject * CreateTypedArrayFromSource(JSC::ExecState* exec, void* data, size_t length) {
	auto buffer = ArrayBuffer::create(data, length*byteLength);
	auto int32Array = ArrayType::create(buffer, 0, length);
   
    return JSArray::create(exec->vm(), exec->lexicalGlobalObject()->typedArrayStructure(int32Array->getType()), int32Array);
}

template <typename BufferType>JSObject * CreateArrayBufferFromSource(JSC::ExecState* exec, void* data, size_t length) {
    RefPtr<BufferType> buffer = BufferType::create(data, 1);

    return JSArrayBuffer::create(exec->vm(), exec->lexicalGlobalObject()->arrayBufferStructure(), buffer);
}

typedef JSObject*(*CreateTypedArrayFromSourceFuncPtr)(JSC::ExecState*, void*, size_t);
const CreateTypedArrayFromSourceFuncPtr CreateTypedArrayFromSourceFunc[] = {
    NULL,
	CreateTypedArrayFromSource<Int8Array, JSInt8Array, sizeof(int8_t)>,
	CreateTypedArrayFromSource<Uint8Array, JSUint8Array, sizeof(uint8_t)>,
	CreateTypedArrayFromSource<Uint8ClampedArray, JSUint8ClampedArray, sizeof(UInt8)>,
	CreateTypedArrayFromSource<Int16Array, JSInt16Array, sizeof(int16_t)>,
	CreateTypedArrayFromSource<Uint16Array, JSUint16Array, sizeof(uint16_t)>,
    CreateTypedArrayFromSource<Int32Array, JSInt32Array, sizeof(int32_t)>,
	CreateTypedArrayFromSource<Uint32Array, JSUint32Array, sizeof(uint32_t)>,
	CreateTypedArrayFromSource<Float32Array, JSFloat32Array, sizeof(float)>,
	CreateTypedArrayFromSource<Float64Array, JSFloat64Array, sizeof(double)>,
    CreateArrayBufferFromSource<ArrayBuffer>
};

JSTypedArrayType JSTypedArrayGetType(JSContextRef ctx, JSValueRef value) {
    ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
	JSTypedArrayType type = JSTypedArrayType::NotTypedArray;
	if( jsValue.inherits(JSArrayBufferView::info()) ) {
        JSObject* object = jsValue.getObject();
        type = (JSTypedArrayType) object->classInfo()->typedArrayStorageType;
    }
	else if( jsValue.inherits(JSArrayBuffer::info()) ) {
        type = JSTypedArrayType::TypeDataView;
    }
    return type;
}

JSObjectRef JSTypedArrayMake(JSContextRef ctx, JSTypedArrayType arrayType, size_t length) {
    ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);
    
	JSObject* result = NULL;
	if( arrayType > JSTypedArrayType::NotTypedArray && arrayType <= JSTypedArrayType::TypeDataView ) {
		result = CreateTypedArrayFunc[arrayType](exec, length);
    }

    return toRef(result);
}

JSObjectRef JSTypedArrayMakeFromSource(JSContextRef ctx, JSTypedArrayType arrayType, void* data, size_t length)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);
    
	JSObject* result = NULL;
	if( arrayType > JSTypedArrayType::NotTypedArray && arrayType <= JSTypedArrayType::TypeDataView ) {
		result = CreateTypedArrayFromSourceFunc[arrayType](exec, data, length);
    }

    return toRef(result);
}

inline ArrayBuffer* toArrayBufferView(JSValue value)
{
    JSArrayBufferView* wrapper = jsDynamicCast<JSArrayBufferView*>(value);
    if (!wrapper)
        return 0;
	return wrapper->buffer();
}

void * JSTypedArrayGetDataPtr(JSContextRef ctx, JSValueRef value, size_t * byteLength) {
    ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);
    
    JSValue jsValue = toJS(exec, value);
    if( ArrayBuffer * view = toArrayBufferView(jsValue) ) {
        if( byteLength ) {
            *byteLength = view->byteLength();
        }
		return view->data();
    }
    if( ArrayBuffer * buffer = toArrayBuffer(jsValue) ) {
        if( byteLength ) {
            *byteLength = buffer->byteLength();
        }
        return buffer->data();
    }
    
    if( byteLength ) {
        *byteLength = 0;
    }
    return NULL;
}