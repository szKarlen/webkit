/*
 * Copyright (C) 2006, 2010, 2013 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef FontGlyphs_h
#define FontGlyphs_h

#include "FontSelector.h"
#include "GlyphPage.h"
#include "SimpleFontData.h"
#include "WidthCache.h"
#include <wtf/Forward.h>
#include <wtf/MainThread.h>

#if PLATFORM(IOS)
#include "WebCoreThread.h"
#endif

namespace WebCore {

class GlyphPageTreeNode;
class GraphicsContext;
class IntRect;
class FontDescription;
class FontPlatformData;
class FontSelector;

const int cAllFamiliesScanned = -1;

class FontGlyphs : public RefCounted<FontGlyphs> {
    WTF_MAKE_NONCOPYABLE(FontGlyphs);
public:
    typedef HashMap<int, GlyphPageTreeNode*, DefaultHash<int>::Hash> GlyphPages;

    static Ref<FontGlyphs> create(PassRefPtr<FontSelector> fontSelector) { return adoptRef(*new FontGlyphs(fontSelector)); }
    static Ref<FontGlyphs> createForPlatformFont(const FontPlatformData& platformData) { return adoptRef(*new FontGlyphs(platformData)); }

    ~FontGlyphs() { releaseFontData(); }

    bool isForPlatformFont() const { return m_isForPlatformFont; }

    GlyphData glyphDataForCharacter(const FontDescription&, UChar32, bool mirror, FontDataVariant);

    bool isFixedPitch(const FontDescription&);
    void determinePitch(const FontDescription&);

    bool loadingCustomFonts() const { return m_loadingCustomFonts; }

    FontSelector* fontSelector() { return m_fontSelector.get(); }
    // FIXME: It should be possible to combine fontSelectorVersion and generation.
    unsigned fontSelectorVersion() const { return m_fontSelectorVersion; }
    unsigned generation() const { return m_generation; }

    WidthCache& widthCache() { return m_widthCache; }
    const WidthCache& widthCache() const { return m_widthCache; }

    const SimpleFontData* primarySimpleFontData(const FontDescription&);
    const FontData* primaryFontData(const FontDescription& description) { return realizeFontDataAt(description, 0); }
    WEBCORE_EXPORT const FontData* realizeFontDataAt(const FontDescription&, unsigned index);

private:
    FontGlyphs(PassRefPtr<FontSelector>);
    FontGlyphs(const FontPlatformData&);

    GlyphData glyphDataForSystemFallback(UChar32, const FontDescription&, FontDataVariant, unsigned pageNumber, GlyphPageTreeNode&);
    GlyphData glyphDataForVariant(UChar32, const FontDescription&, FontDataVariant, unsigned pageNumber, GlyphPageTreeNode*&);

    WEBCORE_EXPORT void releaseFontData();
    
    Vector<RefPtr<FontData>, 1> m_realizedFontData;
    GlyphPages m_pages;
    GlyphPageTreeNode* m_pageZero;
    const SimpleFontData* m_cachedPrimarySimpleFontData;
    RefPtr<FontSelector> m_fontSelector;
    WidthCache m_widthCache;
    unsigned m_fontSelectorVersion;
    int m_familyIndex;
    unsigned short m_generation;
    unsigned m_pitch : 3; // Pitch
    unsigned m_loadingCustomFonts : 1;
    unsigned m_isForPlatformFont : 1;
};

inline bool FontGlyphs::isFixedPitch(const FontDescription& description)
{
    if (m_pitch == UnknownPitch)
        determinePitch(description);
    return m_pitch == FixedPitch;
};

inline const SimpleFontData* FontGlyphs::primarySimpleFontData(const FontDescription& description)
{
    ASSERT(isMainThread());
    if (!m_cachedPrimarySimpleFontData)
        m_cachedPrimarySimpleFontData = primaryFontData(description)->fontDataForCharacter(' ');
    return m_cachedPrimarySimpleFontData;
}

}

#endif
