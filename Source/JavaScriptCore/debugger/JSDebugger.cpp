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
#include "JSDebugger.h"

#include "InitializeThreading.h"
#include "JSDebuggerRef.h"

using namespace JSC;

JSDebugger::JSDebugger(const JSDebuggerDefinition* definition)
	: Debugger(true),
	callEventCallback(definition->callEvent),
	atStatementCallback(definition->atStatement),
	returnEventCallback(definition->returnEvent),
	exceptionCallback(definition->exception),
	willExecuteProgramCallback(definition->willExecuteProgram),
	didExecuteProgramCallback(definition->didExecuteProgram),
	didReachBreakpointCallback(definition->didReachBreakpoint),
	sourceParsedCallback(definition->sourceParsed)
{
	initializeThreading();
}

JSDebugger::~JSDebugger()
{

}

void JSDebugger::sourceParsed(ExecState* exec, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
{
	m_source = sourceProvider->asID();
		
    if (sourceParsedCallback)
	{
		String sourceUrl = sourceProvider->url();
		JSStringRef url = OpaqueJSString::create(sourceUrl).leakRef();
		sourceParsedCallback(toRef(exec), m_source, url, !errorMessage.isEmpty());
	}
}

void JSDebugger::notifyDoneProcessingDebuggerEvents()
{
	
}

bool JSDebugger::needPauseHandling(JSGlobalObject* globalObject)
{
	return true;
}

JSC::SourceID JSDebugger::currentSourceID() const
{
	return m_source;
}

void JSDebugger::handlePause(JSGlobalObject* vmEntryGlobalObject, Debugger::ReasonForPause reason)
{
	auto callFrame = currentDebuggerCallFrame();
	::SourceID sourceID = callFrame->sourceID();
	switch (reason)
	{
	case JSC::Debugger::PausedForException:
		if (exceptionCallback)
		{
			TextPosition position = callFrame->position();
			exceptionCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt(), sourceID);
		}
		break;
	case JSC::Debugger::PausedAtStatement:
	case JSC::Debugger::PausedAfterCall:
	case JSC::Debugger::PausedBeforeReturn:
		if (atStatementCallback)
		{
			this->atStatementCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column(), sourceID, toRef(callFrame));
		}
		break;
	case JSC::Debugger::PausedAtStartOfProgram:
		if (willExecuteProgramCallback)
		{
			TextPosition position = callFrame->position();

			willExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt(), sourceID);
		}
		break;
	case JSC::Debugger::PausedAtEndOfProgram:
		if (didExecuteProgramCallback)
		{
			TextPosition position = callFrame->position();

			didExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt(), sourceID);
		}
		break;
	case JSC::Debugger::PausedForBreakpoint:
		if (didReachBreakpointCallback)
		{
			this->didReachBreakpointCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column(), sourceID, toRef(callFrame));
		}
		break;
	default:
		break;
	}
}