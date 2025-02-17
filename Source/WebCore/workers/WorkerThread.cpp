/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 *
 */

#include "config.h"

#include "WorkerThread.h"

#include "DedicatedWorkerGlobalScope.h"
#include "InspectorInstrumentation.h"
#include "ScriptSourceCode.h"
#include "SecurityOrigin.h"
#include "ThreadGlobalData.h"
#include "URL.h"
#include <utility>
#include <wtf/NeverDestroyed.h>
#include <wtf/Noncopyable.h>
#include <wtf/text/WTFString.h>

#if ENABLE(SQL_DATABASE)
#include "DatabaseManager.h"
#include "DatabaseTask.h"
#endif

#if PLATFORM(IOS)
#include "FloatingPointEnvironment.h"
#include "WebCoreThread.h"
#endif

namespace WebCore {

static std::mutex& threadSetMutex()
{
    static std::once_flag onceFlag;
    static LazyNeverDestroyed<std::mutex> mutex;

    std::call_once(onceFlag, []{
        mutex.construct();
    });

    return mutex;
}

static HashSet<WorkerThread*>& workerThreads()
{
    static NeverDestroyed<HashSet<WorkerThread*>> workerThreads;

    return workerThreads;
}

unsigned WorkerThread::workerThreadCount()
{
    std::lock_guard<std::mutex> lock(threadSetMutex());

    return workerThreads().size();
}

struct WorkerThreadStartupData {
    WTF_MAKE_NONCOPYABLE(WorkerThreadStartupData); WTF_MAKE_FAST_ALLOCATED;
public:
    WorkerThreadStartupData(const URL& scriptURL, const String& userAgent, const GroupSettings*, const String& sourceCode, WorkerThreadStartMode, const String& contentSecurityPolicy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType, const SecurityOrigin* topOrigin);

    URL m_scriptURL;
    String m_userAgent;
    std::unique_ptr<GroupSettings> m_groupSettings;
    String m_sourceCode;
    WorkerThreadStartMode m_startMode;
    String m_contentSecurityPolicy;
    ContentSecurityPolicy::HeaderType m_contentSecurityPolicyType;
    RefPtr<SecurityOrigin> m_topOrigin;
};

WorkerThreadStartupData::WorkerThreadStartupData(const URL& scriptURL, const String& userAgent, const GroupSettings* settings, const String& sourceCode, WorkerThreadStartMode startMode, const String& contentSecurityPolicy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType, const SecurityOrigin* topOrigin)
    : m_scriptURL(scriptURL.copy())
    , m_userAgent(userAgent.isolatedCopy())
    , m_sourceCode(sourceCode.isolatedCopy())
    , m_startMode(startMode)
    , m_contentSecurityPolicy(contentSecurityPolicy.isolatedCopy())
    , m_contentSecurityPolicyType(contentSecurityPolicyType)
    , m_topOrigin(topOrigin ? &topOrigin->isolatedCopy().get() : nullptr)
{
    if (!settings)
        return;

    m_groupSettings = std::make_unique<GroupSettings>();
    m_groupSettings->setLocalStorageQuotaBytes(settings->localStorageQuotaBytes());
    m_groupSettings->setIndexedDBQuotaBytes(settings->indexedDBQuotaBytes());
    m_groupSettings->setIndexedDBDatabasePath(settings->indexedDBDatabasePath().isolatedCopy());
}

WorkerThread::WorkerThread(const URL& scriptURL, const String& userAgent, const GroupSettings* settings, const String& sourceCode, WorkerLoaderProxy& workerLoaderProxy, WorkerReportingProxy& workerReportingProxy, WorkerThreadStartMode startMode, const String& contentSecurityPolicy, ContentSecurityPolicy::HeaderType contentSecurityPolicyType, const SecurityOrigin* topOrigin)
    : m_threadID(0)
    , m_workerLoaderProxy(workerLoaderProxy)
    , m_workerReportingProxy(workerReportingProxy)
    , m_startupData(std::make_unique<WorkerThreadStartupData>(scriptURL, userAgent, settings, sourceCode, startMode, contentSecurityPolicy, contentSecurityPolicyType, topOrigin))
#if ENABLE(NOTIFICATIONS) || ENABLE(LEGACY_NOTIFICATIONS)
    , m_notificationClient(0)
#endif
{
    std::lock_guard<std::mutex> lock(threadSetMutex());

    workerThreads().add(this);
}

WorkerThread::~WorkerThread()
{
    std::lock_guard<std::mutex> lock(threadSetMutex());

    ASSERT(workerThreads().contains(this));
    workerThreads().remove(this);
}

bool WorkerThread::start()
{
    // Mutex protection is necessary to ensure that m_threadID is initialized when the thread starts.
    MutexLocker lock(m_threadCreationMutex);

    if (m_threadID)
        return true;

    m_threadID = createThread(WorkerThread::workerThreadStart, this, "WebCore: Worker");

    return m_threadID;
}

void WorkerThread::workerThreadStart(void* thread)
{
    static_cast<WorkerThread*>(thread)->workerThread();
}

void WorkerThread::workerThread()
{
    // Propagate the mainThread's fenv to workers.
#if PLATFORM(IOS)
    FloatingPointEnvironment::shared().propagateMainThreadEnvironment();
#endif

    {
        MutexLocker lock(m_threadCreationMutex);
        m_workerGlobalScope = createWorkerGlobalScope(m_startupData->m_scriptURL, m_startupData->m_userAgent, WTF::move(m_startupData->m_groupSettings), m_startupData->m_contentSecurityPolicy, m_startupData->m_contentSecurityPolicyType, m_startupData->m_topOrigin.release());

        if (m_runLoop.terminated()) {
            // The worker was terminated before the thread had a chance to run. Since the context didn't exist yet,
            // forbidExecution() couldn't be called from stop().
            m_workerGlobalScope->script()->forbidExecution();
        }
    }

    WorkerScriptController* script = m_workerGlobalScope->script();
#if ENABLE(INSPECTOR)
    InspectorInstrumentation::willEvaluateWorkerScript(workerGlobalScope(), m_startupData->m_startMode);
#endif
    script->evaluate(ScriptSourceCode(m_startupData->m_sourceCode, m_startupData->m_scriptURL));
    // Free the startup data to cause its member variable deref's happen on the worker's thread (since
    // all ref/derefs of these objects are happening on the thread at this point). Note that
    // WorkerThread::~WorkerThread happens on a different thread where it was created.
    m_startupData = nullptr;

    runEventLoop();

    ThreadIdentifier threadID = m_threadID;

    ASSERT(m_workerGlobalScope->hasOneRef());

    // The below assignment will destroy the context, which will in turn notify messaging proxy.
    // We cannot let any objects survive past thread exit, because no other thread will run GC or otherwise destroy them.
    m_workerGlobalScope = 0;

    // Clean up WebCore::ThreadGlobalData before WTF::WTFThreadData goes away!
    threadGlobalData().destroy();

    // The thread object may be already destroyed from notification now, don't try to access "this".
    detachThread(threadID);
}

void WorkerThread::runEventLoop()
{
    // Does not return until terminated.
    m_runLoop.run(m_workerGlobalScope.get());
}

void WorkerThread::stop()
{
    // Mutex protection is necessary because stop() can be called before the context is fully created.
    MutexLocker lock(m_threadCreationMutex);

    // Ensure that tasks are being handled by thread event loop. If script execution weren't forbidden, a while(1) loop in JS could keep the thread alive forever.
    if (m_workerGlobalScope) {
        m_workerGlobalScope->script()->scheduleExecutionTermination();

#if ENABLE(SQL_DATABASE)
        DatabaseManager::manager().interruptAllDatabasesForContext(m_workerGlobalScope.get());
#endif
        m_runLoop.postTaskAndTerminate({ ScriptExecutionContext::Task::CleanupTask, [] (ScriptExecutionContext& context ) {
            WorkerGlobalScope& workerGlobalScope = downcast<WorkerGlobalScope>(context);

#if ENABLE(SQL_DATABASE)
            // FIXME: Should we stop the databases as part of stopActiveDOMObjects() below?
            DatabaseTaskSynchronizer cleanupSync;
            DatabaseManager::manager().stopDatabases(&workerGlobalScope, &cleanupSync);
#endif

            workerGlobalScope.stopActiveDOMObjects();

            workerGlobalScope.notifyObserversOfStop();

            // Event listeners would keep DOMWrapperWorld objects alive for too long. Also, they have references to JS objects,
            // which become dangling once Heap is destroyed.
            workerGlobalScope.removeAllEventListeners();

#if ENABLE(SQL_DATABASE)
            // We wait for the database thread to clean up all its stuff so that we
            // can do more stringent leak checks as we exit.
            cleanupSync.waitForTaskCompletion();
#endif

            // Stick a shutdown command at the end of the queue, so that we deal
            // with all the cleanup tasks the databases post first.
            workerGlobalScope.postTask({ ScriptExecutionContext::Task::CleanupTask, [] (ScriptExecutionContext& context) {
                WorkerGlobalScope& workerGlobalScope = downcast<WorkerGlobalScope>(context);
                // It's not safe to call clearScript until all the cleanup tasks posted by functions initiated by WorkerThreadShutdownStartTask have completed.
                workerGlobalScope.clearScript();
            } });

        } });
        return;
    }
    m_runLoop.terminate();
}

void WorkerThread::releaseFastMallocFreeMemoryInAllThreads()
{
    std::lock_guard<std::mutex> lock(threadSetMutex());

    for (auto* workerThread : workerThreads())
        workerThread->runLoop().postTask([] (ScriptExecutionContext&) {
            WTF::releaseFastMallocFreeMemory();
        });
}

} // namespace WebCore
