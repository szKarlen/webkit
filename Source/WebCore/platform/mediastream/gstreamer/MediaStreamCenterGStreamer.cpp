/*
 * Copyright (C) 2011 Ericsson AB. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Ericsson nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(MEDIA_STREAM)

#include "MediaStreamCenterGStreamer.h"

#include "MediaStreamCreationClient.h"
#include "MediaStreamPrivate.h"
#include "MediaStreamSourceCapabilities.h"
#include "MediaStreamTrackSourcesRequestClient.h"
#include "NotImplemented.h"
#include <wtf/MainThread.h>

namespace WebCore {

MediaStreamCenter& MediaStreamCenter::platformCenter()
{
    ASSERT(isMainThread());
    DEPRECATED_DEFINE_STATIC_LOCAL(MediaStreamCenterGStreamer, center, ());
    return center;
}

MediaStreamCenterGStreamer::MediaStreamCenterGStreamer()
{
}

MediaStreamCenterGStreamer::~MediaStreamCenterGStreamer()
{
}

void MediaStreamCenterGStreamer::validateRequestConstraints(PassRefPtr<MediaStreamCreationClient> prpQueryClient, PassRefPtr<MediaConstraints>, PassRefPtr<MediaConstraints>)
{
    RefPtr<MediaStreamCreationClient> client = prpQueryClient;
    // FIXME: Actually do constraints validation. The MediaConstraints
    // need to comply with the available audio/video device(s)
    // capabilities. See bug #123345.
    client->constraintsValidated();
}
    
void MediaStreamCenterGStreamer::createMediaStream(PassRefPtr<MediaStreamCreationClient> prpQueryClient, PassRefPtr<MediaConstraints>, PassRefPtr<MediaConstraints>)
{
    RefPtr<MediaStreamCreationClient> client = prpQueryClient;
    ASSERT(client);

    Vector<RefPtr<MediaStreamSource>> audioSources;
    Vector<RefPtr<MediaStreamSource>> videoSources;
    // FIXME: fill source vectors, see bug #110150.
    client->didCreateStream(MediaStreamPrivate::create(audioSources, videoSources));

}

bool MediaStreamCenterGStreamer::getMediaStreamTrackSources(PassRefPtr<MediaStreamTrackSourcesRequestClient>)
{
    notImplemented();
    return false;
}

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)
