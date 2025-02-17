/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2009 Apple Inc. All rights reserved.
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

#ifndef Comment_h
#define Comment_h

#include "CharacterData.h"

namespace WebCore {

class Comment final : public CharacterData {
public:
    static Ref<Comment> create(Document&, const String&);
    static Ref<Comment> create(ScriptExecutionContext&, const String&);

private:
    Comment(Document&, const String&);

    virtual String nodeName() const override;
    virtual NodeType nodeType() const override;
    virtual RefPtr<Node> cloneNodeInternal(Document&, CloningOperation) override;
    virtual bool childTypeAllowed(NodeType) const override;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::Comment)
    static bool isType(const WebCore::Node& node) { return node.nodeType() == WebCore::Node::COMMENT_NODE; }
SPECIALIZE_TYPE_TRAITS_END()

#endif // Comment_h
