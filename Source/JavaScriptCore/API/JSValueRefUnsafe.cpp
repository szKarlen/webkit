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
#include "JSValueRef.h"
#include "JSValueRefUnsafe.h"

#include "APICast.h"
#include "JSAPIWrapperObject.h"
#include "JSCJSValue.h"
#include "JSCallbackObject.h"
#include "JSGlobalObject.h"
#include "JSONObject.h"
#include "JSString.h"
#include "LiteralParser.h"
#include "JSCInlines.h"
#include "Protect.h"

#include <wtf/Assertions.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

#include "Strong.h"
#include "StrongInlines.h"

#include "Handle.h"
#include "HandleSet.h"

#include <algorithm> // for std::min

#if PLATFORM(MAC)
#include <mach-o/dyld.h>
#endif

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectInspectorController.h"
#endif

using namespace JSC;

#if PLATFORM(MAC)
static bool evernoteHackNeeded()
{
    static const int32_t webkitLastVersionWithEvernoteHack = 35133959;
    static bool hackNeeded = CFEqual(CFBundleGetIdentifier(CFBundleGetMainBundle()), CFSTR("com.evernote.Evernote"))
        && NSVersionOfLinkTimeLibrary("JavaScriptCore") <= webkitLastVersionWithEvernoteHack;

    return hackNeeded;
}
#endif

int32_t JSValueToIntNumber(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return PNaN;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    int32_t number = jsValue.toInt32(exec);
    if (exec->hadException()) {
        JSValue exceptionValue = exec->exception();
        if (exception)
            *exception = toRef(exec, exceptionValue);
        exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
        exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
        number = PNaN;
    }
    return number;
}

/* JSLR additions */

::JSType JSParameterGetType(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return kJSTypeUndefined;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);

    if (jsValue.isUndefined())
        return kJSTypeUndefined;
    if (jsValue.isNull())
        return kJSTypeNull;
    if (jsValue.isBoolean())
        return kJSTypeBoolean;
    if (jsValue.isNumber())
    {
        if (jsValue.isInt32())
            return kJSTypeIntNumber;
        return kJSTypeNumber;
    }
    if (jsValue.isString())
        return kJSTypeString;
	if (jsValue.isFunction())
		return kJSTypeFunction;
	if (jsValue.inherits(exec->lexicalGlobalObject()->dateStructure()->classInfo()))
		return kJSTypeDate;
	if (jsValue.inherits(exec->lexicalGlobalObject()->arrayStructureForProfileDuringAllocation(static_cast<ArrayAllocationProfile*>(0))->classInfo()))
		return kJSTypeArray;
	if (jsValue.inherits(JSArrayBufferView::info()))
		return kJSTypeTypedArray;
	ASSERT(jsValue.isObject());
	auto jsObject = jsValue.toObject(exec);
	if (jsObject->isErrorInstance())
		return kJSError;
	if (jsValue.inherits(exec->lexicalGlobalObject()->regExpStructure()->classInfo()))
		return kJSTypeRegExp;
    return kJSTypeObject;
}

bool JSParameterIsUndefined(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isUndefined();
}

bool JSParameterIsNull(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isNull();
}

bool JSParameterIsBoolean(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isBoolean();
}

bool JSTryGetParameterAsBoolean(JSContextRef ctx, JSValueRef value, bool* result)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);

    if (jsValue.isBoolean())
    {
        bool state = jsValue.toBoolean(exec);

        *result = state;

        return true;
    }
    return false;
}

bool JSParameterIsNumber(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isNumber();
}

bool JSTryGetParameterAsNumber(JSContextRef ctx, JSValueRef value, double* result)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);

    double number = jsValue.toNumber(exec);

    if (exec->hadException()) {
        exec->clearException();
        number = PNaN;
        return false;
    }
    *result = number;
    return true;
}

bool JSParameterIsString(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isString();
}

bool JSParameterIsObject(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);
    return jsValue.isObject();
}

bool JSParameterIsObjectOfClass(JSContextRef ctx, JSValueRef value, JSClassRef jsClass)
{
    if (!ctx || !jsClass) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);

    if (JSObject* o = jsValue.getObject()) {
        if (o->inherits(JSCallbackObject<JSGlobalObject>::info()))
            return jsCast<JSCallbackObject<JSGlobalObject>*>(o)->inherits(jsClass);
        if (o->inherits(JSCallbackObject<JSDestructibleObject>::info()))
            return jsCast<JSCallbackObject<JSDestructibleObject>*>(o)->inherits(jsClass);
#if JSC_OBJC_API_ENABLED
        if (o->inherits(JSCallbackObject<JSAPIWrapperObject>::info()))
            return jsCast<JSCallbackObject<JSAPIWrapperObject>*>(o)->inherits(jsClass);
#endif
    }
    return false;
}

bool JSParameterIsEqual(JSContextRef ctx, JSValueRef a, JSValueRef b, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsA = toJS(exec, a);
    JSValue jsB = toJS(exec, b);

    bool result = JSValue::equal(exec, jsA, jsB); // false if an exception is thrown
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return result;
}

bool JSParameterIsStrictEqual(JSContextRef ctx, JSValueRef a, JSValueRef b)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsA = toJS(exec, a);
    JSValue jsB = toJS(exec, b);

    return JSValue::strictEqual(exec, jsA, jsB);
}

bool JSParameterIsInstanceOfConstructor(JSContextRef ctx, JSValueRef value, JSObjectRef constructor, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);

    JSValue jsValue = toJS(exec, value);

    JSObject* jsConstructor = toJS(constructor);
    if (!jsConstructor->structure()->typeInfo().implementsHasInstance())
        return false;
    bool result = jsConstructor->hasInstance(exec, jsValue); // false if an exception is thrown
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return result;
}

bool JSValueToBooleanUnsafe(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    //JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
    return jsValue.toBoolean(exec);
}

double JSValueToNumberUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return PNaN;
    }
    ExecState* exec = toJS(ctx);
    //JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    double number = jsValue.toNumber(exec);
    if (exec->hadException()) {
        JSValue exceptionValue = exec->exception();
        if (exception)
            *exception = toRef(exec, exceptionValue);
        exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
        exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
        number = PNaN;
    }
    return number;
}

int32_t JSValueToIntNumberUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return PNaN;
    }
    ExecState* exec = toJS(ctx);
    //JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    int32_t number = jsValue.toInt32(exec);
    if (exec->hadException()) {
        JSValue exceptionValue = exec->exception();
        if (exception)
            *exception = toRef(exec, exceptionValue);
        exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
        exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
        number = PNaN;
    }
    return number;
}

JSStringRef JSValueToStringCopyUnsafe(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    //JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    RefPtr<OpaqueJSString> stringRef(OpaqueJSString::create(jsValue.toString(exec)->value(exec)));
    if (exec->hadException()) {
        JSValue exceptionValue = exec->exception();
        if (exception)
            *exception = toRef(exec, exceptionValue);
        exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
        exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
        stringRef.clear();
    }
    return stringRef.release().leakRef();
}

JSValueRef JSValueMakeUndefinedUnsafe(JSContextRef ctx)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	return toRef(exec, jsUndefined());
}

JSValueRef JSValueMakeNullUnsafe(JSContextRef ctx)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	return toRef(exec, jsNull());
}

JSValueRef JSValueMakeBooleanUnsafe(JSContextRef ctx, bool value)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	return toRef(exec, jsBoolean(value));
}

JSValueRef JSValueMakeNumberUnsafe(JSContextRef ctx, double value)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	return toRef(exec, jsNumber(purifyNaN(value)));
}

JSValueRef JSValueMakeStringUnsafe(JSContextRef ctx, JSStringRef string)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	return toRef(exec, jsString(exec, string ? string->string() : String()));
}

JSValueRef JSValueMakeFromJSONStringUnsafe(JSContextRef ctx, JSStringRef string)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);

	String str = string->string();
	unsigned length = str.length();
	if (!length || str.is8Bit()) {
		LiteralParser<LChar> parser(exec, str.characters8(), length, StrictJSON);
		return toRef(exec, parser.tryLiteralParse());
	}
	LiteralParser<UChar> parser(exec, str.characters16(), length, StrictJSON);
	return toRef(exec, parser.tryLiteralParse());
}

JSStringRef JSValueCreateJSONStringUnsafe(JSContextRef ctx, JSValueRef apiValue, unsigned indent, JSValueRef* exception)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	
	JSValue value = toJS(exec, apiValue);
	String result = JSONStringify(exec, value, indent);
	if (exception)
		*exception = 0;
	if (exec->hadException()) {
		JSValue exceptionValue = exec->exception();
		if (exception)
			*exception = toRef(exec, exceptionValue);
		exec->clearException();
#if ENABLE(REMOTE_INSPECTOR)
		exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exceptionValue);
#endif
		return 0;
	}
	return OpaqueJSString::create(result).leakRef();
}

JSHandleRef JSFunctionProtect(JSContextRef ctx, JSValueRef value)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return 0;
	}
	ExecState* exec = toJS(ctx);
	JSValue jsFunction = toJS(exec, value);

	Strong<JSC::Unknown>* handle = new Strong<JSC::Unknown>(exec->vm(), jsFunction);

	return reinterpret_cast<JSHandleRef>(handle);
}

void JSFunctionUnprotect(JSContextRef ctx, JSHandleRef handle)
{
	if (!ctx) {
		ASSERT_NOT_REACHED();
		return;
	}
	ExecState* exec = toJS(ctx);
	Strong<JSC::Unknown>* jsHandle = reinterpret_cast<Strong<JSC::Unknown>*>(const_cast<OpaqueJSHandle*>(handle));

	jsHandle->clear();

	delete jsHandle;
}