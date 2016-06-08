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
import org.apache.mina.filter.codec.ProtocolDecoderOutput;
import org.apache.mina.filter.codec.demux.MessageDecoder;
import org.apache.mina.filter.codec.demux.MessageDecoderResult;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.messages.MessageType;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Apache MINA message decoder for InsertMessage objects.
 * 
 * @author Robert Moore
 */
public class InsertDecoder implements MessageDecoder {

  @Override
  public MessageDecoderResult decodable(final IoSession session,
      final IoBuffer buffer) {
    MessageDecoderResult result;
    // Store the current cursor position in the buffer
    buffer.mark();
    // Need 2 bytes to check version and type
    if (buffer.remaining() < 4) {
      buffer.reset();
      result = MessageDecoderResult.NEED_DATA;
    } else {

      // Skip the version number
      // TODO: What to do with the version?
      buffer.get();
      final byte type = buffer.get();
      final int needRemaining = buffer.getUnsignedShort() - 4;
      // Reset the cursor so we don't modify the buffer data.
      buffer.reset();
      if (type == MessageType.INSERT.value())
        if (buffer.remaining() >= needRemaining) {
          result = MessageDecoderResult.OK;
        } else {
          result = MessageDecoderResult.NEED_DATA;
        }
      else {
        result = MessageDecoderResult.NOT_OK;
      }
    }
    return result;
  }

  /*
   * (non-Javadoc)
   * 
   * @see
   * org.apache.mina.filter.codec.demux.MessageDecoder#decode(org.apache.mina
   * .core.session.IoSession, org.apache.mina.core.buffer.IoBuffer,
   * org.apache.mina.filter.codec.ProtocolDecoderOutput)
   */
  @Override
  public MessageDecoderResult decode(final IoSession session,
      final IoBuffer buffer, final ProtocolDecoderOutput out) {
    /*
     * Common message header stuff
     */
    final byte version = buffer.get();
    // Ignore type, since this is checked in decodable(IoSession, IoBuffer)
    buffer.get();
    // Ignore message length (not used)
    final int length = buffer.getUnsignedShort();
    final long requestId = buffer.getUnsignedInt();

    // Offset values
    final int optionsOffset = buffer.getUnsignedShort();
    final int payloadOffset = buffer.getUnsignedShort();

    // Origin address
    final AddressType addrType = AddressType.valueOf(buffer.getUnsignedShort());

    final int originAddrLength = buffer.getUnsignedShort();
    final byte[] originAddr = new byte[originAddrLength];
    buffer.get(originAddr);
    final NetworkAddress originAddress = new NetworkAddress(addrType,
        originAddr);

    final InsertMessage msg = new InsertMessage();
    msg.setVersion(version);
    msg.setRequestId(requestId);
    msg.setOriginAddress(originAddress);

    // Insert-specific stuff
    final GUID guid = new GUID();
    final byte[] guidBytes = new byte[GUID.SIZE_OF_GUID];
    buffer.get(guidBytes);
    guid.setBinaryForm(guidBytes);
    msg.setGuid(guid);

    final long numBindings = buffer.getUnsignedInt();
    final NetworkAddress[] bindings = new NetworkAddress[(int) numBindings];

    for (int i = 0; i < numBindings; ++i) {
      final int addxType = buffer.getUnsignedShort();
      final int addxLength = buffer.getUnsignedShort();
      final byte[] addxBytes = new byte[addxLength];
      buffer.get(addxBytes);

      final NetworkAddress netAddr = new NetworkAddress(
          AddressType.valueOf(addxType), addxBytes);
      bindings[i] = netAddr;
    }
    msg.setBindings(bindings);
    
    if(optionsOffset > 0){
      List<Option> options = RequestOptionsTranscoder.decode(buffer,length-optionsOffset);
      if(options != null && !options.isEmpty()){
        for(Option opt : options){
          msg.addOption(opt);
        }
      }
    }

    // Send the decoded message to the next filter
    out.write(msg);

    // Everything OK!
    return MessageDecoderResult.OK;
  }

  @Override
  public void finishDecode(final IoSession arg0,
      final ProtocolDecoderOutput arg1) {
    // Nothing to do
  }

}
