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
package edu.rutgers.winlab.mfirst.messages;

import static org.junit.Assert.*;

import java.io.UnsupportedEncodingException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * @author Robert Moore
 */
public class InsertMessageTest {

  public GUID guid1;
  public GUID nullGuid;
  public NetworkAddress addr1;
  public NetworkAddress addr2;
  public NetworkAddress addr3;
  public NetworkAddress[] bind1;

  public static final byte[] GUID_BYTE1 = new byte[] { 0x0, 0x1, 0x2, 0x3, 0x4,
      0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12,
      0x13 };

  public static final String STRING_INIT = "INS #0 null -> {}";
  public static final String STRING_TEST = "INS #1234 GUID(000102030405060708090a0b0c0d0e0f10111213) -> {127.0.0.1:55, 127.0.0.1:56, 127.0.0.1:57}";

  @Before
  public void prepTest() {
    this.guid1 = new GUID();
    this.guid1.setBinaryForm(GUID_BYTE1);

    this.nullGuid = new GUID();
    this.nullGuid.setBinaryForm(null);

    try {
      this.addr1 = IPv4UDPAddress.fromASCII("127.0.0.1:55");
      this.addr2 = IPv4UDPAddress.fromASCII("127.0.0.1:56");
      this.addr3 = IPv4UDPAddress.fromASCII("127.0.0.1:57");
    } catch (UnsupportedEncodingException e) {
      fail("No text encoding available?");
    }

    this.bind1 = new NetworkAddress[] { this.addr1, this.addr2, this.addr3 };
  }

  @Test
  public void testFields() {
    InsertMessage msg = new InsertMessage();

    Assert.assertNull(msg.getGuid());
    Assert.assertNull(msg.getOriginAddress());
    Assert.assertEquals(0, msg.getRequestId());
    Assert.assertEquals(0, msg.getNumBindings());
    Assert.assertEquals(0, msg.getBindingsLength());
    Assert.assertEquals(4, msg.getPayloadLength());
    Assert.assertEquals(16, msg.getMessageLength());
    Assert.assertEquals(0, msg.getVersion());

    msg.setGuid(this.guid1);

    msg.setOriginAddress(this.addr1);
    msg.setRequestId(1234);
    msg.setBindings(this.bind1);
    msg.setVersion((byte) 0x10);

    Assert.assertTrue(this.guid1.equals(msg.getGuid()));

    Assert.assertTrue(this.addr1.equals(msg.getOriginAddress()));
    Assert.assertEquals(1234, msg.getRequestId());
    Assert.assertEquals(this.bind1.length, msg.getNumBindings());
    Assert.assertEquals(30, msg.getBindingsLength());
    Assert.assertEquals(54, msg.getPayloadLength());
    Assert.assertEquals(76, msg.getMessageLength());
    Assert.assertEquals(0x10, msg.getVersion());

    msg.setType(MessageType.UNKNOWN);
    Assert.assertTrue(MessageType.UNKNOWN.equals(msg.getType()));

    msg.setGuid(this.nullGuid);
    Assert.assertTrue(this.nullGuid.equals(msg.getGuid()));

    msg.setBindings(null);
    Assert.assertNull(msg.getBindings());
    Assert.assertEquals(0, msg.getNumBindings());

    Assert.assertEquals(0, msg.getBindingsLength());
    Assert.assertEquals(4, msg.getPayloadLength());
  }

  @Test
  public void testToString() {
    InsertMessage msg = new InsertMessage();
    Assert.assertEquals(STRING_INIT, msg.toString());

    msg.setGuid(this.guid1);

    msg.setOriginAddress(this.addr1);
    msg.setRequestId(1234);
    msg.setBindings(this.bind1);
    Assert.assertEquals(STRING_TEST, msg.toString());
  }

}
