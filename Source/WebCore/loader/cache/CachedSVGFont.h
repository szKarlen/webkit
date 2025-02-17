/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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

#ifndef CachedSVGFont_h
#define CachedSVGFont_h

#if ENABLE(SVG_FONTS)

#include "CachedFont.h"

namespace WebCore {

class SVGFontFaceElement;

class CachedSVGFont final : public CachedFont {
public:
    CachedSVGFont(const ResourceRequest&, SessionID);

    virtual bool ensureCustomFontData(bool externalSVG) override;
    
    virtual PassRefPtr<SimpleFontData> getFontData(const FontDescription&, const AtomicString& remoteURI, bool syntheticBold, bool syntheticItalic, bool externalSVG) override;

private:
    FontPlatformData platformDataFromCustomData(float size, bool bold, bool italic, FontOrientation = Horizontal, FontWidthVariant = RegularWidth, FontRenderingMode = NormalRenderingMode);

    SVGFontElement* getSVGFontById(const String&) const;

    SVGFontFaceElement* firstFontFace(const AtomicString& remoteURI);

    RefPtr<SVGDocument> m_externalSVGDocument;
    SVGFontElement* m_externalSVGFontElement;
};

}

SPECIALIZE_TYPE_TRAITS_CACHED_RESOURCE(CachedSVGFont, CachedResource::SVGFontResource)

#endif

#endif
