/*
 * Copyright (C) 2013 Apple Inc.  All rights reserved.
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

#if ENABLE(VIDEO_TRACK)

#include "HTMLElement.h"

namespace WebCore {

enum WebVTTNodeType {
    WebVTTNodeTypeNone = 0,
    WebVTTNodeTypeClass,
    WebVTTNodeTypeItalic,
    WebVTTNodeTypeLanguage,
    WebVTTNodeTypeBold,
    WebVTTNodeTypeUnderline,
    WebVTTNodeTypeRuby,
    WebVTTNodeTypeRubyText,
    WebVTTNodeTypeVoice
};

class WebVTTElement final : public Element {
public:
    static PassRefPtr<WebVTTElement> create(const WebVTTNodeType, Document&);
    PassRefPtr<HTMLElement> createEquivalentHTMLElement(Document&);

    virtual RefPtr<Element> cloneElementWithoutAttributesAndChildren(Document&) override;

    void setWebVTTNodeType(WebVTTNodeType type) { m_webVTTNodeType = static_cast<unsigned>(type); }
    WebVTTNodeType webVTTNodeType() const { return static_cast<WebVTTNodeType>(m_webVTTNodeType); }

    bool isPastNode() const { return m_isPastNode; }
    void setIsPastNode(bool value) { m_isPastNode = value; }

    AtomicString language() const { return m_language; }
    void setLanguage(const AtomicString& value) { m_language = value; }

    static const QualifiedName& voiceAttributeName()
    {
        DEPRECATED_DEFINE_STATIC_LOCAL(QualifiedName, voiceAttr, (nullAtom, "voice", nullAtom));
        return voiceAttr;
    }
    
    static const QualifiedName& langAttributeName()
    {
        DEPRECATED_DEFINE_STATIC_LOCAL(QualifiedName, voiceAttr, (nullAtom, "lang", nullAtom));
        return voiceAttr;
    }

private:
    WebVTTElement(WebVTTNodeType, Document&);

    virtual bool isWebVTTElement() const override { return true; }

    unsigned m_isPastNode : 1;
    unsigned m_webVTTNodeType : 4;
    
    AtomicString m_language;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::WebVTTElement)
    static bool isType(const WebCore::Node& node) { return node.isWebVTTElement(); }
SPECIALIZE_TYPE_TRAITS_END()

#endif
