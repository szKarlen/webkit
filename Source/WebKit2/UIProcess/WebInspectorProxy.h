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

#ifndef WebInspectorProxy_h
#define WebInspectorProxy_h

#if ENABLE(INSPECTOR)

#include "APIObject.h"
#include "Attachment.h"
#include "MessageReceiver.h"
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

#if PLATFORM(MAC)
#include "WKGeometry.h"
#include <wtf/HashMap.h>
#include <wtf/RetainPtr.h>
#include <wtf/RunLoop.h>

OBJC_CLASS NSButton;
OBJC_CLASS NSURL;
OBJC_CLASS NSWindow;
OBJC_CLASS WKWebInspectorProxyObjCAdapter;
OBJC_CLASS WKWebInspectorWKView;
#endif

#if PLATFORM(GTK)
#include "WebInspectorClientGtk.h"
#endif

#if PLATFORM(EFL)
#include <Ecore_Evas.h>
#include <Evas.h>
#endif

namespace WebKit {

class WebFrameProxy;
class WebPageGroup;
class WebPageProxy;
class WebProcessPool;

enum AttachmentSide {
    AttachmentSideBottom,
    AttachmentSideRight
};

class WebInspectorProxy : public API::ObjectImpl<API::Object::Type::Inspector>, public IPC::MessageReceiver {
public:
    static PassRefPtr<WebInspectorProxy> create(WebPageProxy* page)
    {
        return adoptRef(new WebInspectorProxy(page));
    }

    ~WebInspectorProxy();

    void invalidate();

    // Public APIs
    WebPageProxy* page() const { return m_page; }

    bool isConnected() const { return !!m_inspectorPage; }
    bool isVisible() const { return m_isVisible; }
    bool isFront();

    void connect();

    void show();
    void hide();
    void close();

    void didRelaunchInspectorPageProcess();
    
#if PLATFORM(MAC)
    void createInspectorWindow();
    void updateInspectorWindowTitle() const;
    void inspectedViewFrameDidChange(CGFloat = 0);
    void windowFrameDidChange();
    NSWindow* inspectorWindow() const { return m_inspectorWindow.get(); }

    void setInspectorWindowFrame(WKRect&);
    WKRect inspectorWindowFrame();

    void closeTimerFired();
#endif

#if PLATFORM(GTK)
    GtkWidget* inspectorView() const { return m_inspectorView; };
    void initializeInspectorClientGtk(const WKInspectorClientGtkBase*);
#endif

    void showConsole();
    void showResources();
    void showMainResourceForFrame(WebFrameProxy*);

    bool isAttached() const { return m_isAttached; }
    void attachRight();
    void attachBottom();
    void attach(AttachmentSide = AttachmentSideBottom);
    void detach();

    void setAttachedWindowHeight(unsigned);
    void setAttachedWindowWidth(unsigned);
    void setToolbarHeight(unsigned height) { platformSetToolbarHeight(height); }

    bool isProfilingPage() const { return m_isProfilingPage; }
    void togglePageProfiling();

    static bool isInspectorPage(WebPageProxy&);
    static WebProcessPool& inspectorProcessPool();

    // Provided by platform WebInspectorProxy implementations.
    String inspectorPageURL() const;
    String inspectorTestPageURL() const;
    String inspectorBaseURL() const;

#if ENABLE(INSPECTOR_SERVER)
    void enableRemoteInspection();
    void remoteFrontendConnected();
    void remoteFrontendDisconnected();
    void dispatchMessageFromRemoteFrontend(const String& message);
    int remoteInspectionPageID() const { return m_remoteInspectionPageId; }
#endif

private:
    explicit WebInspectorProxy(WebPageProxy*);

    void eagerlyCreateInspectorPage();

    // IPC::MessageReceiver
    virtual void didReceiveMessage(IPC::Connection*, IPC::MessageDecoder&) override;

    WebPageProxy* platformCreateInspectorPage();
    void platformOpen();
    void platformDidClose();
    void platformInvalidate();
    void platformBringToFront();
    void platformHide();
    bool platformIsFront();
    void platformAttachAvailabilityChanged(bool);
    void platformInspectedURLChanged(const String&);
    unsigned platformInspectedWindowHeight();
    unsigned platformInspectedWindowWidth();
    void platformAttach();
    void platformDetach();
    void platformSetAttachedWindowHeight(unsigned);
    void platformSetAttachedWindowWidth(unsigned);
    void platformSetToolbarHeight(unsigned);
    void platformSave(const String& filename, const String& content, bool base64Encoded, bool forceSaveAs);
    void platformAppend(const String& filename, const String& content);

    // Called by WebInspectorProxy messages
    void createInspectorPage(IPC::Attachment, bool canAttach, bool underTest);
    void didClose();
    void bringToFront();
    void attachAvailabilityChanged(bool);
    void inspectedURLChanged(const String&);

    void save(const String& filename, const String& content, bool base64Encoded, bool forceSaveAs);
    void append(const String& filename, const String& content);

#if ENABLE(INSPECTOR_SERVER)
    void sendMessageToRemoteFrontend(const String& message);
#endif

    bool canAttach() const { return m_canAttach; }
    bool shouldOpenAttached();

    void open();

    WebPageGroup* inspectorPageGroup() const;

#if PLATFORM(GTK) || PLATFORM(EFL)
    void createInspectorWindow();
#endif

#if PLATFORM(GTK)
    void updateInspectorWindowTitle() const;
    static void dockButtonClicked(GtkWidget*, WebInspectorProxy*);
#endif

    static const unsigned minimumWindowWidth;
    static const unsigned minimumWindowHeight;

    static const unsigned initialWindowWidth;
    static const unsigned initialWindowHeight;

    WebPageProxy* m_page;
    WebPageProxy* m_inspectorPage;

    bool m_underTest;
    bool m_isVisible;
    bool m_isAttached;
    bool m_canAttach;
    bool m_isProfilingPage;
    bool m_showMessageSent;
    bool m_ignoreFirstBringToFront;

    // The debugger stops all the pages in the same PageGroup. Having
    // all the inspectors in the same group will make it impossible to debug
    // the inspector code, so we use the level to make different page groups.
    unsigned m_level;
    
    IPC::Attachment m_connectionIdentifier;

    AttachmentSide m_attachmentSide;

#if PLATFORM(MAC)
    RetainPtr<WKWebInspectorWKView> m_inspectorView;
    RetainPtr<NSWindow> m_inspectorWindow;
    RetainPtr<NSButton> m_dockBottomButton;
    RetainPtr<NSButton> m_dockRightButton;
    RetainPtr<WKWebInspectorProxyObjCAdapter> m_inspectorProxyObjCAdapter;
    HashMap<String, RetainPtr<NSURL>> m_suggestedToActualURLMap;
    RunLoop::Timer<WebInspectorProxy> m_closeTimer;
    String m_urlString;
#elif PLATFORM(GTK)
    WebInspectorClientGtk m_client;
    GtkWidget* m_inspectorView;
    GtkWidget* m_inspectorWindow;
    GtkWidget* m_headerBar;
    GtkWidget* m_dockBottomButton;
    GtkWidget* m_dockRightButton;
    String m_inspectedURLString;
#elif PLATFORM(EFL)
    Evas_Object* m_inspectorView;
    Ecore_Evas* m_inspectorWindow;
#endif
#if ENABLE(INSPECTOR_SERVER)
    int m_remoteInspectionPageId;
#endif
};

} // namespace WebKit

#endif // ENABLE(INSPECTOR)

#endif // WebInspectorProxy_h
