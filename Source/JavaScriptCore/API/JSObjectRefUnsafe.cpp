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

#include "config.h"
#include "JSObjectRefUnsafe.h"
#include "JSObjectRefPrivate.h"

#include "APICast.h"
#include "ButterflyInlines.h"
#include "CodeBlock.h"
#include "CopiedSpaceInlines.h"
#include "DateConstructor.h"
#include "ErrorConstructor.h"
#include "FunctionConstructor.h"
#include "Identifier.h"
#include "InitializeThreading.h"
#include "JSAPIWrapperObject.h"
#include "JSArray.h"
#include "JSCallbackConstructor.h"
#include "JSCallbackFunction.h"
#include "JSCallbackObject.h"
#include "JSClassRef.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSRetainPtr.h"
#include "JSString.h"
#include "JSValueRefUnsafe.h"
#include "ObjectConstructor.h"
#include "ObjectPrototype.h"
#include "JSCInlines.h"
#include "PropertyNameArray.h"
#include "RegExpConstructor.h"

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectInspectorController.h"
#endif

using namespace JSC;

::JSType JSValueGetDataWithType(JSContextRef ctx, JSValueRef value, JSData* data)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);

	JSValue jsValue = toJS(exec, value);

	if (jsValue.isUndefined())
		return kJSTypeUndefined;
	if (jsValue.isNull())
		return kJSTypeNull;
	if (jsValue.isBoolean()) 
	{
		data->b = jsValue.toBoolean(exec);
		return kJSTypeBoolean;
	}
	else if (jsValue.isNumber())
	{
		if (jsValue.isInt32())
		{
			data->i = jsValue.toInt32(exec);
			return kJSTypeIntNumber;
		}
		data->d = jsValue.toNumber(exec);
		return kJSTypeNumber;
	}
	else if (jsValue.isString())
	{
		RefPtr<OpaqueJSString> stringRef(OpaqueJSString::create(jsValue.toString(exec)->value(exec)));
		data->str = stringRef.release().leakRef();
		return kJSTypeString;
	}
	else if (jsValue.isFunction())
		return kJSTypeFunction;
	else if (jsValue.inherits(exec->lexicalGlobalObject()->dateStructure()->classInfo()))
	{
		data->d = jsValue.toNumber(exec);
		return kJSTypeDate;
	}
	else if (jsValue.inherits(exec->lexicalGlobalObject()->arrayStructureForProfileDuringAllocation(static_cast<ArrayAllocationProfile*>(0))->classInfo()))
		return kJSTypeArray;
	if (jsValue.inherits(JSArrayBufferView::info()))
		return kJSTypeTypedArray;
	else if (jsValue.isObject())
	{
		auto jsObject = jsValue.toObject(exec);
		if (jsObject->isErrorInstance())
			return kJSError;
		if (jsObject->isProxy())
			return kJSTypeProxy;
		if (jsValue.inherits(exec->lexicalGlobalObject()->regExpStructure()->classInfo()))
			return kJSTypeRegExp;
		data->data = JSObjectGetPrivate(toRef(jsObject));
		return kJSTypeObject;
	}
	
	return kJSTypeObject;
}

::JSType JSValueGetDataWithTypeUnsafe(JSContextRef ctx, JSValueRef value, JSData* data)
{
	ExecState* exec = toJS(ctx);

	JSValue jsValue = toJS(exec, value);

	if (jsValue.isUndefined())
		return kJSTypeUndefined;
	if (jsValue.isNull())
		return kJSTypeNull;
	if (jsValue.isBoolean())
	{
		data->b = jsValue.toBoolean(exec);
		return kJSTypeBoolean;
	}
	else if (jsValue.isNumber())
	{
		if (jsValue.isInt32())
		{
			data->i = jsValue.toInt32(exec);
			return kJSTypeIntNumber;
		}
		data->d = jsValue.toNumber(exec);
		return kJSTypeNumber;
	}
	else if (jsValue.isString())
	{
		RefPtr<OpaqueJSString> stringRef(OpaqueJSString::create(jsValue.toString(exec)->value(exec)));
		data->str = stringRef.release().leakRef();
		return kJSTypeString;
	}
	else if (jsValue.isFunction())
		return kJSTypeFunction;
	else if (jsValue.inherits(exec->lexicalGlobalObject()->dateStructure()->classInfo()))
	{
		data->d = jsValue.toNumber(exec);
		return kJSTypeDate;
	}
	else if (jsValue.inherits(exec->lexicalGlobalObject()->arrayStructureForProfileDuringAllocation(static_cast<ArrayAllocationProfile*>(0))->classInfo()))
		return kJSTypeArray;
	if (jsValue.inherits(JSArrayBufferView::info()))
		return kJSTypeTypedArray;
	else if (jsValue.isObject())
	{
		auto jsObject = jsValue.toObject(exec);
		if (jsValue.toObject(exec)->isErrorInstance())
			return kJSError;
		if (jsValue.inherits(exec->lexicalGlobalObject()->regExpStructure()->classInfo()))
			return kJSTypeRegExp;
		data->data = JSObjectGetPrivate(toRef(jsObject));
		return kJSTypeObject;
	}

	return kJSTypeObject;
}

JSObjectRef JSObjectMakeUnsafe(JSContextRef ctx, JSClassRef jsClass, void* data)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	
	if (!jsClass)
		return toRef(constructEmptyObject(exec));

	JSCallbackObject<JSDestructibleObject>* object = JSCallbackObject<JSDestructibleObject>::create(exec, exec->lexicalGlobalObject(), exec->lexicalGlobalObject()->callbackObjectStructure(), jsClass, data);
	if (JSObject* prototype = jsClass->prototype(exec))
		object->setPrototype(exec->vm(), prototype);

	return toRef(object);
}

JSValueRef JSObjectGetPropertyDirect(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);

	JSObject* jsObject = toJS(object);

	JSValue jsValue = jsObject->getDirect(exec->vm(), propertyName->identifier(&exec->vm()));
	if (exec->hadException()) {
		JSValue exceptionValue = exec->exception();
		if (exception)
			*exception = toRef(exec, exceptionValue);
		exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
		exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
	}
	return toRef(exec, jsValue);
}

JSValueRef JSObjectGetPropertyUnsafe(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	//JSLockHolder locker(exec);

	JSObject* jsObject = toJS(object);

	JSValue jsValue = jsObject->get(exec, propertyName->identifier(&exec->vm()));
	if (exec->hadException()) {
		JSValue exceptionValue = exec->exception();
		if (exception)
			*exception = toRef(exec, exceptionValue);
		exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
		exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
	}
	return toRef(exec, jsValue);
}

struct OpaqueJSPropertyNameArray {
	WTF_MAKE_FAST_ALLOCATED;
public:
	OpaqueJSPropertyNameArray(VM* vm)
		: refCount(0)
		, vm(vm)
	{
	}

	unsigned refCount;
	VM* vm;
	Vector<JSRetainPtr<JSStringRef>> array;
};

JSPropertyNameArrayRef JSObjectCopyPropertyNamesUnsafe(JSContextRef ctx, JSObjectRef object)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	//JSLockHolder locker(exec);

	VM* vm = &exec->vm();

	JSObject* jsObject = toJS(object);
	JSPropertyNameArrayRef propertyNames = new OpaqueJSPropertyNameArray(vm);
	PropertyNameArray array(vm);
	jsObject->methodTable()->getPropertyNames(jsObject, exec, array, ExcludeDontEnumProperties);

	size_t size = array.size();
	propertyNames->array.reserveInitialCapacity(size);
	for (size_t i = 0; i < size; ++i)
		propertyNames->array.uncheckedAppend(JSRetainPtr<JSStringRef>(Adopt, OpaqueJSString::create(array[i].string()).leakRef()));

	return propertyNames;
}

bool JSObjectIsDate(JSContextRef ctx, JSObjectRef object)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);

	JSObject* jsObject = toJS(object);

	auto jsGlobal = exec->lexicalGlobalObject();
	return jsObject->inherits(jsGlobal->dateStructure()->classInfo());
}

bool JSObjectIsDateUnsafe(JSContextRef ctx, JSObjectRef object)
{
	ExecState* exec = toJS(ctx);
	JSObject* jsObject = toJS(object);

	auto jsGlobal = exec->lexicalGlobalObject();
	return jsObject->inherits(jsGlobal->dateStructure()->classInfo());
}

bool JSObjectIsFunctionUnsafe(JSContextRef ctx, JSObjectRef object)
{
	if (!object)
		return false;
	
	CallData callData;
	JSCell* cell = toJS(object);
	return cell->methodTable()->getCallData(cell, callData) != CallTypeNone;
}