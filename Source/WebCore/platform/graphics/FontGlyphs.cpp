/*
 * Copyright (C) 2006, 2013 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FontGlyphs.h"

#include "Font.h"
#include "FontCache.h"
#include "GlyphPageTreeNode.h"
#include "SegmentedFontData.h"

namespace WebCore {


FontGlyphs::FontGlyphs(PassRefPtr<FontSelector> fontSelector)
    : m_pageZero(0)
    , m_cachedPrimarySimpleFontData(0)
    , m_fontSelector(fontSelector)
    , m_fontSelectorVersion(m_fontSelector ? m_fontSelector->version() : 0)
    , m_familyIndex(0)
    , m_generation(fontCache().generation())
    , m_pitch(UnknownPitch)
    , m_loadingCustomFonts(false)
    , m_isForPlatformFont(false)
{
}

FontGlyphs::FontGlyphs(const FontPlatformData& platformData)
    : m_pageZero(0)
    , m_cachedPrimarySimpleFontData(0)
    , m_fontSelector(0)
    , m_fontSelectorVersion(0)
    , m_familyIndex(cAllFamiliesScanned)
    , m_generation(fontCache().generation())
    , m_pitch(UnknownPitch)
    , m_loadingCustomFonts(false)
    , m_isForPlatformFont(true)
{
    RefPtr<FontData> fontData = fontCache().getCachedFontData(&platformData);
    m_realizedFontData.append(fontData.release());
}

void FontGlyphs::releaseFontData()
{
    unsigned numFonts = m_realizedFontData.size();
    for (unsigned i = 0; i < numFonts; ++i) {
        if (m_realizedFontData[i]->isCustomFont())
            continue;
        fontCache().releaseFontData(downcast<SimpleFontData>(m_realizedFontData[i].get()));
    }
}

void FontGlyphs::determinePitch(const FontDescription& description)
{
    const FontData* fontData = primaryFontData(description);
    if (is<SimpleFontData>(*fontData))
        m_pitch = downcast<SimpleFontData>(*fontData).pitch();
    else {
        const SegmentedFontData& segmentedFontData = downcast<SegmentedFontData>(*fontData);
        unsigned numRanges = segmentedFontData.numRanges();
        if (numRanges == 1)
            m_pitch = segmentedFontData.rangeAt(0).fontData()->pitch();
        else
            m_pitch = VariablePitch;
    }
}

const FontData* FontGlyphs::realizeFontDataAt(const FontDescription& description, unsigned realizedFontIndex)
{
    if (realizedFontIndex < m_realizedFontData.size())
        return m_realizedFontData[realizedFontIndex].get(); // This fallback font is already in our list.

    // Make sure we're not passing in some crazy value here.
    ASSERT(realizedFontIndex == m_realizedFontData.size());

    if (m_familyIndex <= cAllFamiliesScanned) {
        if (!m_fontSelector)
            return 0;

        size_t index = cAllFamiliesScanned - m_familyIndex;
        if (index == m_fontSelector->fallbackFontDataCount())
            return 0;

        m_familyIndex--;
        RefPtr<FontData> fallback = m_fontSelector->getFallbackFontData(description, index);
        if (fallback)
            m_realizedFontData.append(fallback);
        return fallback.get();
    }

    // Ask the font cache for the font data.
    // We are obtaining this font for the first time. We keep track of the families we've looked at before
    // in |m_familyIndex|, so that we never scan the same spot in the list twice. getFontData will adjust our
    // |m_familyIndex| as it scans for the right font to make.
    ASSERT(fontCache().generation() == m_generation);
    RefPtr<FontData> result = fontCache().getFontData(description, m_familyIndex, m_fontSelector.get());
    if (result) {
        m_realizedFontData.append(result);
        if (result->isLoading())
            m_loadingCustomFonts = true;
    }
    return result.get();
}

static inline bool isInRange(UChar32 character, UChar32 lowerBound, UChar32 upperBound)
{
    return character >= lowerBound && character <= upperBound;
}

static bool shouldIgnoreRotation(UChar32 character)
{
    if (character == 0x000A7 || character == 0x000A9 || character == 0x000AE)
        return true;

    if (character == 0x000B6 || character == 0x000BC || character == 0x000BD || character == 0x000BE)
        return true;

    if (isInRange(character, 0x002E5, 0x002EB))
        return true;
    
    if (isInRange(character, 0x01100, 0x011FF) || isInRange(character, 0x01401, 0x0167F) || isInRange(character, 0x01800, 0x018FF))
        return true;

    if (character == 0x02016 || character == 0x02020 || character == 0x02021 || character == 0x2030 || character == 0x02031)
        return true;

    if (isInRange(character, 0x0203B, 0x0203D) || character == 0x02042 || character == 0x02044 || character == 0x02047
        || character == 0x02048 || character == 0x02049 || character == 0x2051)
        return true;

    if (isInRange(character, 0x02065, 0x02069) || isInRange(character, 0x020DD, 0x020E0)
        || isInRange(character, 0x020E2, 0x020E4) || isInRange(character, 0x02100, 0x02117)
        || isInRange(character, 0x02119, 0x02131) || isInRange(character, 0x02133, 0x0213F))
        return true;

    if (isInRange(character, 0x02145, 0x0214A) || character == 0x0214C || character == 0x0214D
        || isInRange(character, 0x0214F, 0x0218F))
        return true;

    if (isInRange(character, 0x02300, 0x02307) || isInRange(character, 0x0230C, 0x0231F)
        || isInRange(character, 0x02322, 0x0232B) || isInRange(character, 0x0237D, 0x0239A)
        || isInRange(character, 0x023B4, 0x023B6) || isInRange(character, 0x023BA, 0x023CF)
        || isInRange(character, 0x023D1, 0x023DB) || isInRange(character, 0x023E2, 0x024FF))
        return true;

    if (isInRange(character, 0x025A0, 0x02619) || isInRange(character, 0x02620, 0x02767)
        || isInRange(character, 0x02776, 0x02793) || isInRange(character, 0x02B12, 0x02B2F)
        || isInRange(character, 0x02B4D, 0x02BFF) || isInRange(character, 0x02E80, 0x03007))
        return true;

    if (character == 0x03012 || character == 0x03013 || isInRange(character, 0x03020, 0x0302F)
        || isInRange(character, 0x03031, 0x0309F) || isInRange(character, 0x030A1, 0x030FB)
        || isInRange(character, 0x030FD, 0x0A4CF))
        return true;

    if (isInRange(character, 0x0A840, 0x0A87F) || isInRange(character, 0x0A960, 0x0A97F)
        || isInRange(character, 0x0AC00, 0x0D7FF) || isInRange(character, 0x0E000, 0x0FAFF))
        return true;

    if (isInRange(character, 0x0FE10, 0x0FE1F) || isInRange(character, 0x0FE30, 0x0FE48)
        || isInRange(character, 0x0FE50, 0x0FE57) || isInRange(character, 0x0FE5F, 0x0FE62)
        || isInRange(character, 0x0FE67, 0x0FE6F))
        return true;

    if (isInRange(character, 0x0FF01, 0x0FF07) || isInRange(character, 0x0FF0A, 0x0FF0C)
        || isInRange(character, 0x0FF0E, 0x0FF19) || character == 0x0FF1B || isInRange(character, 0x0FF1F, 0x0FF3A))
        return true;

    if (character == 0x0FF3C || character == 0x0FF3E)
        return true;

    if (isInRange(character, 0x0FF40, 0x0FF5A) || isInRange(character, 0x0FFE0, 0x0FFE2)
        || isInRange(character, 0x0FFE4, 0x0FFE7) || isInRange(character, 0x0FFF0, 0x0FFF8)
        || character == 0x0FFFD)
        return true;

    if (isInRange(character, 0x13000, 0x1342F) || isInRange(character, 0x1B000, 0x1B0FF)
        || isInRange(character, 0x1D000, 0x1D1FF) || isInRange(character, 0x1D300, 0x1D37F)
        || isInRange(character, 0x1F000, 0x1F64F) || isInRange(character, 0x1F680, 0x1F77F))
        return true;
    
    if (isInRange(character, 0x20000, 0x2FFFD) || isInRange(character, 0x30000, 0x3FFFD))
        return true;

    return false;
}

#if PLATFORM(COCOA)
static GlyphData glyphDataForCJKCharacterWithoutSyntheticItalic(UChar32 character, GlyphData& data, unsigned pageNumber)
{
    RefPtr<SimpleFontData> nonItalicFontData = data.fontData->nonSyntheticItalicFontData();
    GlyphPageTreeNode* nonItalicNode = GlyphPageTreeNode::getRootChild(nonItalicFontData.get(), pageNumber);
    GlyphPage* nonItalicPage = nonItalicNode->page();
    if (nonItalicPage) {
        GlyphData nonItalicData = nonItalicPage->glyphDataForCharacter(character);
        if (nonItalicData.fontData)
            return nonItalicData;
    }
    return data;
}
#endif
    
static GlyphData glyphDataForNonCJKCharacterWithGlyphOrientation(UChar32 character, NonCJKGlyphOrientation orientation, GlyphData& data, unsigned pageNumber)
{
    if (orientation == NonCJKGlyphOrientationUpright || shouldIgnoreRotation(character)) {
        RefPtr<SimpleFontData> uprightFontData = data.fontData->uprightOrientationFontData();
        GlyphPageTreeNode* uprightNode = GlyphPageTreeNode::getRootChild(uprightFontData.get(), pageNumber);
        GlyphPage* uprightPage = uprightNode->page();
        if (uprightPage) {
            GlyphData uprightData = uprightPage->glyphDataForCharacter(character);
            // If the glyphs are the same, then we know we can just use the horizontal glyph rotated vertically to be upright.
            if (data.glyph == uprightData.glyph)
                return data;
            // The glyphs are distinct, meaning that the font has a vertical-right glyph baked into it. We can't use that
            // glyph, so we fall back to the upright data and use the horizontal glyph.
            if (uprightData.fontData)
                return uprightData;
        }
    } else if (orientation == NonCJKGlyphOrientationVerticalRight) {
        RefPtr<SimpleFontData> verticalRightFontData = data.fontData->verticalRightOrientationFontData();
        GlyphPageTreeNode* verticalRightNode = GlyphPageTreeNode::getRootChild(verticalRightFontData.get(), pageNumber);
        GlyphPage* verticalRightPage = verticalRightNode->page();
        if (verticalRightPage) {
            GlyphData verticalRightData = verticalRightPage->glyphDataForCharacter(character);
            // If the glyphs are distinct, we will make the assumption that the font has a vertical-right glyph baked
            // into it.
            if (data.glyph != verticalRightData.glyph)
                return data;
            // The glyphs are identical, meaning that we should just use the horizontal glyph.
            if (verticalRightData.fontData)
                return verticalRightData;
        }
    }
    return data;
}

GlyphData FontGlyphs::glyphDataForSystemFallback(UChar32 c, const FontDescription& description, FontDataVariant variant, unsigned pageNumber, GlyphPageTreeNode& node)
{
    ASSERT(node.page());
    ASSERT(node.isSystemFallback());
    // System fallback is character-dependent. When we get here, we
    // know that the character in question isn't in the system fallback
    // font's glyph page. Try to lazily create it here.
    UChar codeUnits[2];
    int codeUnitsLength;
    if (c <= 0xFFFF) {
        codeUnits[0] = Font::normalizeSpaces(c);
        codeUnitsLength = 1;
    } else {
        codeUnits[0] = U16_LEAD(c);
        codeUnits[1] = U16_TRAIL(c);
        codeUnitsLength = 2;
    }
    const SimpleFontData* originalFontData = primaryFontData(description)->fontDataForCharacter(c);
    RefPtr<SimpleFontData> characterFontData = fontCache().systemFallbackForCharacters(description, originalFontData, m_isForPlatformFont, codeUnits, codeUnitsLength);
    if (!characterFontData)
        return GlyphData();

    if (characterFontData->platformData().orientation() == Vertical && !characterFontData->hasVerticalGlyphs() && Font::isCJKIdeographOrSymbol(c))
        variant = BrokenIdeographVariant;
    if (variant != NormalVariant) {
        characterFontData = characterFontData->variantFontData(description, variant);
        ASSERT(characterFontData);
    }

    GlyphData data;
    if (GlyphPage* fallbackPage = GlyphPageTreeNode::getRootChild(characterFontData.get(), pageNumber)->page())
        data = fallbackPage->glyphDataForCharacter(c);

    // Cache it so we don't have to do system fallback again next time.
    if (variant == NormalVariant && data.glyph) {
        node.page()->setGlyphDataForCharacter(c, data.glyph, data.fontData);
        data.fontData->setMaxGlyphPageTreeLevel(std::max(data.fontData->maxGlyphPageTreeLevel(), node.level()));
        if (!Font::isCJKIdeographOrSymbol(c) && data.fontData->platformData().orientation() != Horizontal && !data.fontData->isTextOrientationFallback())
            return glyphDataForNonCJKCharacterWithGlyphOrientation(c, description.nonCJKGlyphOrientation(), data, pageNumber);
    }
    return data;
}

GlyphData FontGlyphs::glyphDataForVariant(UChar32 c, const FontDescription& description, FontDataVariant variant, unsigned pageNumber, GlyphPageTreeNode*& node)
{
    while (true) {
        if (GlyphPage* page = node->page()) {
            GlyphData data = page->glyphDataForCharacter(c);
            if (data.fontData) {
                // The variantFontData function should not normally return 0.
                // But if it does, we will just render the capital letter big.
                RefPtr<SimpleFontData> variantFontData = data.fontData->variantFontData(description, variant);
                if (!variantFontData)
                    return data;

                GlyphPageTreeNode* variantNode = GlyphPageTreeNode::getRootChild(variantFontData.get(), pageNumber);
                GlyphPage* variantPage = variantNode->page();
                if (variantPage)
                    return variantPage->glyphDataForCharacter(c);

                // Do not attempt system fallback off the variantFontData. This is the very unlikely case that
                // a font has the lowercase character but the small caps font does not have its uppercase version.
                return GlyphData();
            }

            if (node->isSystemFallback())
                return glyphDataForSystemFallback(c, description, variant, pageNumber, *node);
        }

        node = node->getChild(realizeFontDataAt(description, node->level()), pageNumber);
    }
}

GlyphData FontGlyphs::glyphDataForCharacter(const FontDescription& description, UChar32 c, bool mirror, FontDataVariant variant)
{
    ASSERT(isMainThread());

    if (variant == AutoVariant) {
        if (description.smallCaps() && !primarySimpleFontData(description)->isSVGFont()) {
            UChar32 upperC = u_toupper(c);
            if (upperC != c) {
                c = upperC;
                variant = SmallCapsVariant;
            } else
                variant = NormalVariant;
        } else
            variant = NormalVariant;
    }

    if (mirror)
        c = u_charMirror(c);

    const unsigned pageNumber = (c / GlyphPage::size);

    GlyphPageTreeNode*& node = pageNumber ? m_pages.add(pageNumber, nullptr).iterator->value : m_pageZero;
    if (!node)
        node = GlyphPageTreeNode::getRootChild(realizeFontDataAt(description, 0), pageNumber);

    if (variant != NormalVariant)
        return glyphDataForVariant(c, description, variant, pageNumber, node);

    while (true) {
        if (GlyphPage* page = node->page()) {
            GlyphData data = page->glyphDataForCharacter(c);
            if (data.fontData) {
                if (data.fontData->platformData().orientation() == Vertical && !data.fontData->isTextOrientationFallback()) {
                    if (!Font::isCJKIdeographOrSymbol(c))
                        return glyphDataForNonCJKCharacterWithGlyphOrientation(c, description.nonCJKGlyphOrientation(), data, pageNumber);

                    if (!data.fontData->hasVerticalGlyphs()) {
                        // Use the broken ideograph font data. The broken ideograph font will use the horizontal width of glyphs
                        // to make sure you get a square (even for broken glyphs like symbols used for punctuation).
                        return glyphDataForVariant(c, description, BrokenIdeographVariant, pageNumber, node);
                    }
#if PLATFORM(COCOA)
                    if (data.fontData->platformData().syntheticOblique())
                        return glyphDataForCJKCharacterWithoutSyntheticItalic(c, data, pageNumber);
#endif
                }

                return data;
            }

            if (node->isSystemFallback())
                return glyphDataForSystemFallback(c, description, variant, pageNumber, *node);
        }

        node = node->getChild(realizeFontDataAt(description, node->level()), pageNumber);
    }
}

}
