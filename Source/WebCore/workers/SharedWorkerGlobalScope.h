/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SharedWorkerGlobalScope_h
#define SharedWorkerGlobalScope_h

#if ENABLE(SHARED_WORKERS)

#include "ContentSecurityPolicy.h"
#include "WorkerGlobalScope.h"

namespace WebCore {

    class MessageEvent;
    class SharedWorkerThread;

    class SharedWorkerGlobalScope : public WorkerGlobalScope {
    public:
        typedef WorkerGlobalScope Base;
        static Ref<SharedWorkerGlobalScope> create(const String& name, const URL&, const String& userAgent, std::unique_ptr<GroupSettings>, SharedWorkerThread&, const String& contentSecurityPolicy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType);
        virtual ~SharedWorkerGlobalScope();

        virtual bool isSharedWorkerGlobalScope() const override { return true; }

        // EventTarget
        virtual EventTargetInterface eventTargetInterface() const override;

        // Setters/Getters for attributes in SharedWorkerGlobalScope.idl
        DEFINE_ATTRIBUTE_EVENT_LISTENER(connect);
        String name() const { return m_name; }

        SharedWorkerThread& thread();

    private:
        SharedWorkerGlobalScope(const String& name, const URL&, const String& userAgent, std::unique_ptr<GroupSettings>, SharedWorkerThread&);
        virtual void logExceptionToConsole(const String& errorMessage, const String& sourceURL, int lineNumber, int columnNumber, PassRefPtr<Inspector::ScriptCallStack>) override;

        String m_name;
    };

    PassRefPtr<MessageEvent> createConnectEvent(PassRefPtr<MessagePort>);

} // namespace WebCore

#endif // ENABLE(SHARED_WORKERS)

#endif // SharedWorkerGlobalScope_h
