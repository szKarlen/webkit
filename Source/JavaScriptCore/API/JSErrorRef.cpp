/*
 * Copyright (C) 2015 Swise Corporation All rights reserved.
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
#include "JSErrorRef.h"

#include "APICast.h"
#include "OpaqueJSString.h"

#include "Error.h"
#include "JSGlobalObject.h"
#include "VM.h"
#include "JSObject.h"
#include "ObjectConstructor.h"
#include "NativeErrorConstructor.h"

using namespace JSC;

JSObjectRef JSEvalErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());
	
	return toRef(createEvalError(exec, message->string()));
}

JSObjectRef JSRangeErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(createRangeError(exec, message->string()));
}

JSObjectRef JSReferenceErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(createReferenceError(exec, message->string()));
}

JSObjectRef JSSyntaxErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(createSyntaxError(exec, message->string()));
}

JSObjectRef JSTypeErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(createTypeError(exec, message->string()));
}

JSObjectRef JSURIErrorMake(JSContextRef ctx, JSStringRef message)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(createURIError(exec, message->string()));
}

JSObjectRef JSThrowError(JSContextRef ctx, JSObjectRef error)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder lock(exec->vm());

	return toRef(exec->vm().throwException(exec, toJS(error)));
}

::JSErrorType JSErrorGetType(JSContextRef ctx, JSObjectRef error)
{
	ExecState* exec = toJS(ctx);
	JSLockHolder locker(exec);

	auto jsValue = toJS(exec, error);
	if (!jsValue.isObject())
		return kJSValueError;
	auto jsError = toJS(error);
	if (jsError->isErrorInstance())
	{
		auto jsGlobal = exec->lexicalGlobalObject();
		auto jsStructure = jsError->structure();
		if (jsGlobal->evalErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSEvalError;
		}
		else if (jsGlobal->rangeErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSRangeError;
		}
		else if (jsGlobal->referenceErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSReferenceError;
		}
		else if (jsGlobal->syntaxErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSSyntaxError;
		}
		else if (jsGlobal->typeErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSTypeError;
		}
		else if (jsGlobal->URIErrorConstructor()->hasInstance(exec, jsValue))
		{
			return kJSURIError;
		}
		else
		{
			return kJSDefaultError;
		}
	}
	return kJSObjectError;
}