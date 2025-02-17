/*
 * Copyright (C) 2010, 2014 Apple Inc. All rights reserved.
 * Portions Copyright (c) 2011 Motorola Mobility, Inc.  All rights reserved.
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

#include "config.h"
#include "WebInspectorProxy.h"

#if ENABLE(INSPECTOR)

#include "APIURLRequest.h"
#include "WebFramePolicyListenerProxy.h"
#include "WebFrameProxy.h"
#include "WebInspectorMessages.h"
#include "WebInspectorProxyMessages.h"
#include "WebInspectorUIMessages.h"
#include "WebPageGroup.h"
#include "WebPageProxy.h"
#include "WebPreferences.h"
#include "WebProcessPool.h"
#include "WebProcessProxy.h"
#include <WebCore/SchemeRegistry.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/StringBuilder.h>

#if ENABLE(INSPECTOR_SERVER)
#include "WebInspectorServer.h"
#endif

using namespace WebCore;

namespace WebKit {

const unsigned WebInspectorProxy::minimumWindowWidth = 750;
const unsigned WebInspectorProxy::minimumWindowHeight = 400;

const unsigned WebInspectorProxy::initialWindowWidth = 1000;
const unsigned WebInspectorProxy::initialWindowHeight = 650;

class WebInspectorPageGroups {
public:
    static WebInspectorPageGroups& shared()
    {
        static NeverDestroyed<WebInspectorPageGroups> instance;
        return instance;
    }

    unsigned inspectorLevel(WebPageGroup& inspectedPageGroup)
    {
        return isInspectorPageGroup(inspectedPageGroup) ? inspectorPageGroupLevel(inspectedPageGroup) + 1 : 1;
    }

    bool isInspectorPageGroup(WebPageGroup& group)
    {
        return m_pageGroupLevel.contains(&group);
    }

    unsigned inspectorPageGroupLevel(WebPageGroup& group)
    {
        ASSERT(isInspectorPageGroup(group));
        return m_pageGroupLevel.get(&group);
    }

    WebPageGroup* inspectorPageGroupForLevel(unsigned level)
    {
        // The level is the key of the HashMap, so it cannot be 0.
        ASSERT(level);

        auto iterator = m_pageGroupByLevel.find(level);
        if (iterator != m_pageGroupByLevel.end())
            return iterator->value.get();

        RefPtr<WebPageGroup> group = createInspectorPageGroup(level);
        m_pageGroupByLevel.set(level, group.get());
        m_pageGroupLevel.set(group.get(), level);
        return group.get();
    }

private:
    static PassRefPtr<WebPageGroup> createInspectorPageGroup(unsigned level)
    {
        RefPtr<WebPageGroup> pageGroup = WebPageGroup::create(String::format("__WebInspectorPageGroupLevel%u__", level), false, false);

#ifndef NDEBUG
        // Allow developers to inspect the Web Inspector in debug builds without changing settings.
        pageGroup->preferences().setDeveloperExtrasEnabled(true);
        pageGroup->preferences().setLogsPageMessagesToSystemConsoleEnabled(true);
#endif

        pageGroup->preferences().setAllowFileAccessFromFileURLs(true);
        pageGroup->preferences().setApplicationChromeModeEnabled(true);

        return pageGroup.release();
    }

    typedef HashMap<unsigned, RefPtr<WebPageGroup> > PageGroupByLevelMap;
    typedef HashMap<WebPageGroup*, unsigned> PageGroupLevelMap;

    PageGroupByLevelMap m_pageGroupByLevel;
    PageGroupLevelMap m_pageGroupLevel;
};

WebInspectorProxy::WebInspectorProxy(WebPageProxy* page)
    : m_page(page)
    , m_inspectorPage(nullptr)
    , m_underTest(false)
    , m_isVisible(false)
    , m_isAttached(false)
    , m_canAttach(false)
    , m_isProfilingPage(false)
    , m_showMessageSent(false)
    , m_ignoreFirstBringToFront(false)
    , m_attachmentSide(AttachmentSideBottom)
#if PLATFORM(MAC)
    , m_closeTimer(RunLoop::main(), this, &WebInspectorProxy::closeTimerFired)
#elif PLATFORM(GTK) || PLATFORM(EFL)
    , m_inspectorView(nullptr)
    , m_inspectorWindow(nullptr)
#if PLATFORM(GTK)
    , m_headerBar(nullptr)
    , m_dockBottomButton(nullptr)
    , m_dockRightButton(nullptr)
#endif
#endif
#if ENABLE(INSPECTOR_SERVER)
    , m_remoteInspectionPageId(0)
#endif
{
    m_level = WebInspectorPageGroups::shared().inspectorLevel(m_page->pageGroup());
    m_page->process().addMessageReceiver(Messages::WebInspectorProxy::messageReceiverName(), m_page->pageID(), *this);
}

WebInspectorProxy::~WebInspectorProxy()
{
}

WebPageGroup* WebInspectorProxy::inspectorPageGroup() const
{
    return WebInspectorPageGroups::shared().inspectorPageGroupForLevel(m_level);
}

void WebInspectorProxy::invalidate()
{
#if ENABLE(INSPECTOR_SERVER)
    if (m_remoteInspectionPageId)
        WebInspectorServer::shared().unregisterPage(m_remoteInspectionPageId);
#endif

    m_page->process().removeMessageReceiver(Messages::WebInspectorProxy::messageReceiverName(), m_page->pageID());

    didClose();
    platformInvalidate();

    m_page = nullptr;
}

// Public APIs
bool WebInspectorProxy::isFront()
{
    if (!m_page)
        return false;

    return platformIsFront();
}

void WebInspectorProxy::connect()
{
    if (!m_page)
        return;

    if (m_showMessageSent)
        return;

    m_showMessageSent = true;
    m_ignoreFirstBringToFront = true;

    eagerlyCreateInspectorPage();

    m_page->process().send(Messages::WebInspector::Show(), m_page->pageID());
}

void WebInspectorProxy::show()
{
    if (!m_page)
        return;

    if (isConnected()) {
        bringToFront();
        return;
    }

    connect();

    // Don't ignore the first bringToFront so it opens the Inspector.
    m_ignoreFirstBringToFront = false;
}

void WebInspectorProxy::hide()
{
    if (!m_page)
        return;

    m_isVisible = false;

    platformHide();
}

void WebInspectorProxy::close()
{
    if (!m_page)
        return;

    m_page->process().send(Messages::WebInspector::Close(), m_page->pageID());

    didClose();
}

void WebInspectorProxy::didRelaunchInspectorPageProcess()
{
    m_inspectorPage->process().addMessageReceiver(Messages::WebInspectorProxy::messageReceiverName(), m_page->pageID(), *this);
    m_inspectorPage->process().assumeReadAccessToBaseURL(inspectorBaseURL());

    // When didRelaunchInspectorPageProcess is called we can assume it is during a load request.
    // Any messages we would have sent to a terminated process need to be re-sent.

    m_inspectorPage->process().send(Messages::WebInspectorUI::EstablishConnection(m_connectionIdentifier, m_page->pageID(), m_underTest), m_inspectorPage->pageID());
}

void WebInspectorProxy::showConsole()
{
    if (!m_page)
        return;

    eagerlyCreateInspectorPage();

    m_page->process().send(Messages::WebInspector::ShowConsole(), m_page->pageID());
}

void WebInspectorProxy::showResources()
{
    if (!m_page)
        return;

    eagerlyCreateInspectorPage();

    m_page->process().send(Messages::WebInspector::ShowResources(), m_page->pageID());
}

void WebInspectorProxy::showMainResourceForFrame(WebFrameProxy* frame)
{
    if (!m_page)
        return;

    eagerlyCreateInspectorPage();

    m_page->process().send(Messages::WebInspector::ShowMainResourceForFrame(frame->frameID()), m_page->pageID());
}

void WebInspectorProxy::attachBottom()
{
    attach(AttachmentSideBottom);
}

void WebInspectorProxy::attachRight()
{
    attach(AttachmentSideRight);
}

void WebInspectorProxy::attach(AttachmentSide side)
{
    if (!m_page || !canAttach())
        return;

    m_isAttached = true;
    m_attachmentSide = side;

    inspectorPageGroup()->preferences().setInspectorAttachmentSide(side);

    if (m_isVisible)
        inspectorPageGroup()->preferences().setInspectorStartsAttached(true);

    m_page->process().send(Messages::WebInspector::SetAttached(true), m_page->pageID());

    switch (m_attachmentSide) {
    case AttachmentSideBottom:
        m_inspectorPage->process().send(Messages::WebInspectorUI::AttachedBottom(), m_inspectorPage->pageID());
        break;

    case AttachmentSideRight:
        m_inspectorPage->process().send(Messages::WebInspectorUI::AttachedRight(), m_inspectorPage->pageID());
        break;
    }

    platformAttach();
}

void WebInspectorProxy::detach()
{
    if (!m_page)
        return;

    m_isAttached = false;

    if (m_isVisible)
        inspectorPageGroup()->preferences().setInspectorStartsAttached(false);

    m_page->process().send(Messages::WebInspector::SetAttached(false), m_page->pageID());
    m_inspectorPage->process().send(Messages::WebInspectorUI::Detached(), m_inspectorPage->pageID());

    platformDetach();
}

void WebInspectorProxy::setAttachedWindowHeight(unsigned height)
{
    inspectorPageGroup()->preferences().setInspectorAttachedHeight(height);
    platformSetAttachedWindowHeight(height);
}

void WebInspectorProxy::setAttachedWindowWidth(unsigned width)
{
    inspectorPageGroup()->preferences().setInspectorAttachedWidth(width);
    platformSetAttachedWindowWidth(width);
}

void WebInspectorProxy::togglePageProfiling()
{
    if (!m_page)
        return;

    if (m_isProfilingPage)
        m_page->process().send(Messages::WebInspector::StopPageProfiling(), m_page->pageID());
    else
        m_page->process().send(Messages::WebInspector::StartPageProfiling(), m_page->pageID());

    // FIXME: have the WebProcess notify us on state changes.
    m_isProfilingPage = !m_isProfilingPage;
}

bool WebInspectorProxy::isInspectorPage(WebPageProxy& page)
{
    return WebInspectorPageGroups::shared().isInspectorPageGroup(page.pageGroup());
}

WebProcessPool& WebInspectorProxy::inspectorProcessPool()
{
    // Having our own process pool removes us from the main process pool and
    // guarantees no process sharing for our user interface.

    static WebProcessPool* processPool;
    if (!processPool) {
        WebProcessPoolConfiguration configuration;
        WebProcessPool::applyPlatformSpecificConfigurationDefaults(configuration);
        
        processPool = (WebProcessPool::create(WTF::move(configuration))).leakRef();
        processPool->setProcessModel(ProcessModelMultipleSecondaryProcesses);
    }

    return *processPool;
}

static bool isMainOrTestInspectorPage(const WebInspectorProxy* webInspectorProxy, WKURLRequestRef requestRef)
{
    URL requestURL(URL(), toImpl(requestRef)->resourceRequest().url());
    if (!WebCore::SchemeRegistry::shouldTreatURLSchemeAsLocal(requestURL.protocol()))
        return false;

    // Use URL so we can compare just the paths.
    URL mainPageURL(URL(), webInspectorProxy->inspectorPageURL());
    ASSERT(WebCore::SchemeRegistry::shouldTreatURLSchemeAsLocal(mainPageURL.protocol()));
    if (decodeURLEscapeSequences(requestURL.path()) == decodeURLEscapeSequences(mainPageURL.path()))
        return true;

    // We might not have a Test URL in Production builds.
    String testPageURLString = webInspectorProxy->inspectorTestPageURL();
    if (testPageURLString.isNull())
        return false;

    URL testPageURL(URL(), webInspectorProxy->inspectorTestPageURL());
    ASSERT(WebCore::SchemeRegistry::shouldTreatURLSchemeAsLocal(testPageURL.protocol()));
    return decodeURLEscapeSequences(requestURL.path()) == decodeURLEscapeSequences(testPageURL.path());
}

static void processDidCrash(WKPageRef, const void* clientInfo)
{
    WebInspectorProxy* webInspectorProxy = static_cast<WebInspectorProxy*>(const_cast<void*>(clientInfo));
    ASSERT(webInspectorProxy);
    webInspectorProxy->close();
}

static void decidePolicyForNavigationAction(WKPageRef, WKFrameRef frameRef, WKFrameNavigationType, WKEventModifiers, WKEventMouseButton, WKFrameRef, WKURLRequestRef requestRef, WKFramePolicyListenerRef listenerRef, WKTypeRef, const void* clientInfo)
{
    // Allow non-main frames to navigate anywhere.
    if (!toImpl(frameRef)->isMainFrame()) {
        toImpl(listenerRef)->use();
        return;
    }

    const WebInspectorProxy* webInspectorProxy = static_cast<const WebInspectorProxy*>(clientInfo);
    ASSERT(webInspectorProxy);

    // Allow loading of the main inspector file.
    if (isMainOrTestInspectorPage(webInspectorProxy, requestRef)) {
        toImpl(listenerRef)->use();
        return;
    }

    // Prevent everything else from loading in the inspector's page.
    toImpl(listenerRef)->ignore();

    // And instead load it in the inspected page.
    webInspectorProxy->page()->loadRequest(toImpl(requestRef)->resourceRequest());
}

#if ENABLE(INSPECTOR_SERVER)
void WebInspectorProxy::enableRemoteInspection()
{
    if (!m_remoteInspectionPageId)
        m_remoteInspectionPageId = WebInspectorServer::shared().registerPage(this);
}

void WebInspectorProxy::remoteFrontendConnected()
{
    m_page->process().send(Messages::WebInspector::RemoteFrontendConnected(), m_page->pageID());
}

void WebInspectorProxy::remoteFrontendDisconnected()
{
    m_page->process().send(Messages::WebInspector::RemoteFrontendDisconnected(), m_page->pageID());
}

void WebInspectorProxy::dispatchMessageFromRemoteFrontend(const String& message)
{
    m_page->process().send(Messages::WebInspector::SendMessageToBackend(message), m_page->pageID());
}
#endif

void WebInspectorProxy::eagerlyCreateInspectorPage()
{
    if (m_inspectorPage)
        return;

    m_inspectorPage = platformCreateInspectorPage();
    ASSERT(m_inspectorPage);
    if (!m_inspectorPage)
        return;

    WKPagePolicyClientV1 policyClient = {
        { 1, this },
        nullptr, // decidePolicyForNavigationAction_deprecatedForUseWithV0
        nullptr, // decidePolicyForNewWindowAction
        nullptr, // decidePolicyForResponse_deprecatedForUseWithV0
        nullptr, // unableToImplementPolicy
        decidePolicyForNavigationAction,
        nullptr, // decidePolicyForResponse
    };

    WKPageLoaderClientV5 loaderClient = {
        { 5, this },
        nullptr, // didStartProvisionalLoadForFrame
        nullptr, // didReceiveServerRedirectForProvisionalLoadForFrame
        nullptr, // didFailProvisionalLoadWithErrorForFrame
        nullptr, // didCommitLoadForFrame
        nullptr, // didFinishDocumentLoadForFrame
        nullptr, // didFinishLoadForFrame
        nullptr, // didFailLoadWithErrorForFrame
        nullptr, // didSameDocumentNavigationForFrame
        nullptr, // didReceiveTitleForFrame
        nullptr, // didFirstLayoutForFrame
        nullptr, // didFirstVisuallyNonEmptyLayoutForFrame
        nullptr, // didRemoveFrameFromHierarchy
        nullptr, // didDisplayInsecureContentForFrame
        nullptr, // didRunInsecureContentForFrame
        nullptr, // canAuthenticateAgainstProtectionSpaceInFrame
        nullptr, // didReceiveAuthenticationChallengeInFrame
        nullptr, // didStartProgress
        nullptr, // didChangeProgress
        nullptr, // didFinishProgress
        nullptr, // didBecomeUnresponsive
        nullptr, // didBecomeResponsive
        processDidCrash,
        nullptr, // didChangeBackForwardList
        nullptr, // shouldGoToBackForwardListItem
        nullptr, // didFailToInitializePlugin_deprecatedForUseWithV0
        nullptr, // didDetectXSSForFrame
        nullptr, // didNewFirstVisuallyNonEmptyLayout_unavailable
        nullptr, // willGoToBackForwardListItem
        nullptr, // interactionOccurredWhileProcessUnresponsive
        nullptr, // pluginDidFail_deprecatedForUseWithV1
        nullptr, // didReceiveIntentForFrame_unavailable
        nullptr, // registerIntentServiceForFrame_unavailable
        nullptr, // didLayout
        nullptr, // pluginLoadPolicy_deprecatedForUseWithV2
        nullptr, // pluginDidFail
        nullptr, // pluginLoadPolicy
        nullptr, // webGLLoadPolicy
        nullptr, // resolveWebGLLoadPolicy
        nullptr, // shouldKeepCurrentBackForwardListItemInList
    };

    WKPageSetPagePolicyClient(toAPI(m_inspectorPage), &policyClient.base);
    WKPageSetPageLoaderClient(toAPI(m_inspectorPage), &loaderClient.base);

    m_inspectorPage->process().addMessageReceiver(Messages::WebInspectorProxy::messageReceiverName(), m_page->pageID(), *this);
    m_inspectorPage->process().assumeReadAccessToBaseURL(inspectorBaseURL());
}

// Called by WebInspectorProxy messages
void WebInspectorProxy::createInspectorPage(IPC::Attachment connectionIdentifier, bool canAttach, bool underTest)
{
    if (!m_page)
        return;

    eagerlyCreateInspectorPage();

    ASSERT(m_inspectorPage);
    if (!m_inspectorPage)
        return;

    m_underTest = underTest;
    m_connectionIdentifier = connectionIdentifier;

    m_inspectorPage->process().send(Messages::WebInspectorUI::EstablishConnection(m_connectionIdentifier, m_page->pageID(), m_underTest), m_inspectorPage->pageID());

    if (!m_underTest) {
        m_canAttach = canAttach;
        m_isAttached = shouldOpenAttached();
        m_attachmentSide = static_cast<AttachmentSide>(inspectorPageGroup()->preferences().inspectorAttachmentSide());

        m_page->process().send(Messages::WebInspector::SetAttached(m_isAttached), m_page->pageID());

        if (m_isAttached) {
            switch (m_attachmentSide) {
            case AttachmentSideBottom:
                m_inspectorPage->process().send(Messages::WebInspectorUI::AttachedBottom(), m_inspectorPage->pageID());
                break;

            case AttachmentSideRight:
                m_inspectorPage->process().send(Messages::WebInspectorUI::AttachedRight(), m_inspectorPage->pageID());
                break;
            }
        } else
            m_inspectorPage->process().send(Messages::WebInspectorUI::Detached(), m_inspectorPage->pageID());
    }

    m_inspectorPage->loadRequest(URL(URL(), m_underTest ? inspectorTestPageURL() : inspectorPageURL()));
}

void WebInspectorProxy::open()
{
    if (m_underTest)
        return;

    m_isVisible = true;

    platformOpen();
}

void WebInspectorProxy::didClose()
{
    if (!m_inspectorPage)
        return;

    m_inspectorPage->process().removeMessageReceiver(Messages::WebInspectorProxy::messageReceiverName(), m_page->pageID());
    m_inspectorPage = nullptr;

    m_isVisible = false;
    m_isProfilingPage = false;
    m_showMessageSent = false;
    m_ignoreFirstBringToFront = false;

    if (m_isAttached)
        platformDetach();
    m_isAttached = false;
    m_canAttach = false;
    m_underTest = false;
    m_connectionIdentifier = IPC::Attachment();

    platformDidClose();
}

void WebInspectorProxy::bringToFront()
{
    // WebCore::InspectorFrontendClientLocal tells us to do this on load. We want to
    // ignore it once if we only wanted to connect. This allows the Inspector to later
    // request to be brought to the front when a breakpoint is hit or some other action.
    if (m_ignoreFirstBringToFront) {
        m_ignoreFirstBringToFront = false;
        return;
    }

    if (m_isVisible)
        platformBringToFront();
    else
        open();
}

void WebInspectorProxy::attachAvailabilityChanged(bool available)
{
    m_canAttach = available;

    platformAttachAvailabilityChanged(available);
}

void WebInspectorProxy::inspectedURLChanged(const String& urlString)
{
    platformInspectedURLChanged(urlString);
}

void WebInspectorProxy::save(const String& filename, const String& content, bool base64Encoded, bool forceSaveAs)
{
    platformSave(filename, content, base64Encoded, forceSaveAs);
}

void WebInspectorProxy::append(const String& filename, const String& content)
{
    platformAppend(filename, content);
}

bool WebInspectorProxy::shouldOpenAttached()
{
    return inspectorPageGroup()->preferences().inspectorStartsAttached() && canAttach();
}

#if ENABLE(INSPECTOR_SERVER)
void WebInspectorProxy::sendMessageToRemoteFrontend(const String& message)
{
    ASSERT(m_remoteInspectionPageId);
    WebInspectorServer::shared().sendMessageOverConnection(m_remoteInspectionPageId, message);
}
#endif

} // namespace WebKit

#endif // ENABLE(INSPECTOR)
