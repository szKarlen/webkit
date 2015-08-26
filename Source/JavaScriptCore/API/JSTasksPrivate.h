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

#ifndef JSTasks_h
#define JSTasks_h

#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSStringRef.h>
#include "Microtask.h"

namespace JSC {
	class Microtask;
	class ExecState;
}
typedef struct OpaqueJSTask* JSTask;

inline JSC::Microtask* toJS(JSTask task)
{
	return reinterpret_cast<JSC::Microtask*>(const_cast<OpaqueJSTask*>(task));
}

inline JSTask toRef(JSC::Microtask* task)
{
	return reinterpret_cast<JSTask>(task);
}

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*JSTasksQueueCallback)(JSContextRef ctx, JSTask task);

	JS_EXPORT void JSContextSetTasksQueueCallback(JSContextRef ctx, JSTasksQueueCallback callback);

	JS_EXPORT void JSContextExecuteTask(JSContextRef ctx, JSTask task);

	JS_EXPORT void JSContextExecuteTaskUnsafe(JSContextRef ctx, JSTask task);

	JS_EXPORT void JSTaskRetain(JSTask task);

	JS_EXPORT void JSTaskRelease(JSTask task);

#ifdef __cplusplus
}
#endif

#endif /* JSTasks_h */
