/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#import "config.h"
#import "WebVideoFullscreenManagerProxy.h"

#if PLATFORM(IOS)

#import "RemoteLayerTreeDrawingAreaProxy.h"
#import "WebPageProxy.h"
#import "WebProcessProxy.h"
#import "WebVideoFullscreenManagerMessages.h"
#import "WebVideoFullscreenManagerProxyMessages.h"
#import <QuartzCore/CoreAnimation.h>
#import <WebCore/TimeRanges.h>
#import <WebKitSystemInterface.h>

#if USE(APPLE_INTERNAL_SDK)
#import <UIKit/UIWindow_Private.h>
#else
#import <UIKit/UIWindow.h>
@interface UIWindow (Details)
+ (mach_port_t)_synchronizeDrawingAcrossProcesses;
@end
#endif

using namespace WebCore;

namespace WebKit {

#if __IPHONE_OS_VERSION_MIN_REQUIRED > 80200

PassRefPtr<WebVideoFullscreenManagerProxy> WebVideoFullscreenManagerProxy::create(WebPageProxy& page)
{
    return adoptRef(new WebVideoFullscreenManagerProxy(page));
}

WebVideoFullscreenManagerProxy::WebVideoFullscreenManagerProxy(WebPageProxy& page)
    : m_page(&page)
{
    m_page->process().addMessageReceiver(Messages::WebVideoFullscreenManagerProxy::messageReceiverName(), m_page->pageID(), *this);
    setWebVideoFullscreenModel(this);
    setWebVideoFullscreenChangeObserver(this);
}

WebVideoFullscreenManagerProxy::~WebVideoFullscreenManagerProxy()
{
    if (!m_page)
        return;
    invalidate();
}

void WebVideoFullscreenManagerProxy::invalidate()
{
    WebVideoFullscreenInterfaceAVKit::invalidate();

    setWebVideoFullscreenModel(nullptr);
    setWebVideoFullscreenChangeObserver(nullptr);

    m_page->process().removeMessageReceiver(Messages::WebVideoFullscreenManagerProxy::messageReceiverName(), m_page->pageID());
    m_page = nullptr;

    [m_layerHost removeFromSuperlayer];
    m_layerHost.clear();
}

void WebVideoFullscreenManagerProxy::setupFullscreenWithID(uint32_t videoLayerID, WebCore::IntRect initialRect, float hostingDeviceScaleFactor, HTMLMediaElement::VideoFullscreenMode videoFullscreenMode, bool allowOptimizedFullscreen)
{
    ASSERT(videoLayerID);
    m_layerHost = WKMakeRenderLayer(videoLayerID);
    if (hostingDeviceScaleFactor != 1) {
        // Invert the scale transform added in the WebProcess to fix <rdar://problem/18316542>.
        float inverseScale = 1 / hostingDeviceScaleFactor;
        [m_layerHost setTransform:CATransform3DMakeScale(inverseScale, inverseScale, 1)];
    }

    UIView *parentView = downcast<RemoteLayerTreeDrawingAreaProxy>(*m_page->drawingArea()).remoteLayerTreeHost().rootLayer();
    setupFullscreen(*m_layerHost.get(), initialRect, parentView, videoFullscreenMode, allowOptimizedFullscreen);
}
    
void WebVideoFullscreenManagerProxy::fullscreenModeChanged(HTMLMediaElement::VideoFullscreenMode mode)
{
    m_page->send(Messages::WebVideoFullscreenManager::fullscreenModeChanged(mode), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::setSeekableRangesVector(const Vector<std::pair<double, double>>& ranges)
{
    RefPtr<TimeRanges> timeRanges = TimeRanges::create();
    for (const auto& range : ranges)
    {
        ASSERT(isfinite(range.first));
        ASSERT(isfinite(range.second));
        ASSERT(range.second >= range.first);
        timeRanges->add(range.first, range.second);
    }

    setSeekableRanges(*timeRanges);
}

void WebVideoFullscreenManagerProxy::setExternalPlaybackProperties(bool enabled, uint32_t targetType, String localizedDeviceName)
{
    WebVideoFullscreenInterface::ExternalPlaybackTargetType type = static_cast<WebVideoFullscreenInterface::ExternalPlaybackTargetType>(targetType);
    
    ASSERT(type == WebVideoFullscreenInterface::TargetTypeAirPlay || type == WebVideoFullscreenInterface::TargetTypeTVOut || type == WebVideoFullscreenInterface::TargetTypeNone);
    
    setExternalPlayback(enabled, type, localizedDeviceName);
}
    
void WebVideoFullscreenManagerProxy::fullscreenMayReturnToInline()
{
    m_page->fullscreenMayReturnToInline();
}

void WebVideoFullscreenManagerProxy::requestExitFullscreen()
{
    m_page->send(Messages::WebVideoFullscreenManager::RequestExitFullscreen(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::didExitFullscreen()
{
    m_page->send(Messages::WebVideoFullscreenManager::DidExitFullscreen(), m_page->pageID());
    m_page->didExitFullscreen();
}
    
void WebVideoFullscreenManagerProxy::didCleanupFullscreen()
{
    [m_layerHost removeFromSuperlayer];
    m_layerHost.clear();
    m_page->send(Messages::WebVideoFullscreenManager::DidCleanupFullscreen(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::didSetupFullscreen()
{
    m_page->send(Messages::WebVideoFullscreenManager::DidSetupFullscreen(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::didEnterFullscreen()
{
    m_page->send(Messages::WebVideoFullscreenManager::DidEnterFullscreen(), m_page->pageID());
    m_page->didEnterFullscreen();
}

void WebVideoFullscreenManagerProxy::play()
{
    m_page->send(Messages::WebVideoFullscreenManager::Play(), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::pause()
{
    m_page->send(Messages::WebVideoFullscreenManager::Pause(), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::togglePlayState()
{
    m_page->send(Messages::WebVideoFullscreenManager::TogglePlayState(), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::beginScrubbing()
{
    m_page->send(Messages::WebVideoFullscreenManager::BeginScrubbing(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::endScrubbing()
{
    m_page->send(Messages::WebVideoFullscreenManager::EndScrubbing(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::seekToTime(double time)
{
    m_page->send(Messages::WebVideoFullscreenManager::SeekToTime(time), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::fastSeek(double time)
{
    m_page->send(Messages::WebVideoFullscreenManager::FastSeek(time), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::beginScanningForward()
{
    m_page->send(Messages::WebVideoFullscreenManager::BeginScanningForward(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::beginScanningBackward()
{
    m_page->send(Messages::WebVideoFullscreenManager::BeginScanningBackward(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::endScanning()
{
    m_page->send(Messages::WebVideoFullscreenManager::EndScanning(), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::setVideoLayerFrame(WebCore::FloatRect frame)
{
    IPC::Attachment fencePort([UIWindow _synchronizeDrawingAcrossProcesses], MACH_MSG_TYPE_MOVE_SEND);
    m_page->send(Messages::WebVideoFullscreenManager::SetVideoLayerFrameFenced(frame, fencePort), m_page->pageID());
}

void WebVideoFullscreenManagerProxy::setVideoLayerGravity(WebCore::WebVideoFullscreenModel::VideoGravity gravity)
{
    m_page->send(Messages::WebVideoFullscreenManager::SetVideoLayerGravityEnum((unsigned)gravity), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::selectAudioMediaOption(uint64_t index)
{
    m_page->send(Messages::WebVideoFullscreenManager::SelectAudioMediaOption(index), m_page->pageID());
}
    
void WebVideoFullscreenManagerProxy::selectLegibleMediaOption(uint64_t index)
{
    m_page->send(Messages::WebVideoFullscreenManager::SelectLegibleMediaOption(index), m_page->pageID());
}
#endif

} // namespace WebKit

#endif // PLATFORM(IOS)
