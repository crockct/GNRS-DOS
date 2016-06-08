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
package edu.rutgers.winlab.mfirst.net;

import static org.junit.Assert.*;

import java.util.Arrays;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Robert Moore
 */
public class NetworkAddressTest {

  public static final byte[] ADDRESS_BYTES_1 = new byte[] { 0x0, 0x1, 0x2, 0x3,
      0x4, 0x1 };
  public static final byte[] ADDRESS_BYTES_2 = new byte[] { 0x0, 0x1, 0x2, 0x3,
      0x4, 0x2 };
  public static final byte[] ADDRESS_BYTES_3 = new byte[] { 0x0, 0x1, 0x2, 0x3 };

  public static final byte[] ADDRESS_BYTES_4 = new byte[] { 0x0, 0x1, 0x2, 0x3,
      0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11,
      0x12, 0x13 };

  public static final String NA1_STRING = "NA(000102030401)";
  public static final String NA2_STRING = "NA(000102030402)";

  public NetworkAddress netAddr1;
  public NetworkAddress netAddr1Copy;
  public NetworkAddress netAddr2;
  public NetworkAddress netAddr3;
  public NetworkAddress netAddr4;

  @Before
  public void setupAddresses() {
    netAddr1 = new NetworkAddress(AddressType.INET_4_UDP, ADDRESS_BYTES_1);
    netAddr1Copy = new NetworkAddress(AddressType.INET_4_UDP, ADDRESS_BYTES_1);
    netAddr2 = new NetworkAddress(AddressType.INET_4_UDP, ADDRESS_BYTES_2);
    netAddr3 = new NetworkAddress(AddressType.INET_4_UDP, ADDRESS_BYTES_3);
    netAddr4 = new NetworkAddress(AddressType.GUID, ADDRESS_BYTES_4);
  }

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.NetworkAddress}.
   */
  @Test
  public void testHashCode() {

    Assert.assertEquals(Arrays.hashCode(ADDRESS_BYTES_1), netAddr1.hashCode());
    Assert.assertEquals(netAddr1.hashCode(), netAddr1Copy.hashCode());
    Assert.assertEquals(Arrays.hashCode(ADDRESS_BYTES_2), netAddr2.hashCode());
    Assert.assertFalse(netAddr1.hashCode() == netAddr2.hashCode());

  }

  @Test
  public void testEquality() {
    Assert.assertTrue(netAddr1.equals(netAddr1Copy));
    Assert.assertTrue(netAddr1Copy.equals(netAddr1));

    Assert.assertFalse(netAddr1.equals(netAddr2));
    Assert.assertFalse(netAddr2.equals(netAddr1));
    Assert.assertFalse(netAddr1.equals(ADDRESS_BYTES_1));
    
    Assert.assertFalse(netAddr1.equals(netAddr4));
  }

  @Test
  public void testLength() {
    Assert.assertEquals(netAddr1.getLength(), ADDRESS_BYTES_1.length);
    Assert.assertEquals(netAddr3.getLength(), ADDRESS_BYTES_3.length);
  }

  @Test
  public void testValue() {

    NetworkAddress nullValue = new NetworkAddress(AddressType.INET_4_UDP, null);
    Assert.assertNull(nullValue.getValue());
    Assert.assertEquals(0, nullValue.getLength());
    nullValue.setValue(null);
    Assert.assertNull(nullValue.getValue());
    Assert.assertEquals(0, nullValue.getLength());

    Assert.assertTrue(AddressType.INET_4_UDP.equals(netAddr1.getType()));

    Assert.assertTrue(Arrays.equals(netAddr1.getValue(),
        netAddr1Copy.getValue()));

    Assert.assertFalse(Arrays.equals(netAddr1.getValue(), netAddr2.getValue()));

    Assert.assertTrue(Arrays.equals(ADDRESS_BYTES_3, netAddr3.getValue()));
  }

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.NetworkAddress}.
   */
  @Test(expected = IllegalArgumentException.class)
  public void testIllegalConstrLength() {
    NetworkAddress netAddr = new NetworkAddress(AddressType.INET_4_UDP,
        new byte[] { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 });
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.net.NetworkAddress}.
   */
  @Test(expected = IllegalArgumentException.class)
  public void testIllegalSetValueLength() {
    NetworkAddress netAddr = new NetworkAddress(AddressType.INET_4_UDP,
        ADDRESS_BYTES_1);
    netAddr.setValue(new byte[65536]);
  }

  @Test
  public void testToString() {
    Assert.assertEquals(NA1_STRING, this.netAddr1.toString());
    Assert.assertEquals(NA2_STRING, this.netAddr2.toString());
    Assert.assertFalse(NA2_STRING.equals(this.netAddr1.toString()));
  }
  
  @Test
  public void testNullHandling(){
    NetworkAddress null1 = new NetworkAddress(null,null);
    NetworkAddress null2 = new NetworkAddress(null,null);
    
    Assert.assertTrue(null1.equals(null2));
    null2.setType(AddressType.GUID);
    Assert.assertFalse(null1.equals(null2));
    Assert.assertFalse(null2.equals(null1));
  }

}
