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

#include "config.h"

#if ENABLE(SHARED_WORKERS)

#include "DefaultSharedWorkerRepository.h"

#include "Document.h"
#include "ExceptionCode.h"
#include "InspectorInstrumentation.h"
#include "MessageEvent.h"
#include "MessagePort.h"
#include "NotImplemented.h"
#include "PageGroup.h"
#include "PlatformStrategies.h"
#include "SecurityOrigin.h"
#include "SharedWorker.h"
#include "SharedWorkerGlobalScope.h"
#include "SharedWorkerStrategy.h"
#include "SharedWorkerThread.h"
#include "WorkerLoaderProxy.h"
#include "WorkerReportingProxy.h"
#include "WorkerScriptLoader.h"
#include "WorkerScriptLoaderClient.h"
#include <inspector/ScriptCallStack.h>
#include <wtf/NeverDestroyed.h>

namespace WebCore {

class SharedWorkerProxy final : public ThreadSafeRefCounted<SharedWorkerProxy>, public WorkerLoaderProxy, public WorkerReportingProxy {
public:
    static Ref<SharedWorkerProxy> create(const String& name, const URL& url, PassRefPtr<SecurityOrigin> origin) { return adoptRef(*new SharedWorkerProxy(name, url, origin)); }

    void setThread(PassRefPtr<SharedWorkerThread> thread) { m_thread = thread; }
    SharedWorkerThread* thread() { return m_thread.get(); }
    bool isClosing() const { return m_closing; }
    URL url() const { return m_url.copy(); }
    String name() const { return m_name.isolatedCopy(); }
    bool matches(const String& name, PassRefPtr<SecurityOrigin> origin, const URL& urlToMatch) const;

    // WorkerLoaderProxy
    virtual void postTaskToLoader(ScriptExecutionContext::Task) override;
    virtual bool postTaskForModeToWorkerGlobalScope(ScriptExecutionContext::Task, const String&) override;

    // WorkerReportingProxy
    virtual void postExceptionToWorkerObject(const String& errorMessage, int lineNumber, int columnNumber, const String& sourceURL) override;
    virtual void postConsoleMessageToWorkerObject(MessageSource, MessageLevel, const String& message, int lineNumber, int columnNumber, const String& sourceURL) override;
#if ENABLE(INSPECTOR)
    virtual void postMessageToPageInspector(const String&) override;
#endif
    virtual void workerGlobalScopeClosed() override;
    virtual void workerGlobalScopeDestroyed() override;

    // Updates the list of the worker's documents, per section 4.5 of the WebWorkers spec.
    void addToWorkerDocuments(ScriptExecutionContext*);

    bool isInWorkerDocuments(Document* document) { return m_workerDocuments.contains(document); }

    // Removes a detached document from the list of worker's documents. May set the closing flag if this is the last document in the list.
    void documentDetached(Document*);

    GroupSettings* groupSettings() const; // Page GroupSettings used by worker thread.

private:
    SharedWorkerProxy(const String& name, const URL&, PassRefPtr<SecurityOrigin>);
    void close();

    bool m_closing;
    String m_name;
    URL m_url;
    // The thread is freed when the proxy is destroyed, so we need to make sure that the proxy stays around until the SharedWorkerGlobalScope exits.
    RefPtr<SharedWorkerThread> m_thread;
    RefPtr<SecurityOrigin> m_origin;
    HashSet<Document*> m_workerDocuments;
    // Ensures exclusive access to the worker documents. Must not grab any other locks (such as the DefaultSharedWorkerRepository lock) while holding this one.
    Mutex m_workerDocumentsLock;
};

SharedWorkerProxy::SharedWorkerProxy(const String& name, const URL& url, PassRefPtr<SecurityOrigin> origin)
    : m_closing(false)
    , m_name(name.isolatedCopy())
    , m_url(url.copy())
    , m_origin(origin)
{
    // We should be the sole owner of the SecurityOrigin, as we will free it on another thread.
    ASSERT(m_origin->hasOneRef());
}

bool SharedWorkerProxy::matches(const String& name, PassRefPtr<SecurityOrigin> origin, const URL& urlToMatch) const
{
    // If the origins don't match, or the names don't match, then this is not the proxy we are looking for.
    if (!origin->equal(m_origin.get()))
        return false;

    // If the names are both empty, compares the URLs instead per the Web Workers spec.
    if (name.isEmpty() && m_name.isEmpty())
        return urlToMatch == url();

    return name == m_name;
}

void SharedWorkerProxy::postTaskToLoader(ScriptExecutionContext::Task task)
{
    MutexLocker lock(m_workerDocumentsLock);

    if (isClosing())
        return;

    // If we aren't closing, then we must have at least one document.
    ASSERT(m_workerDocuments.size());

    // Just pick an arbitrary active document from the HashSet and pass load requests to it.
    // FIXME: Do we need to deal with the case where the user closes the document mid-load, via a shadow document or some other solution?
    Document* document = *(m_workerDocuments.begin());
    document->postTask(WTF::move(task));
}

bool SharedWorkerProxy::postTaskForModeToWorkerGlobalScope(ScriptExecutionContext::Task task, const String& mode)
{
    if (isClosing())
        return false;
    ASSERT(m_thread);
    m_thread->runLoop().postTaskForMode(WTF::move(task), mode);
    return true;
}

GroupSettings* SharedWorkerProxy::groupSettings() const
{
    if (isClosing())
        return 0;
    ASSERT(m_workerDocuments.size());
    // Just pick the first active document, and use the groupsettings of that page.
    Document* document = *(m_workerDocuments.begin());
    if (document->page())
        return &document->page()->group().groupSettings();

    return 0;
}

void SharedWorkerProxy::postExceptionToWorkerObject(const String& errorMessage, int lineNumber, int columnNumber, const String& sourceURL)
{
    MutexLocker lock(m_workerDocumentsLock);
    StringCapture capturedErrorMessage(errorMessage);
    StringCapture capturedSourceURL(sourceURL);
    for (auto& document : m_workerDocuments)
        document->postTask([capturedErrorMessage, lineNumber, columnNumber, capturedSourceURL] (ScriptExecutionContext& context) {
            context.reportException(capturedErrorMessage.string(), lineNumber, columnNumber, capturedSourceURL.string(), nullptr);
        });
}

void SharedWorkerProxy::postConsoleMessageToWorkerObject(MessageSource source, MessageLevel level, const String& message, int lineNumber, int columnNumber, const String& sourceURL)
{
    MutexLocker lock(m_workerDocumentsLock);
    StringCapture capturedMessage(message);
    StringCapture capturedSourceURL(sourceURL);
    for (auto& document : m_workerDocuments)
        document->postTask([source, level, capturedMessage, capturedSourceURL, lineNumber, columnNumber] (ScriptExecutionContext& context) {
            context.addConsoleMessage(source, level, capturedMessage.string(), capturedSourceURL.string(), lineNumber, columnNumber);
        });
}

#if ENABLE(INSPECTOR)
void SharedWorkerProxy::postMessageToPageInspector(const String&)
{
    notImplemented();
}
#endif

void SharedWorkerProxy::workerGlobalScopeClosed()
{
    if (isClosing())
        return;
    close();
}

void SharedWorkerProxy::workerGlobalScopeDestroyed()
{
    // The proxy may be freed by this call, so do not reference it any further.
    DefaultSharedWorkerRepository::instance().removeProxy(this);
}

void SharedWorkerProxy::addToWorkerDocuments(ScriptExecutionContext* context)
{
    // Nested workers are not yet supported, so passed-in context should always be a Document.
    ASSERT(context);
    ASSERT(!isClosing());
    MutexLocker lock(m_workerDocumentsLock);
    m_workerDocuments.add(downcast<Document>(context));
}

void SharedWorkerProxy::documentDetached(Document* document)
{
    if (isClosing())
        return;
    // Remove the document from our set (if it's there) and if that was the last document in the set, mark the proxy as closed.
    MutexLocker lock(m_workerDocumentsLock);
    m_workerDocuments.remove(document);
    if (!m_workerDocuments.size())
        close();
}

void SharedWorkerProxy::close()
{
    ASSERT(!isClosing());
    m_closing = true;
    // Stop the worker thread - the proxy will stay around until we get workerThreadExited() notification.
    if (m_thread)
        m_thread->stop();
}

class SharedWorkerConnectTask : public ScriptExecutionContext::Task {
public:
    SharedWorkerConnectTask(MessagePortChannel* channel)
        : ScriptExecutionContext::Task([channel] (ScriptExecutionContext& context) {
            RefPtr<MessagePort> port = MessagePort::create(context);
            port->entangle(std::unique_ptr<MessagePortChannel>(channel));
            ASSERT_WITH_SECURITY_IMPLICATION(is<WorkerGlobalScope>(context));
            WorkerGlobalScope& workerGlobalScope = downcast<WorkerGlobalScope>(context);
            // Since close() stops the thread event loop, this should not ever get called while closing.
            ASSERT(!workerGlobalScope.isClosing());
            ASSERT_WITH_SECURITY_IMPLICATION(workerGlobalScope.isSharedWorkerGlobalScope());
            workerGlobalScope.dispatchEvent(createConnectEvent(port));
        })
    {
    }
};

// Loads the script on behalf of a worker.
class SharedWorkerScriptLoader : public RefCounted<SharedWorkerScriptLoader>, private WorkerScriptLoaderClient {
public:
    SharedWorkerScriptLoader(PassRefPtr<SharedWorker>, std::unique_ptr<MessagePortChannel>, PassRefPtr<SharedWorkerProxy>);
    void load(const URL&);

private:
    // WorkerScriptLoaderClient callbacks
    virtual void didReceiveResponse(unsigned long identifier, const ResourceResponse&);
    virtual void notifyFinished();

    RefPtr<SharedWorker> m_worker;
    std::unique_ptr<MessagePortChannel> m_port;
    RefPtr<SharedWorkerProxy> m_proxy;
    RefPtr<WorkerScriptLoader> m_scriptLoader;
};

SharedWorkerScriptLoader::SharedWorkerScriptLoader(PassRefPtr<SharedWorker> worker, std::unique_ptr<MessagePortChannel> port, PassRefPtr<SharedWorkerProxy> proxy)
    : m_worker(worker)
    , m_port(WTF::move(port))
    , m_proxy(proxy)
{
}

void SharedWorkerScriptLoader::load(const URL& url)
{
    // Stay alive (and keep the SharedWorker and JS wrapper alive) until the load finishes.
    this->ref();
    m_worker->setPendingActivity(m_worker.get());

    // Mark this object as active for the duration of the load.
    m_scriptLoader = WorkerScriptLoader::create();
    m_scriptLoader->loadAsynchronously(m_worker->scriptExecutionContext(), url, DenyCrossOriginRequests, this);
}

void SharedWorkerScriptLoader::didReceiveResponse(unsigned long identifier, const ResourceResponse&)
{
    InspectorInstrumentation::didReceiveScriptResponse(m_worker->scriptExecutionContext(), identifier);
}

void SharedWorkerScriptLoader::notifyFinished()
{
    // FIXME: This method is not guaranteed to be invoked if we are loading from WorkerGlobalScope (see comment for WorkerScriptLoaderClient::notifyFinished()).
    // We need to address this before supporting nested workers.

    // Hand off the just-loaded code to the repository to start up the worker thread.
    if (m_scriptLoader->failed())
        m_worker->dispatchEvent(Event::create(eventNames().errorEvent, false, true));
    else {
        InspectorInstrumentation::scriptImported(m_worker->scriptExecutionContext(), m_scriptLoader->identifier(), m_scriptLoader->script());
        DefaultSharedWorkerRepository::instance().workerScriptLoaded(*m_proxy, m_worker->scriptExecutionContext()->userAgent(m_scriptLoader->url()),
            m_scriptLoader->script(), WTF::move(m_port),
            m_worker->scriptExecutionContext()->contentSecurityPolicy()->deprecatedHeader(),
            m_worker->scriptExecutionContext()->contentSecurityPolicy()->deprecatedHeaderType());
    }
    m_worker->unsetPendingActivity(m_worker.get());
    this->deref(); // This frees this object - must be the last action in this function.
}

DefaultSharedWorkerRepository& DefaultSharedWorkerRepository::instance()
{
    static std::once_flag onceFlag;
    static LazyNeverDestroyed<DefaultSharedWorkerRepository> instance;
    std::call_once(onceFlag, []{
        instance.construct();
    });

    return instance;
}

bool DefaultSharedWorkerRepository::isAvailable()
{
    return platformStrategies()->sharedWorkerStrategy()->isAvailable();
}

void DefaultSharedWorkerRepository::workerScriptLoaded(SharedWorkerProxy& proxy, const String& userAgent, const String& workerScript, std::unique_ptr<MessagePortChannel> port, const String& contentSecurityPolicy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType)
{
    MutexLocker lock(m_lock);
    if (proxy.isClosing())
        return;

    // Another loader may have already started up a thread for this proxy - if so, just send a connect to the pre-existing thread.
    if (!proxy.thread()) {
        RefPtr<SharedWorkerThread> thread = SharedWorkerThread::create(proxy.name(), proxy.url(), userAgent, proxy.groupSettings(), workerScript, proxy, proxy, DontPauseWorkerGlobalScopeOnStart, contentSecurityPolicy, contentSecurityPolicyType);
        proxy.setThread(thread);
        thread->start();
    }

    proxy.thread()->runLoop().postTask(SharedWorkerConnectTask(port.release()));
}

bool DefaultSharedWorkerRepository::hasSharedWorkers(Document* document)
{
    MutexLocker lock(m_lock);
    for (unsigned i = 0; i < m_proxies.size(); i++) {
        if (m_proxies[i]->isInWorkerDocuments(document))
            return true;
    }
    return false;
}

void DefaultSharedWorkerRepository::removeProxy(SharedWorkerProxy* proxy)
{
    MutexLocker lock(m_lock);
    for (unsigned i = 0; i < m_proxies.size(); i++) {
        if (proxy == m_proxies[i].get()) {
            m_proxies.remove(i);
            return;
        }
    }
}

void DefaultSharedWorkerRepository::documentDetached(Document* document)
{
    MutexLocker lock(m_lock);
    for (unsigned i = 0; i < m_proxies.size(); i++)
        m_proxies[i]->documentDetached(document);
}

void DefaultSharedWorkerRepository::connectToWorker(PassRefPtr<SharedWorker> worker, std::unique_ptr<MessagePortChannel> port, const URL& url, const String& name, ExceptionCode& ec)
{
    MutexLocker lock(m_lock);
    ASSERT(worker->scriptExecutionContext()->securityOrigin()->canAccess(&SecurityOrigin::create(url).get()));
    // Fetch a proxy corresponding to this SharedWorker.
    RefPtr<SharedWorkerProxy> proxy = getProxy(name, url);

    // FIXME: Why is this done even if we are raising an exception below?
    proxy->addToWorkerDocuments(worker->scriptExecutionContext());

    if (proxy->url() != url) {
        // Proxy already existed under alternate URL - return an error.
        ec = URL_MISMATCH_ERR;
        return;
    }
    // If proxy is already running, just connect to it - otherwise, kick off a loader to load the script.
    if (proxy->thread())
        proxy->thread()->runLoop().postTask(SharedWorkerConnectTask(port.release()));
    else {
        RefPtr<SharedWorkerScriptLoader> loader = adoptRef(new SharedWorkerScriptLoader(worker, WTF::move(port), proxy.release()));
        loader->load(url);
    }
}

// Creates a new SharedWorkerProxy or returns an existing one from the repository. Must only be called while the repository mutex is held.
PassRefPtr<SharedWorkerProxy> DefaultSharedWorkerRepository::getProxy(const String& name, const URL& url)
{
    // Look for an existing worker, and create one if it doesn't exist.
    // Items in the cache are freed on another thread, so do a threadsafe copy of the URL before creating the origin,
    // to make sure no references to external strings linger.
    RefPtr<SecurityOrigin> origin = SecurityOrigin::create(url.copy());
    for (unsigned i = 0; i < m_proxies.size(); i++) {
        if (!m_proxies[i]->isClosing() && m_proxies[i]->matches(name, origin, url))
            return m_proxies[i];
    }
    // Proxy is not in the repository currently - create a new one.
    RefPtr<SharedWorkerProxy> proxy = SharedWorkerProxy::create(name, url, origin.release());
    m_proxies.append(proxy);
    return proxy.release();
}

DefaultSharedWorkerRepository::DefaultSharedWorkerRepository()
{
}

DefaultSharedWorkerRepository::~DefaultSharedWorkerRepository()
{
}

} // namespace WebCore

#endif // ENABLE(SHARED_WORKERS)
