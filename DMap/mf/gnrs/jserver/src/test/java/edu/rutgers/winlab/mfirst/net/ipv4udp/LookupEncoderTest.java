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

import static org.junit.Assert.*;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.DummySession;
import org.apache.mina.filter.codec.ProtocolDecoderOutput;
import org.junit.Assert;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.messages.LookupMessage;

/**
 * @author Robert Moore
 */
public class LookupEncoderTest {

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.net.ipv4udp.LookupEncoder#encode(org.apache.mina.core.session.IoSession, edu.rutgers.winlab.mfirst.messages.LookupMessage, org.apache.mina.filter.codec.ProtocolEncoderOutput)}
   * .
   */
  @Test
  public void testEncode() {
    try {
      byte[] shouldBytes = new byte[] { 16, 1, 0, 42, 0, 0, 4, -46, 0, 0, 0,
          22, 0, 0, 0, 6, 1, 2, 3, 4, 19, -120, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
          10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

      DummySession session = new DummySession();
      ProtocolEncoderTest out = new ProtocolEncoderTest();
      LookupEncoder encoder = new LookupEncoder();
      LookupMessage message = new LookupMessage();

      GUID guid = new GUID();
      byte[] guidBytes = new byte[] { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
          0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13 };
      guid.setBinaryForm(guidBytes);
      message.setGuid(guid);

      IPv4UDPAddress origin = IPv4UDPAddress.fromASCII("1.2.3.4:5000");
      message.setOriginAddress(origin);
      message.setRequestId(1234);
      message.setVersion((byte) 0x10);

      encoder.encode(session, message, out);

      IoBuffer buff = (IoBuffer) out.next();
      byte[] bytes = buff.array();
      Assert.assertTrue(Arrays.equals(shouldBytes, bytes));

    } catch (UnsupportedEncodingException uee) {
      fail("Unable to decode ASCII");
    }
  }

}
