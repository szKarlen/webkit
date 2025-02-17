/*
 * Copyright (C) 2006, 2007, 2011, 2013-2014 Apple Inc. All rights reserved.
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

#ifndef WebFrame_H
#define WebFrame_H

#include "WebKit.h"
#include "WebDataSource.h"
#include "DOMCoreClasses.h"

#include "AccessibleDocument.h"

#include <WebCore/AdjustViewSizeOrNot.h>
#include <WebCore/FrameWin.h>
#include <WebCore/GraphicsContext.h>
#include <WebCore/URL.h>
#include <WebCore/ResourceHandleClient.h>
#include <WebCore/NodeList.h>
#include <WebCore/Element.h>

#include <WTF/RefPtr.h>
#include <WTF/HashMap.h>
#include <WTF/OwnPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {
    class AuthenticationChallenge;
    class DocumentLoader;
    class Element;
    class FloatSize;
    class Frame;
    class GraphicsContext;
    class HTMLFrameOwnerElement;
    class IntRect;
    class Page;
    class ResourceError;
    class SharedBuffer;
	class DOMNodeList;
	class WebProgressTracker;
}

typedef const struct OpaqueJSContext* JSContextRef;
typedef struct OpaqueJSValue* JSObjectRef;

#if USE(CG)
typedef struct CGContext PlatformGraphicsContext;
#elif USE(CAIRO)
namespace WebCore {
class PlatformContextCairo;
}
typedef class WebCore::PlatformContextCairo PlatformGraphicsContext;
#endif

class WebFrame;
class WebFramePolicyListener;
class WebHistory;
class WebView;

interface IWebHistoryItemPrivate;

WebFrame* kit(WebCore::Frame*);
WebCore::Frame* core(WebFrame*);

class DECLSPEC_UUID("{A3676398-4485-4a9d-87DC-CB5A40E6351D}") WebFrame : public IWebFrame, IWebFramePrivate, IWebDocumentText
{
public:
    static WebFrame* createInstance();
protected:
    WebFrame();
    ~WebFrame();

public:
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    //IWebFrame
    virtual HRESULT STDMETHODCALLTYPE name( 
        /* [retval][out] */ BSTR *frameName);
    
    virtual HRESULT STDMETHODCALLTYPE webView( 
        /* [retval][out] */ IWebView **view);

    virtual HRESULT STDMETHODCALLTYPE frameView(
        /* [retval][out] */ IWebFrameView **view);

    virtual HRESULT STDMETHODCALLTYPE DOMDocument( 
        /* [retval][out] */ IDOMDocument** document);

    virtual HRESULT STDMETHODCALLTYPE DOMWindow(/* [retval][out] */ IDOMWindow**);
    
    virtual HRESULT STDMETHODCALLTYPE frameElement( 
        /* [retval][out] */ IDOMHTMLElement **frameElement);
    
    virtual HRESULT STDMETHODCALLTYPE loadRequest( 
        /* [in] */ IWebURLRequest *request);
    
    virtual HRESULT STDMETHODCALLTYPE loadData( 
        /* [in] */ IStream *data,
        /* [in] */ BSTR mimeType,
        /* [in] */ BSTR textEncodingName,
        /* [in] */ BSTR url);
    
    virtual HRESULT STDMETHODCALLTYPE loadHTMLString( 
        /* [in] */ BSTR string,
        /* [in] */ BSTR baseURL);
    
    virtual HRESULT STDMETHODCALLTYPE loadAlternateHTMLString( 
        /* [in] */ BSTR str,
        /* [in] */ BSTR baseURL,
        /* [in] */ BSTR unreachableURL);
    
    virtual HRESULT STDMETHODCALLTYPE loadArchive( 
        /* [in] */ IWebArchive *archive);
    
    virtual HRESULT STDMETHODCALLTYPE dataSource( 
        /* [retval][out] */ IWebDataSource **source);
    
    virtual HRESULT STDMETHODCALLTYPE provisionalDataSource( 
        /* [retval][out] */ IWebDataSource **source);
    
    virtual HRESULT STDMETHODCALLTYPE stopLoading( void);
    
    virtual HRESULT STDMETHODCALLTYPE reload( void);
    
    virtual HRESULT STDMETHODCALLTYPE findFrameNamed( 
        /* [in] */ BSTR name,
        /* [retval][out] */ IWebFrame **frame);
    
    virtual HRESULT STDMETHODCALLTYPE parentFrame( 
        /* [retval][out] */ IWebFrame **frame);
    
    virtual HRESULT STDMETHODCALLTYPE childFrames( 
        /* [retval][out] */ IEnumVARIANT **enumFrames);

    virtual HRESULT STDMETHODCALLTYPE currentForm( 
        /* [retval][out] */ IDOMElement **formElement);

    virtual /* [local] */ JSGlobalContextRef STDMETHODCALLTYPE globalContext();

    // IWebFramePrivate
    virtual HRESULT STDMETHODCALLTYPE unused1() { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE renderTreeAsExternalRepresentation(BOOL forPrinting, BSTR *result);

    virtual HRESULT STDMETHODCALLTYPE pageNumberForElementById(
        /* [in] */ BSTR id,
        /* [in] */ float pageWidthInPixels,
        /* [in] */ float pageHeightInPixels,
        /* [retval][out] */ int* result);

    virtual HRESULT STDMETHODCALLTYPE numberOfPages(
        /* [in] */ float pageWidthInPixels,
        /* [in] */ float pageHeightInPixels,
        /* [retval][out] */ int* result);

    virtual HRESULT STDMETHODCALLTYPE scrollOffset(
        /* [retval][out] */ SIZE* offset);

    virtual HRESULT STDMETHODCALLTYPE layout();

    virtual HRESULT STDMETHODCALLTYPE firstLayoutDone(
        /* [retval][out] */ BOOL* result);

    virtual HRESULT STDMETHODCALLTYPE unused2() { return E_NOTIMPL; }

    virtual HRESULT STDMETHODCALLTYPE pendingFrameUnloadEventCount( 
        /* [retval][out] */ UINT* result);

    virtual HRESULT STDMETHODCALLTYPE unused3() { return E_NOTIMPL; }
    
    virtual HRESULT STDMETHODCALLTYPE setInPrintingMode( 
        /* [in] */ BOOL value,
        /* [in] */ HDC printDC);
        
    virtual HRESULT STDMETHODCALLTYPE getPrintedPageCount( 
        /* [in] */ HDC printDC,
        /* [retval][out] */ UINT *pageCount);
    
    virtual HRESULT STDMETHODCALLTYPE spoolPages( 
        /* [in] */ HDC printDC,
        /* [in] */ UINT startPage,
        /* [in] */ UINT endPage,
        /* [retval][out] */ void* ctx);

    virtual HRESULT STDMETHODCALLTYPE isFrameSet( 
        /* [retval][out] */ BOOL* result);

    virtual HRESULT STDMETHODCALLTYPE string( 
        /* [retval][out] */ BSTR* result);

    virtual HRESULT STDMETHODCALLTYPE size( 
        /* [retval][out] */ SIZE *size);

    virtual HRESULT STDMETHODCALLTYPE hasScrollBars( 
        /* [retval][out] */ BOOL *result);
    
    virtual HRESULT STDMETHODCALLTYPE contentBounds( 
        /* [retval][out] */ RECT *result);
    
    virtual HRESULT STDMETHODCALLTYPE frameBounds( 
        /* [retval][out] */ RECT *result);

    virtual HRESULT STDMETHODCALLTYPE isDescendantOfFrame( 
        /* [in] */ IWebFrame *ancestor,
        /* [retval][out] */ BOOL *result);

    virtual HRESULT STDMETHODCALLTYPE setAllowsScrolling(
        /* [in] */ BOOL flag);

    virtual HRESULT STDMETHODCALLTYPE allowsScrolling(
        /* [retval][out] */ BOOL *flag);

    virtual HRESULT STDMETHODCALLTYPE setIsDisconnected(
        /* [in] */ BOOL flag);

    virtual HRESULT STDMETHODCALLTYPE setExcludeFromTextSearch(
        /* [in] */ BOOL flag);

    virtual HRESULT STDMETHODCALLTYPE reloadFromOrigin();

    virtual HRESULT STDMETHODCALLTYPE paintDocumentRectToContext(/* [in] */ RECT rect, /* [in] */ HDC deviceContext);

    virtual HRESULT STDMETHODCALLTYPE paintScrollViewRectToContextAtPoint(/* [in] */ RECT rect, /* [in] */ POINT pt, /* [in] */ HDC deviceContext);

    virtual HRESULT STDMETHODCALLTYPE elementDoesAutoComplete(
        /* [in] */ IDOMElement* element, 
        /* [retval][out] */ BOOL* result);

    virtual HRESULT STDMETHODCALLTYPE pauseAnimation(BSTR animationName, IDOMNode*, double secondsFromNow, BOOL* animationWasRunning);
    virtual HRESULT STDMETHODCALLTYPE pauseTransition(BSTR propertyName, IDOMNode*, double secondsFromNow, BOOL* transitionWasRunning);
    virtual HRESULT STDMETHODCALLTYPE numberOfActiveAnimations(UINT*);
    virtual HRESULT STDMETHODCALLTYPE loadPlainTextString(BSTR string, BSTR url);

    virtual HRESULT STDMETHODCALLTYPE isDisplayingStandaloneImage(BOOL*);

    virtual HRESULT STDMETHODCALLTYPE allowsFollowingLink(
        /* [in] */ BSTR url,
        /* [retval][out] */ BOOL* result);

    virtual HRESULT STDMETHODCALLTYPE stringByEvaluatingJavaScriptInScriptWorld(IWebScriptWorld*, JSObjectRef globalObjectRef, BSTR script, BSTR* evaluationResult);
    virtual JSGlobalContextRef STDMETHODCALLTYPE globalContextForScriptWorld(IWebScriptWorld*);

    virtual HRESULT STDMETHODCALLTYPE visibleContentRect(RECT*);

    virtual HRESULT STDMETHODCALLTYPE layerTreeAsText(BSTR*);

    virtual HRESULT STDMETHODCALLTYPE hasSpellingMarker(
        /* [in] */ UINT from,
        /* [in] */ UINT length,
        /* [retval][out] */ BOOL *result);

    virtual HRESULT STDMETHODCALLTYPE clearOpener();

    virtual HRESULT STDMETHODCALLTYPE setTextDirection(BSTR);

    virtual HRESULT STDMETHODCALLTYPE unused4() { return E_NOTIMPL; }

    virtual HRESULT STDMETHODCALLTYPE resumeAnimations();

    virtual HRESULT STDMETHODCALLTYPE suspendAnimations();

    // IWebDocumentText
    virtual HRESULT STDMETHODCALLTYPE supportsTextEncoding( 
        /* [retval][out] */ BOOL* result);
    
    virtual HRESULT STDMETHODCALLTYPE selectedString( 
        /* [retval][out] */ BSTR* result);
    
    virtual HRESULT STDMETHODCALLTYPE selectAll();
    
    virtual HRESULT STDMETHODCALLTYPE deselectAll();

    virtual HRESULT STDMETHODCALLTYPE isMainFrame(BOOL*);
    
	virtual HRESULT STDMETHODCALLTYPE scrollToAnchor(BSTR);

	virtual HRESULT STDMETHODCALLTYPE querySelector(BSTR, IDOMNode**);

	virtual HRESULT STDMETHODCALLTYPE querySelectorAll(BSTR, IDOMNodeList**);

	virtual HRESULT STDMETHODCALLTYPE setScrollbarPolicy(WebScrollBarOrientation orientation, WebScrollBarPolicy policy);

	virtual HRESULT STDMETHODCALLTYPE setScrollBarValue(WebScrollBarOrientation orientation, UINT value);

	virtual HRESULT STDMETHODCALLTYPE scrollBarMaximum(WebScrollBarOrientation orientation, int* value);

    // FrameLoaderClient
    virtual void frameLoaderDestroyed();

    // WebFrame
    PassRefPtr<WebCore::Frame> createSubframeWithOwnerElement(IWebView*, WebCore::Page*, WebCore::HTMLFrameOwnerElement*);
    void initWithWebView(IWebView*, WebCore::Page*);
    WebCore::Frame* impl();
    void invalidate();
    void unmarkAllMisspellings();
    void unmarkAllBadGrammar();

    void updateBackground();

    // WebFrame (matching WebCoreFrameBridge)
    HRESULT inViewSourceMode(BOOL *flag);
    HRESULT setInViewSourceMode(BOOL flag);
    HRESULT elementWithName(BSTR name, IDOMElement* form, IDOMElement** element);
    HRESULT formForElement(IDOMElement* element, IDOMElement** form);
    HRESULT controlsInForm(IDOMElement* form, IDOMElement** controls, int* cControls);
    HRESULT elementIsPassword(IDOMElement* element, bool* result);
    HRESULT searchForLabelsBeforeElement(const BSTR* labels, unsigned cLabels, IDOMElement* beforeElement, unsigned* resultDistance, BOOL* resultIsInCellAbove, BSTR* result);
    HRESULT matchLabelsAgainstElement(const BSTR* labels, int cLabels, IDOMElement* againstElement, BSTR* result);
    HRESULT canProvideDocumentSource(bool* result);

    WebCore::URL url() const;

    WebView* webView() const;
    void setWebView(WebView*);

    COMPtr<IAccessible> accessible() const;

protected:
    void loadHTMLString(BSTR string, BSTR baseURL, BSTR unreachableURL);
    void loadData(PassRefPtr<WebCore::SharedBuffer>, BSTR mimeType, BSTR textEncodingName, BSTR baseURL, BSTR failingURL);
    const Vector<WebCore::IntRect>& computePageRects(HDC printDC);
    void setPrinting(bool printing, const WebCore::FloatSize& pageSize, const WebCore::FloatSize& originalPageSize, float maximumShrinkRatio, WebCore::AdjustViewSizeOrNot);
    void headerAndFooterHeights(float*, float*);
    WebCore::IntRect printerMarginRect(HDC);
    void spoolPage (PlatformGraphicsContext* pctx, WebCore::GraphicsContext* spoolCtx, HDC printDC, IWebUIDelegate*, float headerHeight, float footerHeight, UINT page, UINT pageCount);
    void drawHeader(PlatformGraphicsContext* pctx, IWebUIDelegate*, const WebCore::IntRect& pageRect, float headerHeight);
    void drawFooter(PlatformGraphicsContext* pctx, IWebUIDelegate*, const WebCore::IntRect& pageRect, UINT page, UINT pageCount, float headerHeight, float footerHeight);

protected:
    ULONG               m_refCount;
    class WebFramePrivate;
    WebFramePrivate*    d;
    bool                m_quickRedirectComing;
    WebCore::URL       m_originalRequestURL;
    bool                m_inPrintingMode;
    Vector<WebCore::IntRect> m_pageRects;
    int m_pageHeight;   // height of the page adjusted by margins
    mutable COMPtr<AccessibleDocument> m_accessible;
};

#endif
