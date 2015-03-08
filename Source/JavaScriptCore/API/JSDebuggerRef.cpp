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
#include "JSDebuggerRef.h"
#include "InitializeThreading.h"

#include "Interpreter.h"
#include "CallFrame.h"
#include "VM.h"
#include "Register.h"

#include "DebuggerCallFrame.h"
#include "CallFrameInlines.h"

#include "CodeBlock.h"

#include "JSDebugger.h"
#include "VM.h"

using namespace JSC;

JSDebuggerRef JSDebuggerCreate(const JSDebuggerDefinition* definition)
{
	initializeThreading();
	JSDebugger* debugger = new JSDebugger(definition);

	auto ref = adoptRef(debugger).leakRef();
	
	return toRef(ref);
}

JSDebuggerRef JSDebuggerRetain(JSDebuggerRef debugger)
{
	toJS(debugger)->ref();
	return debugger;
}

void JSDebuggerRelease(JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	jsDebugger->deref();
}

void JSContextAttachDebugger(JSContextRef ctx, JSDebuggerRef debugger)
{
	auto exec = toJS(ctx);
	
	auto jsDebugger = toJS(debugger);

	jsDebugger->attach(exec->vmEntryGlobalObject());
}

void JSContextDetachDebugger(JSContextRef ctx, JSDebuggerRef debugger)
{
	auto exec = toJS(ctx);

	auto jsDebugger = toJS(debugger);

	jsDebugger->detach(exec->vmEntryGlobalObject(), JSC::Debugger::ReasonForDetach::TerminatingDebuggingSession);
}

JSValueRef JSContextGetStackTrace(JSContextRef ctx)
{
	auto exec = toJS(ctx);

	Vector<StackFrame> stackTrace;
	
	exec->vm().interpreter->getStackTraceList(stackTrace);

	auto str = exec->vm().interpreter->stackTraceAsString(exec, stackTrace);
	
	return toRef(exec, str);
}

JSDebuggerRef JSDebuggerCreateAndAttach(const JSDebuggerDefinition* definition, JSContextRef ctx)
{
	JSDebugger* debugger = new JSDebugger(definition);

	auto exec = toJS(ctx);

	debugger->attach(exec->vmEntryGlobalObject());
	
	//debugger->recompileAllJSFunctions(&exec->vm());

	auto ref = adoptRef(debugger).leakRef();

	return toRef(ref);
}

void JSDebuggerSetBreakpoint(JSDebuggerRef debugger, unsigned int line, unsigned int column)
{
	auto jsDebugger = toJS(debugger);
	//Breakpoint breakpoint;

	ASSERT(jsDebugger);

	/*auto callFrame = jsDebugger->currentDebuggerCallFrame();

	if (!callFrame)
	{
		std::printf("null frame");
		return;
	}*/

	//auto sourceID = callFrame->sourceID();

	JSC::Breakpoint breakpoint(jsDebugger->currentSourceID(), line, column, String("", 0), false);

	jsDebugger->setBreakpoint(breakpoint, line, column);
}

void JSDebuggerContinue(JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	jsDebugger->continueProgram();
}

void JSDebuggerStepIntoStatement(JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	jsDebugger->stepIntoStatement();
}

void JSDebuggerStepOverStatement(JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	jsDebugger->stepOverStatement();
}

void JSDebuggerStepOutOfFunction(JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	jsDebugger->stepOutOfFunction();
}

void JSDebuggerRecompile(JSContextRef ctx, JSDebuggerRef debugger)
{
	auto jsDebugger = toJS(debugger);

	auto exec = toJS(ctx);

	jsDebugger->recompileAllJSFunctions(&exec->vm());
}

void JSStackFrameGetDesc(JSDebuggerCallFrameRef debuggerFrame, JSStackFrameDesc* desc)
{
	auto callFrame = toJS(debuggerFrame);

	desc->functionName = OpaqueJSString::create(callFrame->functionName()).leakRef();
	desc->scope = toRef(callFrame->exec(), (JSC::JSObject*)callFrame->scope());
	desc->thisObject = toRef(callFrame->exec(), callFrame->thisValue());
	desc->type = (JSC::DebuggerCallFrame::Type) callFrame->type();
}

void JSStackFrameGetDescAtIndex(JSDebuggerCallFrameRef debuggerFrame, unsigned int index, JSStackFrameDesc* desc)
{
	/*auto exec = toJS(ctx);
	Vector<StackFrame> stack;
	toJS(ctx)->interpreter()->getStackTraceList(stack);
	auto stackFrame = &stack[index];*/
	//CallFrame* callFrame = (CallFrame*)&stack[index].executable;
	
	//std::printf("callframe is empty: %s", stackFrame->scope == NULL ? "null" : "not null");

	/*
	JSStackFrameDesc description = *desc;
	//std::printf("function: %s", !callFrame->functionName().isNull() && !callFrame->functionName().isEmpty() ? callFrame->functionName().utf8() : "");
	
	description.functionName = OpaqueJSString::create(stackFrame->friendlyFunctionName(exec)).leakRef();
	//description.global = toRef(exec, callFrame->vmEntryGlobalObject());
	description.scope = toRef(exec, stackFrame->scope.get());
	//description.thisObject = toRef(exec, callFrame->thisValue());
	description.type = (JSC::DebuggerCallFrame::Type) stackFrame->codeType;
	
	//*desc = &description;
	*/

	auto callFrame = toJS(debuggerFrame);
	unsigned int currentFrame = 0;
	while (currentFrame != index)
	{
		callFrame = callFrame->callerFrame().leakRef();
		currentFrame++;
	}
	desc->functionName = OpaqueJSString::create(callFrame->functionName()).leakRef();
	desc->scope = toRef(callFrame->exec(), (JSC::JSObject*)callFrame->scope());
	desc->thisObject = toRef(callFrame->exec(), callFrame->thisValue());
	desc->type = (JSC::DebuggerCallFrame::Type) callFrame->type();
}

void JSDebuggerGetStackSize(JSContextRef ctx, unsigned int *stackSize)
{
	auto exec = toJS(ctx);

	Vector<StackFrame> stackTrace;

	exec->vm().interpreter->getStackTraceList(stackTrace);

	//*stackList = toRef(&stackTrace);
	*stackSize = stackTrace.size();
}