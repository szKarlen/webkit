/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "TestRunner.h"

#include "InjectedBundle.h"

namespace WTR {

void TestRunner::platformInitialize()
{
}

void TestRunner::invalidateWaitToDumpWatchdogTimer()
{
    if (!m_waitToDumpWatchdogTimer)
        return;

    CFRunLoopTimerInvalidate(m_waitToDumpWatchdogTimer.get());
    m_waitToDumpWatchdogTimer = 0;
}

static void waitUntilDoneWatchdogTimerFired(CFRunLoopTimerRef timer, void* info)
{
    InjectedBundle::shared().testRunner()->waitToDumpWatchdogTimerFired();
}

void TestRunner::initializeWaitToDumpWatchdogTimerIfNeeded()
{
    if (m_waitToDumpWatchdogTimer)
        return;

    CFTimeInterval interval = m_timeout / 1000.0;
    m_waitToDumpWatchdogTimer = adoptCF(CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + interval, 0, 0, 0, WTR::waitUntilDoneWatchdogTimerFired, NULL));
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), m_waitToDumpWatchdogTimer.get(), kCFRunLoopCommonModes);
}

JSRetainPtr<JSStringRef> TestRunner::pathToLocalResource(JSStringRef url)
{
    return JSStringRetain(url); // Do nothing on mac.
}

} // namespace WTR
