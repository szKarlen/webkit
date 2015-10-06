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

#ifndef JSDebuggerBase_h
#define JSDebuggerBase_h

#ifdef __cplusplus
extern "C" {
#endif
	struct JSStackFrameDesc2
	{
	public:
		JSStringRef functionName;
		JSValueRef thisObject;
		JSC::DebuggerCallFrame::Type type;
		JSValueRef global;
		JSValueRef scope;
	};
	typedef size_t SourceID;

	typedef void (*JSDebuggerCallEventCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID);
	
	typedef void(*JSDebuggerAtStatementCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID, JSDebuggerCallFrameRef callFrame);
	
	typedef void(*JSDebuggerReturnEventCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID);
	
	typedef void(*JSDebuggerExceptionCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID, JSValueRef exception);
	
	typedef void(*JSDebuggerWillExecuteProgramCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID);
	
	typedef void(*JSDebuggerDidExecuteProgramCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID);
	
	typedef void(*JSDebuggerDidReachBreakpointCallback) (JSObjectRef object, int lineNumber, int columnNumber, SourceID sourceID, JSDebuggerCallFrameRef callFrame);
	
	typedef void(*JSDebuggerSourceParsedCallback) (JSContextRef ctx, SourceID sourceID, JSStringRef url, bool hasException);
	
	typedef struct {
		JSDebuggerCallEventCallback          callEvent;
		JSDebuggerAtStatementCallback        atStatement;
		JSDebuggerReturnEventCallback        returnEvent;
		JSDebuggerExceptionCallback			 exception;
		JSDebuggerWillExecuteProgramCallback willExecuteProgram;
		JSDebuggerDidExecuteProgramCallback  didExecuteProgram;
		JSDebuggerDidReachBreakpointCallback didReachBreakpoint;
		JSDebuggerSourceParsedCallback       sourceParsed;
		JSDebuggerExceptionCallback			 breakpointException;
	} JSDebuggerDefinition;

#ifdef __cplusplus
}
#endif

#endif