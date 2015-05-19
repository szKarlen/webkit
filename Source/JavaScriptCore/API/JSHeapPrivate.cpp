/*
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
#include "JSHeapPrivate.h"

#include "APICast.h"
#include "OpaqueJSString.h"

#include "ObjectConstructor.h"

#include "JSONObject.h"

#include "HeapStatistics.h"

using namespace JSC;

JSValueRef JSHeapGetGroupedByTypeObjectsCount(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);

	auto heap = exec->heap();

	JSLockHolder lock(exec->vm());
	std::unique_ptr<TypeCountSet> jsObjectTypeNames(exec->vm().heap.objectTypeCounts());
	typedef TypeCountSet::const_iterator Iterator;
	Iterator end = jsObjectTypeNames->end();
	HashMap<String, int> typeCountMap;

	JSObject* nodes = constructEmptyObject(exec);

	unsigned int index = 0;

	for (Iterator current = jsObjectTypeNames->begin(); current != end; ++current)
	{
		index++;

		nodes->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String(current->key))->identifier(&exec->vm())), jsNumber(current->value));
	}

	return toRef(nodes);
}

size_t JSHeapGetObjectsCount(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);

	auto heap = exec->heap();

	return heap->objectCount();
}

size_t JSHeapGetProtectedObjectsCount(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);
		auto heap = exec->heap();

	return heap->protectedObjectCount();
}

JSValueRef JSHeapGetGroupedByTypeProtectedObjectsCount(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);

	auto heap = exec->heap();

	JSLockHolder lock(exec->vm());
	std::unique_ptr<TypeCountSet> jsObjectTypeNames(exec->vm().heap.protectedObjectTypeCounts());
	typedef TypeCountSet::const_iterator Iterator;
	Iterator end = jsObjectTypeNames->end();
	HashMap<String, int> typeCountMap;

	JSObject* nodes = constructEmptyObject(exec);

	unsigned int index = 0;

	for (Iterator current = jsObjectTypeNames->begin(); current != end; ++current)
	{
		index++;

		nodes->putDirect(exec->vm(), PropertyName(OpaqueJSString::create(String(current->key))->identifier(&exec->vm())), jsNumber(current->value));
	}

	return toRef(nodes);
}