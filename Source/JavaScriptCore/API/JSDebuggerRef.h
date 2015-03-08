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

#ifndef JSDebuggerRef_h
#define JSDebuggerRef_h

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSValueRef.h>

#include "APICast.h"

#include "Debugger.h"

#include "JSDebuggerBase.h"

#include "InitializeThreading.h"

#include "DebuggerCallFrame.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct JSStackFrameDesc
	{
	public:
		JSStringRef functionName;
		JSValueRef thisObject;
		JSC::DebuggerCallFrame::Type type;
		JSValueRef global;
		JSValueRef scope;
	};

	enum CallFrameFunctionType
	{
		ProgramType,
		FunctionType
	};

JS_EXPORT JSDebuggerRef JSDebuggerCreate(const JSDebuggerDefinition* definition);

JS_EXPORT JSDebuggerRef JSDebuggerRetain(JSDebuggerRef debugger);

JS_EXPORT void JSDebuggerRelease(JSDebuggerRef debugger);

JS_EXPORT void JSContextAttachDebugger(JSContextRef ctx, JSDebuggerRef debugger);

JS_EXPORT void JSContextDetachDebugger(JSContextRef ctx, JSDebuggerRef debugger);

JS_EXPORT JSValueRef JSContextGetStackTrace(JSContextRef ctx);

JS_EXPORT JSDebuggerRef JSDebuggerCreateAndAttach(const JSDebuggerDefinition* definition, JSContextRef ctx);

JS_EXPORT void JSDebuggerSetBreakpoint(JSDebuggerRef debugger, unsigned int line, unsigned int column);

JS_EXPORT void JSDebuggerContinue(JSDebuggerRef debugger);

JS_EXPORT void JSDebuggerStepIntoStatement(JSDebuggerRef debugger);

JS_EXPORT void JSDebuggerStepOverStatement(JSDebuggerRef debugger);

JS_EXPORT void JSDebuggerStepOutOfFunction(JSDebuggerRef debugger);

JS_EXPORT void JSDebuggerRecompile(JSContextRef ctx, JSDebuggerRef debugger);

//JS_EXPORT void JSStackFrameGetDesc(JSDebuggerStackListRef stackList, unsigned int index, JSStackFrameDesc** desc);

//JS_EXPORT void JSDebuggerGetStackList(JSContextRef ctx, JSDebuggerStackListRef* stackList, unsigned int *stackSize);

JS_EXPORT void JSStackFrameGetDesc(JSDebuggerCallFrameRef debuggerFrame, JSStackFrameDesc* desc);

JS_EXPORT void JSStackFrameGetDescAtIndex(JSDebuggerCallFrameRef debuggerFrame, unsigned int index, JSStackFrameDesc* desc);

JS_EXPORT void JSDebuggerGetStackSize(JSContextRef ctx, unsigned int *stackSize);

#ifdef __cplusplus
}
#endif
#endif /* JSDebuggerRef_h */