/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Swise Corporation.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSProfilerPrivate.h"

#include "APICast.h"
#include "LegacyProfiler.h"

#include "JSGlobalObject.h"

#include "ObjectConstructor.h"

#include "JSONObject.h"

using namespace JSC;

JSValue fillCallNode(ExecState* exec, const JSC::ProfileNode::Call& call)
{
	JSObject* nodes = constructEmptyObject(exec);
	nodes->putDirectIndex(exec, 0, jsNumber(call.startTime()));
	nodes->putDirectIndex(exec, 1, jsNumber(call.elapsedTime()));

	return nodes;
}

JSValue fillNodes(ExecState* exec, ProfileNode* node)
{
	JSObject* result = constructEmptyObject(exec);

	if (!node->functionName().isEmpty())
		result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("functionName"))->identifier(&exec->vm())), jsString(exec, node->functionName()));

	if (!node->url().isEmpty())
		result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("url"))->identifier(&exec->vm())), jsString(exec, node->url()));

	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("lineNumber"))->identifier(&exec->vm())), jsNumber(node->lineNumber()));
	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("columnNumber"))->identifier(&exec->vm())), jsNumber(node->columnNumber()));

	JSObject* nodes = constructEmptyObject(exec);

	unsigned int index = 0;
	for (RefPtr<JSC::ProfileNode> profileNode : node->children())
	{
		nodes->putDirectIndex(exec, index++, fillNodes(exec, profileNode.get()));
	}

	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("nodes"))->identifier(&exec->vm())), nodes);
	
	JSObject* calls = constructEmptyObject(exec);

	double startTime = node->calls()[0].startTime();
	double endTime = node->calls().last().startTime() + node->calls().last().elapsedTime();

	double totalTime = 0;
	index = 0;
	for (const JSC::ProfileNode::Call& call : node->calls())
	{
		totalTime += call.elapsedTime();
		calls->putDirectIndex(exec, index++, fillCallNode(exec, call));
	}

	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("numberOfCalls"))->identifier(&exec->vm())), jsNumber(node->calls().size()));
	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("startTime"))->identifier(&exec->vm())), jsNumber(startTime));
	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("endTime"))->identifier(&exec->vm())), jsNumber(endTime));
	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("totalTime"))->identifier(&exec->vm())), jsNumber(totalTime));
	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("hash"))->identifier(&exec->vm())), jsNumber(node->callIdentifier().hash()));

	result->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("calls"))->identifier(&exec->vm())), calls);

	return result;
}

void fillProfile(ExecState* exec, Profile* profile, JSObject* jsProfile)
{
	jsProfile->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("title"))->identifier(&exec->vm())), jsString(exec, profile->title()));

	jsProfile->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("uid"))->identifier(&exec->vm())), jsNumber(profile->uid()));

	jsProfile->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String("profile"))->identifier(&exec->vm())), fillNodes(exec, profile->rootNode()));
}

JSValue toJS(ExecState* exec, Profile* profile)
{
	JSObject* result = constructEmptyObject(exec);

	fillProfile(exec, profile, result);

	return result;
}

void JSStartProfiling(JSContextRef ctx, JSStringRef title)
{
	ExecState* exec = toJS(ctx);
	LegacyProfiler* profiler = LegacyProfiler::profiler();
	RefPtr<Stopwatch> profilerStopwatch = Stopwatch::create();
	profilerStopwatch->start();
	profiler->startProfiling(exec, title->string(), profilerStopwatch.release());
}

JSValueRef JSEndProfilingWithReport(JSContextRef ctx, JSStringRef title)
{
	ExecState* exec = toJS(ctx);
	LegacyProfiler* profiler = LegacyProfiler::profiler();
	auto profile = profiler->stopProfiling(exec, title->string());

	JSGlobalObject* globalObject = JSGlobalObject::create(exec->vm(), JSGlobalObject::createStructure(exec->vm(), jsNull()));

	return toRef(globalObject->globalExec(), toJS(globalObject->globalExec(), profile.leakRef()));
}

JSStringRef JSEndProfilingWithJSON(JSContextRef ctx, JSStringRef title)
{
	ExecState* exec = toJS(ctx);
	LegacyProfiler* profiler = LegacyProfiler::profiler();
	auto profile = profiler->stopProfiling(exec, title->string());

	JSGlobalObject* globalObject = JSGlobalObject::create(exec->vm(), JSGlobalObject::createStructure(exec->vm(), jsNull()));

	const String str = JSONStringify(globalObject->globalExec(), toJS(globalObject->globalExec(), profile.leakRef()), 0);

	return OpaqueJSString::create(str).leakRef();
}

