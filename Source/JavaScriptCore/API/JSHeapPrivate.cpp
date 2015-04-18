#include "config.h"
#include "JSHeapPrivate.h"

#include "APICast.h"
#include "OpaqueJSString.h"

#include "ObjectConstructor.h"

#include "JSONObject.h"

#include "HeapStatistics.h"

using namespace JSC;

void JSHeapStatistics(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);

	auto heap = exec->heap();

	JSC::HeapStatistics::showObjectStatistics(heap);

	JSLockHolder lock(exec->vm());
	std::unique_ptr<TypeCountSet> jsObjectTypeNames(exec->vm().heap.objectTypeCounts());
	typedef TypeCountSet::const_iterator Iterator;
	Iterator end = jsObjectTypeNames->end();
	HashMap<String, int> typeCountMap;
	for (Iterator current = jsObjectTypeNames->begin(); current != end; ++current)
		//typeCountMap.set(current->key, current->value);
		std::printf("Type: %s; Count: %i", current->key, current->value);
}

JSValueRef JSHeapGetStructure(JSContextRef ctx)
{
	ExecState* exec = toJS(ctx);

	auto heap = exec->heap();

	JSC::HeapStatistics::showObjectStatistics(heap);

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