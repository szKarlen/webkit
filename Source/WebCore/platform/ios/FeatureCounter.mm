/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#import "config.h"
#import "FeatureCounter.h"

#if PLATFORM(IOS) && USE(APPLE_INTERNAL_SDK)
#import <AppSupport/CPAggregateDictionary.h>

namespace WebCore {

void FeatureCounter::incrementKey(Page* page, const char* const key)
{
    if (!shouldUseForPage(page))
        return;

    NSString *nsKey = [[NSString alloc] initWithCharactersNoCopy:reinterpret_cast<unichar*>(const_cast<char*>(key)) length:strlen(key) freeWhenDone:NO];
    [[CPAggregateDictionary sharedAggregateDictionary] incrementKey:nsKey];
}

void FeatureCounter::setKey(Page* page, const char* const key, int64_t value)
{
    if (!shouldUseForPage(page))
        return;

    NSString *nsKey = [[NSString alloc] initWithCharactersNoCopy:reinterpret_cast<unichar*>(const_cast<char*>(key)) length:strlen(key) freeWhenDone:NO];
    [[CPAggregateDictionary sharedAggregateDictionary] setValue:value forScalarKey:nsKey];
}

} // namespace WebCore

#endif // PLATFORM(IOS) && USE(APPLE_INTERNAL_SDK)

