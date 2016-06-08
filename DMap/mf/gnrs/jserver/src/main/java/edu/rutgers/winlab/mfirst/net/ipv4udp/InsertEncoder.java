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

import java.util.List;

import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolEncoderOutput;
import org.apache.mina.filter.codec.demux.MessageEncoder;

import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Apache MINA message encoder for {@link InsertMessage} objects.
 * 
 * @author Robert Moore
 */
public class InsertEncoder implements MessageEncoder<InsertMessage> {

  @Override
  public void encode(final IoSession session, final InsertMessage message,
      final ProtocolEncoderOutput out) {

    final IoBuffer buff = IoBuffer.allocate(message.getMessageLength());

    // Generic request stuff
    buff.put(message.getVersion());
    buff.put(message.getType().value());
    buff.putUnsignedShort(message.getMessageLength());

    buff.putUnsignedInt(message.getRequestId());

    // Offset values
    int optionsOffset = 0;
    // 12 + address T&L + address length
    int payloadOffset = 16 + message.getOriginAddress().getLength();
    if (!message.getOptions().isEmpty()) {
      optionsOffset = payloadOffset + message.getPayloadLength();
    }
    buff.putUnsignedShort(optionsOffset);
    buff.putUnsignedShort(payloadOffset);

    buff.putUnsignedShort(message.getOriginAddress().getType().value());
    buff.putUnsignedShort(message.getOriginAddress().getLength());
    buff.put(message.getOriginAddress().getValue());

    // Specific for Insert messages
    buff.put(message.getGuid().getBinaryForm());

    buff.putUnsignedInt(message.getNumBindings());
    if (message.getNumBindings() > 0) {
      for (final NetworkAddress addx : message.getBindings()) {
        buff.putUnsignedShort(addx.getType().value());
        buff.putUnsignedShort(addx.getLength());
        buff.put(addx.getValue());
      }
    }

    List<Option> options = message.getOptions();
    if (options != null && !options.isEmpty()) {
      buff.put(RequestOptionsTranscoder.encode(options));
    }

    buff.flip();
    out.write(buff);

  }

}
