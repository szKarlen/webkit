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