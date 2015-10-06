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
#include "JSDebuggerRef.h"
#include "InitializeThreading.h"

#include "Interpreter.h"
#include "CallFrame.h"
#include "VM.h"
#include "Register.h"

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

void JSDebuggerSetBreakpoint(JSDebuggerRef debugger, ::SourceID sourceID, unsigned int line, unsigned int column)
{
	auto jsDebugger = toJS(debugger);
	
	ASSERT(jsDebugger);

	JSC::Breakpoint breakpoint(sourceID != 0 ? sourceID : jsDebugger->currentSourceID(), line, column, String("", 0), false);

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

/*
Taken from JSJavaScriptCallFrame::scopeType(ExecState* exec)
*/
static ::ScopeType GetScopeType(ExecState* exec, DebuggerScope* scopeChain)
{
	if (!exec->argument(0).isInt32())
		return ::ScopeType::UNDEFINED;
	int index = exec->argument(0).asInt32();

	DebuggerScope::iterator end = scopeChain->end();

	bool foundLocalScope = false;
	for (DebuggerScope::iterator iter = scopeChain->begin(); iter != end; ++iter) {
		DebuggerScope* scope = iter.get();

		if (!foundLocalScope && scope->isFunctionOrEvalScope()) {
			// First function scope is the local scope, each successive one is a closure.
			if (!index)
				return ::ScopeType::LOCAL_SCOPE;
			foundLocalScope = true;
		}

		if (!index) {
			if (scope->isCatchScope())
				return ::ScopeType::CATCH_SCOPE;
			if (scope->isFunctionNameScope())
				return ::ScopeType::FUNCTION_NAME_SCOPE;
			if (scope->isWithScope())
				return ::ScopeType::WITH_SCOPE;
			if (scope->isGlobalScope()) {
				ASSERT(++iter == end);
				return ::ScopeType::GLOBAL_SCOPE;
			}
			ASSERT(scope->isFunctionOrEvalScope());
			return ::ScopeType::CLOSURE_SCOPE;
		}

		--index;
	}

	ASSERT_NOT_REACHED();
	return ::ScopeType::UNDEFINED;
}

size_t _cdecl JSCaptureStackBackTrace(JSDebuggerCallFrameRef initialFrame, unsigned int framesToSkip, unsigned int framesToCapture, JSStackFrameDesc** backTrace)
{
	if (!initialFrame)
		return 0;
	
	framesToSkip++;
	
	auto callFrame = toJS(initialFrame);
	while (--framesToSkip > 0 && callFrame)
		callFrame = callFrame->callerFrame().get();

	ExecState* exec = callFrame->exec();
	auto result = *backTrace;
	unsigned int currentFrame = 0;
	unsigned int desiredStackSize = framesToCapture - framesToSkip;
	do
	{
		JSStackFrameDesc* desc = &result[currentFrame];
		desc->functionName = OpaqueJSString::create(callFrame->functionName()).leakRef();
		desc->scope = toRef(exec, (JSC::JSObject*)callFrame->scope());
		desc->thisObject = toRef(exec, callFrame->thisValue());
		desc->type = (::CallFrameFunctionType) callFrame->type();
		desc->column = callFrame->column();
		desc->line = callFrame->line();
		desc->scopeType = ::GetScopeType(exec, callFrame->scope());
		desc->url = OpaqueJSString::create(exec->codeBlock()->ownerExecutable()->sourceURL()).leakRef();
		desc->pointer = toRef(callFrame);
		desc->sourceID = callFrame->sourceID();

		callFrame = callFrame->callerFrame() && callFrame->callerFrame()->isValid() 
			? callFrame->callerFrame().get() 
			: nullptr;

		currentFrame++;
	} while (callFrame && currentFrame != desiredStackSize);
	return currentFrame;
}

JSValueRef JSDebuggerEvaluate(JSContextRef ctx, JSDebuggerCallFrameRef debuggerFrame, JSStringRef source, JSValueRef* ex)
{
	auto exec = toJS(ctx);
	auto callFrame = toJS(debuggerFrame);
	JSValue exception;
	auto result = toRef(exec, callFrame->evaluateNonBlocking(source->string(), exception));
	if (exception && ex)
		*ex = toRef(exec, exception);
	return result;
}
