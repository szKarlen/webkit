/*
 * Copyright (C) 2010, 2011, 2012 Apple Inc. All rights reserved.
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
#include "WebPlatformStrategies.h"

#include "BlockingResponseMap.h"
#include "DataReference.h"
#include "NetworkResourceLoadParameters.h"
#include "PluginInfoStore.h"
#include "SessionTracker.h"
#include "WebCookieManager.h"
#include "WebCoreArgumentCoders.h"
#include "WebErrors.h"
#include "WebFrame.h"
#include "WebFrameLoaderClient.h"
#include "WebFrameNetworkingContext.h"
#include "WebIDBFactoryBackend.h"
#include "WebPage.h"
#include "WebPasteboardOverrides.h"
#include "WebProcess.h"
#include "WebProcessPoolMessages.h"
#include "WebProcessProxyMessages.h"
#include <WebCore/Color.h>
#include <WebCore/IDBFactoryBackendInterface.h>
#include <WebCore/LoaderStrategy.h>
#include <WebCore/MainFrame.h>
#include <WebCore/NetworkStorageSession.h>
#include <WebCore/NetworkingContext.h>
#include <WebCore/Page.h>
#include <WebCore/PageGroup.h>
#include <WebCore/PlatformCookieJar.h>
#include <WebCore/PlatformPasteboard.h>
#include <WebCore/ResourceError.h>
#include <WebCore/SessionID.h>
#include <WebCore/StorageNamespace.h>
#include <WebCore/SubframeLoader.h>
#include <WebCore/URL.h>
#include <wtf/Atomics.h>

#if ENABLE(NETWORK_PROCESS)
#include "BlobRegistryProxy.h"
#include "NetworkConnectionToWebProcessMessages.h"
#include "NetworkProcessConnection.h"
#include "WebResourceLoadScheduler.h"
#endif

using namespace WebCore;

namespace WebKit {

void WebPlatformStrategies::initialize()
{
    static NeverDestroyed<WebPlatformStrategies> platformStrategies;
    setPlatformStrategies(&platformStrategies.get());
}

WebPlatformStrategies::WebPlatformStrategies()
#if ENABLE(NETSCAPE_PLUGIN_API)
    : m_pluginCacheIsPopulated(false)
    , m_shouldRefreshPlugins(false)
#endif // ENABLE(NETSCAPE_PLUGIN_API)
{
}

CookiesStrategy* WebPlatformStrategies::createCookiesStrategy()
{
    return this;
}

DatabaseStrategy* WebPlatformStrategies::createDatabaseStrategy()
{
    return this;
}

LoaderStrategy* WebPlatformStrategies::createLoaderStrategy()
{
    return this;
}

PasteboardStrategy* WebPlatformStrategies::createPasteboardStrategy()
{
    return this;
}

PluginStrategy* WebPlatformStrategies::createPluginStrategy()
{
    return this;
}

SharedWorkerStrategy* WebPlatformStrategies::createSharedWorkerStrategy()
{
    return this;
}

// CookiesStrategy

String WebPlatformStrategies::cookiesForDOM(const NetworkStorageSession& session, const URL& firstParty, const URL& url)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        String result;
        if (!WebProcess::shared().networkConnection()->connection()->sendSync(Messages::NetworkConnectionToWebProcess::CookiesForDOM(SessionTracker::sessionID(session), firstParty, url), Messages::NetworkConnectionToWebProcess::CookiesForDOM::Reply(result), 0))
            return String();
        return result;
    }
#endif

    return WebCore::cookiesForDOM(session, firstParty, url);
}

void WebPlatformStrategies::setCookiesFromDOM(const NetworkStorageSession& session, const URL& firstParty, const URL& url, const String& cookieString)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        WebProcess::shared().networkConnection()->connection()->send(Messages::NetworkConnectionToWebProcess::SetCookiesFromDOM(SessionTracker::sessionID(session), firstParty, url, cookieString), 0);
        return;
    }
#endif

    WebCore::setCookiesFromDOM(session, firstParty, url, cookieString);
}

bool WebPlatformStrategies::cookiesEnabled(const NetworkStorageSession& session, const URL& firstParty, const URL& url)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        bool result;
        if (!WebProcess::shared().networkConnection()->connection()->sendSync(Messages::NetworkConnectionToWebProcess::CookiesEnabled(SessionTracker::sessionID(session), firstParty, url), Messages::NetworkConnectionToWebProcess::CookiesEnabled::Reply(result), 0))
            return false;
        return result;
    }
#endif

    return WebCore::cookiesEnabled(session, firstParty, url);
}

String WebPlatformStrategies::cookieRequestHeaderFieldValue(const NetworkStorageSession& session, const URL& firstParty, const URL& url)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        String result;
        if (!WebProcess::shared().networkConnection()->connection()->sendSync(Messages::NetworkConnectionToWebProcess::CookieRequestHeaderFieldValue(SessionTracker::sessionID(session), firstParty, url), Messages::NetworkConnectionToWebProcess::CookieRequestHeaderFieldValue::Reply(result), 0))
            return String();
        return result;
    }
#endif

    return WebCore::cookieRequestHeaderFieldValue(session, firstParty, url);
}

bool WebPlatformStrategies::getRawCookies(const NetworkStorageSession& session, const URL& firstParty, const URL& url, Vector<Cookie>& rawCookies)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        if (!WebProcess::shared().networkConnection()->connection()->sendSync(Messages::NetworkConnectionToWebProcess::GetRawCookies(SessionTracker::sessionID(session), firstParty, url), Messages::NetworkConnectionToWebProcess::GetRawCookies::Reply(rawCookies), 0))
            return false;
        return true;
    }
#endif

    return WebCore::getRawCookies(session, firstParty, url, rawCookies);
}

void WebPlatformStrategies::deleteCookie(const NetworkStorageSession& session, const URL& url, const String& cookieName)
{
#if ENABLE(NETWORK_PROCESS)
    if (WebProcess::shared().usesNetworkProcess()) {
        WebProcess::shared().networkConnection()->connection()->send(Messages::NetworkConnectionToWebProcess::DeleteCookie(SessionTracker::sessionID(session), url, cookieName), 0);
        return;
    }
#endif

    WebCore::deleteCookie(session, url, cookieName);
}

// DatabaseStrategy

#if ENABLE(SQL_DATABASE)
AbstractDatabaseServer* WebPlatformStrategies::getDatabaseServer()
{
    return DatabaseStrategy::getDatabaseServer();
}
#endif // ENABLE(SQL_DATABASE)

#if ENABLE(INDEXED_DATABASE)
PassRefPtr<IDBFactoryBackendInterface> WebPlatformStrategies::createIDBFactoryBackend(const String& databaseDirectoryIdentifier)
{
#if !ENABLE(DATABASE_PROCESS)
    return DatabaseStrategy::createIDBFactoryBackend(databaseDirectoryIdentifier);
#endif

    return WebIDBFactoryBackend::create(databaseDirectoryIdentifier);
}
#endif // ENABLE(INDEXED_DATABASE)

// LoaderStrategy

#if ENABLE(NETWORK_PROCESS)
ResourceLoadScheduler* WebPlatformStrategies::resourceLoadScheduler()
{
    static ResourceLoadScheduler* scheduler;
    if (!scheduler) {
        if (WebProcess::shared().usesNetworkProcess())
            scheduler = &WebProcess::shared().webResourceLoadScheduler();
        else
            scheduler = WebCore::resourceLoadScheduler();
    }
    
    return scheduler;
}

void WebPlatformStrategies::loadResourceSynchronously(NetworkingContext* context, unsigned long resourceLoadIdentifier, const ResourceRequest& request, StoredCredentials storedCredentials, ClientCredentialPolicy clientCredentialPolicy, ResourceError& error, ResourceResponse& response, Vector<char>& data)
{
    if (!WebProcess::shared().usesNetworkProcess()) {
        LoaderStrategy::loadResourceSynchronously(context, resourceLoadIdentifier, request, storedCredentials, clientCredentialPolicy, error, response, data);
        return;
    }

    WebFrameNetworkingContext* webContext = static_cast<WebFrameNetworkingContext*>(context);
    // FIXME: Some entities in WebCore use WebCore's "EmptyFrameLoaderClient" instead of having a proper WebFrameLoaderClient.
    // EmptyFrameLoaderClient shouldn't exist and everything should be using a WebFrameLoaderClient,
    // but in the meantime we have to make sure not to mis-cast.
    WebFrameLoaderClient* webFrameLoaderClient = webContext->webFrameLoaderClient();
    WebFrame* webFrame = webFrameLoaderClient ? webFrameLoaderClient->webFrame() : 0;
    WebPage* webPage = webFrame ? webFrame->page() : 0;

    NetworkResourceLoadParameters loadParameters;
    loadParameters.identifier = resourceLoadIdentifier;
    loadParameters.webPageID = webPage ? webPage->pageID() : 0;
    loadParameters.webFrameID = webFrame ? webFrame->frameID() : 0;
    loadParameters.sessionID = webPage ? webPage->sessionID() : SessionID::defaultSessionID();
    loadParameters.request = request;
    loadParameters.contentSniffingPolicy = SniffContent;
    loadParameters.allowStoredCredentials = storedCredentials;
    loadParameters.clientCredentialPolicy = clientCredentialPolicy;
    loadParameters.shouldClearReferrerOnHTTPSToHTTPRedirect = context->shouldClearReferrerOnHTTPSToHTTPRedirect();

    data.resize(0);

    if (!WebProcess::shared().networkConnection()->connection()->sendSync(Messages::NetworkConnectionToWebProcess::PerformSynchronousLoad(loadParameters), Messages::NetworkConnectionToWebProcess::PerformSynchronousLoad::Reply(error, response, data), 0)) {
        response = ResourceResponse();
        error = internalError(request.url());
    }
}

BlobRegistry* WebPlatformStrategies::createBlobRegistry()
{
    if (!WebProcess::shared().usesNetworkProcess())
        return LoaderStrategy::createBlobRegistry();
    return new BlobRegistryProxy;    
}
#endif

// PluginStrategy

void WebPlatformStrategies::refreshPlugins()
{
#if ENABLE(NETSCAPE_PLUGIN_API)
    m_cachedPlugins.clear();
    m_pluginCacheIsPopulated = false;
    m_shouldRefreshPlugins = true;
#endif // ENABLE(NETSCAPE_PLUGIN_API)
}

void WebPlatformStrategies::getPluginInfo(const WebCore::Page* page, Vector<WebCore::PluginInfo>& plugins)
{
#if ENABLE(NETSCAPE_PLUGIN_API)
    populatePluginCache();

    if (page->mainFrame().loader().subframeLoader().allowPlugins(NotAboutToInstantiatePlugin)) {
        plugins = m_cachedPlugins;
        return;
    }

    plugins = m_cachedApplicationPlugins;
#else
    UNUSED_PARAM(page);
    UNUSED_PARAM(plugins);
#endif // ENABLE(NETSCAPE_PLUGIN_API)
}

// SharedWorkerStrategy

bool WebPlatformStrategies::isAvailable() const
{
    // Shared workers do not work across multiple processes, and using network process is tied to multiple secondary process model. 
    return !WebProcess::shared().usesNetworkProcess();
}

#if ENABLE(NETSCAPE_PLUGIN_API)
void WebPlatformStrategies::populatePluginCache()
{
    if (m_pluginCacheIsPopulated)
        return;

    ASSERT(m_cachedPlugins.isEmpty());
    
    // FIXME: Should we do something in case of error here?
    if (!WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessProxy::GetPlugins(m_shouldRefreshPlugins), Messages::WebProcessProxy::GetPlugins::Reply(m_cachedPlugins, m_cachedApplicationPlugins), 0))
        return;

    m_shouldRefreshPlugins = false;
    m_pluginCacheIsPopulated = true;
}
#endif // ENABLE(NETSCAPE_PLUGIN_API)

#if PLATFORM(COCOA)
// PasteboardStrategy

void WebPlatformStrategies::getTypes(Vector<String>& types, const String& pasteboardName)
{
    // First check the overrides.
    // The purpose of the overrides is to avoid messaging back to the UI process.
    // Therefore, if there are any overridden types, we return just those.
    types = WebPasteboardOverrides::sharedPasteboardOverrides().overriddenTypes(pasteboardName);
    if (!types.isEmpty())
        return;

    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardTypes(pasteboardName), Messages::WebProcessPool::GetPasteboardTypes::Reply(types), 0);
}

PassRefPtr<WebCore::SharedBuffer> WebPlatformStrategies::bufferForType(const String& pasteboardType, const String& pasteboardName)
{
    // First check the overrides.
    Vector<char> overrideBuffer;
    if (WebPasteboardOverrides::sharedPasteboardOverrides().getDataForOverride(pasteboardName, pasteboardType, overrideBuffer))
        return SharedBuffer::adoptVector(overrideBuffer);

    // Fallback to messaging the UI process for native pasteboard content.
    SharedMemory::Handle handle;
    uint64_t size = 0;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardBufferForType(pasteboardName, pasteboardType), Messages::WebProcessPool::GetPasteboardBufferForType::Reply(handle, size), 0);
    if (handle.isNull())
        return 0;
    RefPtr<SharedMemory> sharedMemoryBuffer = SharedMemory::create(handle, SharedMemory::ReadOnly);
    return SharedBuffer::create(static_cast<unsigned char *>(sharedMemoryBuffer->data()), size);
}

void WebPlatformStrategies::getPathnamesForType(Vector<String>& pathnames, const String& pasteboardType, const String& pasteboardName)
{
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardPathnamesForType(pasteboardName, pasteboardType), Messages::WebProcessPool::GetPasteboardPathnamesForType::Reply(pathnames), 0);
}

String WebPlatformStrategies::stringForType(const String& pasteboardType, const String& pasteboardName)
{
    String value;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardStringForType(pasteboardName, pasteboardType), Messages::WebProcessPool::GetPasteboardStringForType::Reply(value), 0);
    return value;
}

long WebPlatformStrategies::copy(const String& fromPasteboard, const String& toPasteboard)
{
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::PasteboardCopy(fromPasteboard, toPasteboard), Messages::WebProcessPool::PasteboardCopy::Reply(newChangeCount), 0);
    return newChangeCount;
}

long WebPlatformStrategies::changeCount(const WTF::String &pasteboardName)
{
    uint64_t changeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardChangeCount(pasteboardName), Messages::WebProcessPool::GetPasteboardChangeCount::Reply(changeCount), 0);
    return changeCount;
}

String WebPlatformStrategies::uniqueName()
{
    String pasteboardName;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardUniqueName(), Messages::WebProcessPool::GetPasteboardUniqueName::Reply(pasteboardName), 0);
    return pasteboardName;
}

Color WebPlatformStrategies::color(const String& pasteboardName)
{
    Color color;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardColor(pasteboardName), Messages::WebProcessPool::GetPasteboardColor::Reply(color), 0);
    return color;
}

URL WebPlatformStrategies::url(const String& pasteboardName)
{
    String urlString;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardURL(pasteboardName), Messages::WebProcessPool::GetPasteboardURL::Reply(urlString), 0);
    return URL(ParsedURLString, urlString);
}

long WebPlatformStrategies::addTypes(const Vector<String>& pasteboardTypes, const String& pasteboardName)
{
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::AddPasteboardTypes(pasteboardName, pasteboardTypes), Messages::WebProcessPool::AddPasteboardTypes::Reply(newChangeCount), 0);
    return newChangeCount;
}

long WebPlatformStrategies::setTypes(const Vector<String>& pasteboardTypes, const String& pasteboardName)
{
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::SetPasteboardTypes(pasteboardName, pasteboardTypes), Messages::WebProcessPool::SetPasteboardTypes::Reply(newChangeCount), 0);
    return newChangeCount;
}

long WebPlatformStrategies::setBufferForType(PassRefPtr<SharedBuffer> buffer, const String& pasteboardType, const String& pasteboardName)
{
    SharedMemory::Handle handle;
    if (buffer) {
        RefPtr<SharedMemory> sharedMemoryBuffer = SharedMemory::create(buffer->size());
        // FIXME: Null check prevents crashing, but it is not great that we will have empty pasteboard content for this type,
        // because we've already set the types.
        if (sharedMemoryBuffer) {
            memcpy(sharedMemoryBuffer->data(), buffer->data(), buffer->size());
            sharedMemoryBuffer->createHandle(handle, SharedMemory::ReadOnly);
        }
    }
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::SetPasteboardBufferForType(pasteboardName, pasteboardType, handle, buffer ? buffer->size() : 0), Messages::WebProcessPool::SetPasteboardBufferForType::Reply(newChangeCount), 0);
    return newChangeCount;
}

long WebPlatformStrategies::setPathnamesForType(const Vector<String>& pathnames, const String& pasteboardType, const String& pasteboardName)
{
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::SetPasteboardPathnamesForType(pasteboardName, pasteboardType, pathnames), Messages::WebProcessPool::SetPasteboardPathnamesForType::Reply(newChangeCount), 0);
    return newChangeCount;
}

long WebPlatformStrategies::setStringForType(const String& string, const String& pasteboardType, const String& pasteboardName)
{
    uint64_t newChangeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::SetPasteboardStringForType(pasteboardName, pasteboardType, string), Messages::WebProcessPool::SetPasteboardStringForType::Reply(newChangeCount), 0);
    return newChangeCount;
}

#if PLATFORM(IOS)
void WebPlatformStrategies::writeToPasteboard(const WebCore::PasteboardWebContent& content)
{
    WebProcess::shared().parentProcessConnection()->send(Messages::WebProcessPool::WriteWebContentToPasteboard(content), 0);
}

void WebPlatformStrategies::writeToPasteboard(const WebCore::PasteboardImage& image)
{
    WebProcess::shared().parentProcessConnection()->send(Messages::WebProcessPool::WriteImageToPasteboard(image), 0);
}

void WebPlatformStrategies::writeToPasteboard(const String& pasteboardType, const String& text)
{
    WebProcess::shared().parentProcessConnection()->send(Messages::WebProcessPool::WriteStringToPasteboard(pasteboardType, text), 0);
}

int WebPlatformStrategies::getPasteboardItemsCount()
{
    uint64_t itemsCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardItemsCount(), Messages::WebProcessPool::GetPasteboardItemsCount::Reply(itemsCount), 0);
    return itemsCount;
}

PassRefPtr<WebCore::SharedBuffer> WebPlatformStrategies::readBufferFromPasteboard(int index, const String& pasteboardType)
{
    SharedMemory::Handle handle;
    uint64_t size = 0;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::ReadBufferFromPasteboard(index, pasteboardType), Messages::WebProcessPool::ReadBufferFromPasteboard::Reply(handle, size), 0);
    if (handle.isNull())
        return 0;
    RefPtr<SharedMemory> sharedMemoryBuffer = SharedMemory::create(handle, SharedMemory::ReadOnly);
    return SharedBuffer::create(static_cast<unsigned char *>(sharedMemoryBuffer->data()), size);
}

WebCore::URL WebPlatformStrategies::readURLFromPasteboard(int index, const String& pasteboardType)
{
    String urlString;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::ReadURLFromPasteboard(index, pasteboardType), Messages::WebProcessPool::ReadURLFromPasteboard::Reply(urlString), 0);
    return URL(ParsedURLString, urlString);
}

String WebPlatformStrategies::readStringFromPasteboard(int index, const String& pasteboardType)
{
    String value;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::ReadStringFromPasteboard(index, pasteboardType), Messages::WebProcessPool::ReadStringFromPasteboard::Reply(value), 0);
    return value;
}

long WebPlatformStrategies::changeCount()
{
    uint64_t changeCount;
    WebProcess::shared().parentProcessConnection()->sendSync(Messages::WebProcessPool::GetPasteboardChangeCount(String()), Messages::WebProcessPool::GetPasteboardChangeCount::Reply(changeCount), 0);
    return changeCount;
}

#endif // PLATFORM(IOS)

#endif // PLATFORM(COCOA)

} // namespace WebKit
