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

#include "JSLRProcessor.h"

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

using namespace JSC;

JSLR_API bool _cdecl JSTryConvertToNumber(JSContextRef ctx, JSValueRef value, double* result)
{
	ExecState* exec = toJS(ctx);
	JSValue jsValue = toJS(exec, value);
	if (jsValue.isNumber())
	{
		*result = jsValue.toNumber(exec);
		return true;
	}
	return false;
}

JSLR_API bool _cdecl JSTryConvertToNumbers(JSContextRef ctx, JSValueRef** value, size_t length, int** indices, double** result)
{
	auto values = *value;
	auto res = *result;
	auto ind = *indices;
	ExecState* exec = toJS(ctx);
	for (unsigned int i = 0; i < length; i++)
	{
		auto ptr = values[ind[i]];
		JSValueRef js_value = *&ptr;
		JSValue jsValue = toJS(exec, js_value);
		if (!jsValue.isNumber())
		{
			return false;
		}
		*&res[i] = jsValue.toNumber(exec);
	}
	return true;
}

JSLR_API bool _cdecl JSTryConvertToBool(JSContextRef ctx, JSValueRef value, bool* result)
{
	ExecState* exec = toJS(ctx);
	JSValue jsValue = toJS(exec, value);
	if (jsValue.isBoolean())
	{
		*result = jsValue.toBoolean(exec);
		return true;
	}
	return false;
}

JSLR_API bool _cdecl JSTryConvertToBooleans(JSContextRef ctx, JSValueRef** value, size_t length, int** indices, bool** result)
{
	auto values = *value;
	auto res = *result;
	auto ind = *indices;
	ExecState* exec = toJS(ctx);
	for (unsigned int i = 0; i < length; i++)
	{
		auto ptr = values[ind[i]];
		JSValueRef js_value = *&ptr;
		JSValue jsValue = toJS(exec, js_value);
		if (!jsValue.isBoolean())
		{
			return false;
		}
		*&res[i] = jsValue.toBoolean(exec);
	}
	return true;
}

JSLR_API bool _cdecl JSTryConvertToString(JSContextRef ctx, JSValueRef value, const char** ptr, size_t* length, JSStringRef* str)
{
	ExecState* exec = toJS(ctx);
	JSValue jsValue = toJS(exec, value);
	if (jsValue.isString())
	{
		RefPtr<OpaqueJSString> stringRef(OpaqueJSString::create(jsValue.toString(exec)->value(exec)));
		
		*ptr = (const char*)stringRef->characters();
		*length = stringRef->length();

		auto js_str = JSStringRetain(stringRef.release().leakRef());
		*str = js_str;

		return true;
	}
	return false;
}

JSLR_API void JSGetEmptyDefinition(JSClassDefinition** result)
{
	**result = kJSClassDefinitionEmpty;
}