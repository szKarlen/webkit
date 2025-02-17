/*
 * Copyright (C) 2008 Apple Inc.  All rights reserved.
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

#ifndef JSProfiler_h
#define JSProfiler_h

#include <JavaScriptCore/JSBase.h>
#include "OpaqueJSString.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

JS_EXPORT void JSStartProfiling(JSContextRef ctx, JSStringRef title);

/*!
@function JSStartProfiling
@abstract Enables the profler.
@param ctx The execution context to use.
@param title The title of the profile.
@result The profiler is turned on.
*/
JS_EXPORT JSValueRef JSEndProfilingWithReport(JSContextRef ctx, JSStringRef title);

/*!
@function JSEndProfiling
@abstract Disables the profler.
@param ctx The execution context to use.
@param title The title of the profile.
@result The profiler is turned off. If there is no name, the most recently started
        profile is stopped. If the name does not match any profile then no profile
        is stopped.
*/
JS_EXPORT JSStringRef JSEndProfilingWithJSON(JSContextRef ctx, JSStringRef title);

#ifdef __cplusplus
}
#endif

#endif /* JSProfiler_h */
