/*
 * Copyright (C) 2006, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2009 Google, Inc. All rights reserved.
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

#ifndef BackForwardList_h
#define BackForwardList_h

#include "BackForwardClient.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

namespace WebCore {

class Page;

typedef Vector<RefPtr<HistoryItem>> HistoryItemVector;
typedef HashSet<RefPtr<HistoryItem>> HistoryItemHashSet;

class BackForwardList : public BackForwardClient {
public: 
    static Ref<BackForwardList> create(Page* page) { return adoptRef(*new BackForwardList(page)); }
    virtual ~BackForwardList();

    Page* page() { return m_page; }

    virtual void addItem(PassRefPtr<HistoryItem>) override;
    WEBCORE_EXPORT void goBack();
    WEBCORE_EXPORT void goForward();
    virtual void goToItem(HistoryItem*) override;
        
    WEBCORE_EXPORT HistoryItem* backItem();
    WEBCORE_EXPORT HistoryItem* currentItem();
    WEBCORE_EXPORT HistoryItem* forwardItem();
    virtual HistoryItem* itemAtIndex(int) override;

    WEBCORE_EXPORT void backListWithLimit(int, HistoryItemVector&);
    WEBCORE_EXPORT void forwardListWithLimit(int, HistoryItemVector&);

    WEBCORE_EXPORT int capacity();
    WEBCORE_EXPORT void setCapacity(int);
    WEBCORE_EXPORT bool enabled();
    WEBCORE_EXPORT void setEnabled(bool);
    virtual int backListCount() override;
    virtual int forwardListCount() override;
    WEBCORE_EXPORT bool containsItem(HistoryItem*);

    virtual void close() override;
    WEBCORE_EXPORT bool closed();

    WEBCORE_EXPORT void removeItem(HistoryItem*);
    WEBCORE_EXPORT HistoryItemVector& entries();

#if PLATFORM(IOS)
    virtual unsigned current() override;
    virtual void setCurrent(unsigned newCurrent) override;

    virtual bool clearAllPageCaches() override;
#endif

private:
    WEBCORE_EXPORT explicit BackForwardList(Page*);

    Page* m_page;
    HistoryItemVector m_entries;
    HistoryItemHashSet m_entryHash;
    unsigned m_current;
    unsigned m_capacity;
    bool m_closed;
    bool m_enabled;
};
    
} // namespace WebCore

#endif // BackForwardList_h
