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

//void JSDebugger::callEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//	if (callEventCallback)
//	{
//		callEventCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::atStatement(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//    if (atStatementCallback)
//	{
//		atStatementCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::returnEvent(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//    if (returnEventCallback)
//	{
//		returnEventCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::exception(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber, bool hasHandler)
//{
//    if (exceptionCallback)
//	{
//		exceptionCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::willExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//    if (willExecuteProgramCallback)
//	{
//		willExecuteProgramCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::didExecuteProgram(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//    if (didExecuteProgramCallback)
//	{
//		didExecuteProgramCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::didReachBreakpoint(const DebuggerCallFrame& debuggerCallFrame, intptr_t sourceID, int lineNumber, int columnNumber)
//{
//    if (didReachBreakpointCallback)
//	{
//		didReachBreakpointCallback(toRef(debuggerCallFrame.vmEntryGlobalObject()), lineNumber, columnNumber);
//	}
//}
//
//void JSDebugger::sourceParsed(ExecState* exec, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
//{
//    if (sourceParsedCallback)
//	{
//		sourceParsedCallback(toRef(exec), !errorMessage.isEmpty());
//	}
//}

//void JSDebugger::callEvent(CallFrame* callFrame)
//{
//	std::printf("callEvent");
//	if (callEventCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		callEventCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}
//
//void JSDebugger::atStatement(CallFrame* callFrame)
//{
//	std::printf("atStatement");
//	if (atStatementCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		atStatementCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}
//
//void JSDebugger::returnEvent(CallFrame* callFrame)
//{
//	std::printf("returnEvent");
//	if (returnEventCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		returnEventCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}

//void JSDebugger::exception(CallFrame* callFrame, JSValue exceptionValue, bool hasHandler)
//{
//	std::printf("exception");
//	if (exceptionCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		exceptionCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}

//void JSDebugger::willExecuteProgram(CallFrame* callFrame)
//{
//	std::printf("willExecuteProgram");
//	if (willExecuteProgramCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		willExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}
//
//void JSDebugger::didExecuteProgram(CallFrame* callFrame)
//{
//	std::printf("didExecuteProgram");
//	if (didExecuteProgramCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		didExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}
//
//void JSDebugger::didReachBreakpoint(CallFrame* callFrame)
//{
//	std::printf("didReachBreakpoint");
//	if (didReachBreakpointCallback)
//	{
//		intptr_t sourceID = DebuggerCallFrame::sourceIDForCallFrame(callFrame);
//		TextPosition position = DebuggerCallFrame::positionForCallFrame(callFrame);
//
//		didReachBreakpointCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
//	}
//}

//
//void JSDebugger::sourceParsed(ExecState* exec, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
//{
//	if (sourceParsedCallback)
//	{
//		sourceParsedCallback(toRef(exec), !errorMessage.isEmpty());
//	}
//	std::printf("sourceParsed");
//}
//
//void JSDebugger::handlePause(ReasonForPause reason, JSGlobalObject* exec)
//{
//	switch (reason)
//	{
//	/*case JSC::Debugger::NotPaused:
//		break;
//	case JSC::Debugger::PausedForException:
//		break;*/
//	case JSC::Debugger::PausedAtStatement:
//		auto callFrame = currentDebuggerCallFrame();
//		if (atStatementCallback)
//		{
//			this->atStatementCallback(toRef(exec), callFrame->line(), callFrame->column());
//		}
//		break;
//	/*case JSC::Debugger::PausedAfterCall:
//		break;
//	case JSC::Debugger::PausedBeforeReturn:
//		break;
//	case JSC::Debugger::PausedAtStartOfProgram:
//		break;
//	case JSC::Debugger::PausedAtEndOfProgram:
//		break;
//	case JSC::Debugger::PausedForBreakpoint:
//		break;
//	default:
//		break;*/
//	}
//	std::printf("Handle");
//}

void JSDebugger::sourceParsed(ExecState* exec, SourceProvider* sourceProvider, int errorLine, const String& errorMessage)
{
	m_source = sourceProvider->asID();

    if (sourceParsedCallback)
	{
		sourceParsedCallback(toRef(exec), !errorMessage.isEmpty());
	}
}

void JSDebugger::notifyDoneProcessingDebuggerEvents()
{
	printf("notifyDoneProcessingDebuggerEvents\n");
}

//void JSDebugger::handlePause(Debugger::ReasonForPause reason, JSGlobalObject* vmEntryGlobalObject)
//{
//	printf("reason %i", reason);
//
//	auto callFrame = currentDebuggerCallFrame();
//	Breakpoint hit;
//
//	switch (reason)
//	{
//	/*case JSC::Debugger::NotPaused:
//		break;
//	case JSC::Debugger::PausedForException:
//		break;*/
//	case JSC::Debugger::PausedAtStatement:
//		if (this->hasBreakpoint(currentSourceID(), TextPosition(OrdinalNumber::fromZeroBasedInt(callFrame->line()), OrdinalNumber::fromZeroBasedInt(callFrame->column())), &hit))
//		{
//			if (didReachBreakpointCallback)
//			{
//				this->didReachBreakpointCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//			}
//		}
//		if (atStatementCallback)
//		{
//			this->atStatementCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//		}
//		break;
//	case JSC::Debugger::PausedAfterCall:
//		if (callEventCallback)
//		{
//			this->callEventCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//		}
//		break;
//	case JSC::Debugger::PausedBeforeReturn:
//		if (returnEventCallback)
//		{
//			this->returnEventCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//		}
//		break;
//	/*case JSC::Debugger::PausedAtStartOfProgram:
//		break;*/
//	case JSC::Debugger::PausedAtEndOfProgram:
//		std::printf("executed callback");
//		if (didExecuteProgramCallback)
//		{
//			didExecuteProgramCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//		}
//		break;
//	case JSC::Debugger::PausedForBreakpoint:
//		if (didReachBreakpointCallback)
//		{
//			this->didReachBreakpointCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column());
//		}
//		break;
//	}
//}

bool JSDebugger::needPauseHandling(JSGlobalObject* globalObject)
{
	return true;
}

JSC::SourceID JSDebugger::currentSourceID() const
{
	return m_source;
}

/*
prev debugger impl
*/
//void JSDebugger::preHandlePause(Debugger::ReasonForPause reason, JSGlobalObject* vmEntryGlobalObject, CallFrame* sourceCallFrame)
//{
//	
//}
void JSDebugger::handlePause(JSGlobalObject* vmEntryGlobalObject, Debugger::ReasonForPause reason)
{
	printf("reason %i", reason);
	auto callFrame = currentDebuggerCallFrame();
	switch (reason)
	{
	case JSC::Debugger::PausedForException:
		if (exceptionCallback)
		{
			intptr_t sourceID = callFrame->sourceID();
			TextPosition position = callFrame->position();

			exceptionCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
		}
		break;
	case JSC::Debugger::PausedAtStatement:
	case JSC::Debugger::PausedAfterCall:
	case JSC::Debugger::PausedBeforeReturn:
		if (atStatementCallback)
		{
			this->atStatementCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column(), toRef(callFrame));
		}
		break;
	case JSC::Debugger::PausedAtStartOfProgram:
		if (willExecuteProgramCallback)
		{
			intptr_t sourceID = callFrame->sourceID();
			TextPosition position = callFrame->position();

			willExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
		}
		break;
	case JSC::Debugger::PausedAtEndOfProgram:
		if (didExecuteProgramCallback)
		{
			intptr_t sourceID = callFrame->sourceID();
			TextPosition position = callFrame->position();

			didExecuteProgramCallback(toRef(callFrame->vmEntryGlobalObject()), position.m_line.zeroBasedInt(), position.m_column.zeroBasedInt());
		}
		break;
	case JSC::Debugger::PausedForBreakpoint:
		if (didReachBreakpointCallback)
		{
			this->didReachBreakpointCallback(toRef(vmEntryGlobalObject), callFrame->line(), callFrame->column(), toRef(callFrame));
		}
		break;
	default:
		break;
	}
}