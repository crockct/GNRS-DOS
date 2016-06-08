/*
 * Copyright (c) 2012, Rutgers University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * + Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * + Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
package edu.rutgers.winlab.mfirst.net.ipv4udp;

import edu.rutgers.winlab.mfirst.messages.*;
import org.apache.mina.filter.codec.demux.DemuxingProtocolCodecFactory;

/**
 * Protocol codec factory class for Apache MINA.
 * <p>
 * Creates a protocol codec for GNRS server/client applications using the Apache
 * MINA networking library.
 * </p>
 * 
 * @author Robert Moore
 */
public class GNRSProtocolCodecFactory extends DemuxingProtocolCodecFactory {
  /**
   * Generates a new protocol codec factory for a server or client using the
   * GNRS protocol.
   * 
   * @param isServer
   *          {@code true} if the factory should be for a server
   */
  public GNRSProtocolCodecFactory() {
    super();

    super.addMessageDecoder(InsertDecoder.class);
    super.addMessageDecoder(LookupDecoder.class);
    super.addMessageDecoder(UpdateDecoder.class);
    super.addMessageDecoder(LookupResponseDecoder.class);
    super.addMessageDecoder(InsertResponseDecoder.class);
    super.addMessageDecoder(UpdateResponseDecoder.class);

    super.addMessageEncoder(InsertMessage.class, InsertEncoder.class);
    super.addMessageEncoder(LookupMessage.class, LookupEncoder.class);
    super.addMessageEncoder(UpdateMessage.class, UpdateEncoder.class);
    super.addMessageEncoder(LookupResponseMessage.class,
        LookupResponseEncoder.class);
    super.addMessageEncoder(InsertResponseMessage.class,
        InsertResponseEncoder.class);
    super.addMessageEncoder(UpdateResponseMessage.class,
            UpdateResponseEncoder.class);

  }
}
