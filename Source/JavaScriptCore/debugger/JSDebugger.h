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

#ifndef JSDebugger_h
#define JSDebugger_h

#include "APICast.h"
#include "Debugger.h"
#include "DebuggerCallFrame.h"

#include "CallFrame.h"

#include "wtf/ThreadSafeRefCounted.h"

#include "JSBase.h"

#include "JSDebuggerBase.h"

namespace JSC {

	class JSGlobalObject;

	class JSDebugger : public JSC::Debugger, public ThreadSafeRefCounted<JSDebugger>
	{
	public:
		JSDebugger();
		JSDebugger(const JSDebuggerDefinition* definition);
		~JSDebugger();

		void sourceParsed(ExecState*, SourceProvider*, int errorLineNumber, const WTF::String& errorMessage);

		virtual bool needPauseHandling(JSC::JSGlobalObject*) override final;

		JSC::SourceID currentSourceID() const;

	protected:
		JSDebuggerSourceParsedCallback sourceParsedCallback;
		JSDebuggerExceptionCallback exceptionCallback;
		JSDebuggerAtStatementCallback atStatementCallback;
		JSDebuggerCallEventCallback callEventCallback;
		JSDebuggerReturnEventCallback returnEventCallback;
		JSDebuggerWillExecuteProgramCallback willExecuteProgramCallback;
		JSDebuggerDidExecuteProgramCallback didExecuteProgramCallback;
		JSDebuggerDidReachBreakpointCallback didReachBreakpointCallback;

		void notifyDoneProcessingDebuggerEvents() override final;
		
		virtual void handlePause(JSC::JSGlobalObject*, JSC::Debugger::ReasonForPause) override final;

		JSC::SourceID m_source = JSC::noSourceID;
	};
}
#endif /* JSDebugger_h */