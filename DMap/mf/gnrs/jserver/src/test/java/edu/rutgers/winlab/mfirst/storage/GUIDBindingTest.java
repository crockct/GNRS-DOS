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
package edu.rutgers.winlab.mfirst.storage;

import static org.junit.Assert.*;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import org.junit.Assert;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * @author Robert Moore
 */
public class GUIDBindingTest {

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.GUIDBinding#hashCode()}.
   */
  @Test
  public void testHashCode() {
    try {
      GUIDBinding bind1 = new GUIDBinding();
      NetworkAddress addr1 = IPv4UDPAddress.fromASCII("1.2.3.4:4444");
      bind1.setAddress(addr1);
      bind1.setTtl(0xFFFFFFFFl);
      bind1.setWeight(0xFFFF);

      GUIDBinding bind2 = new GUIDBinding();
      NetworkAddress addr2 = IPv4UDPAddress.fromASCII("5.4.3.2:9191");
      bind2.setAddress(addr2);
      bind2.setTtl(4);
      bind2.setWeight(8);

      Assert.assertTrue(addr1.equals(bind1.getAddress()));
      Assert.assertEquals(bind1.getTtl(), 0xFFFFFFFFl);
      Assert.assertEquals(bind1.getWeight(), 0xFFFF);

      Assert.assertFalse(bind1.equals("1234"));
      Assert.assertFalse(bind1.equals(bind2));
      Assert.assertEquals(bind1.hashCode(), Arrays.hashCode(addr1.getValue()));

    } catch (UnsupportedEncodingException e) {
      Assert.fail("Unable to encode network addresses.");
    }

  }

  @Test
  public void testNullHandling() {
    GUIDBinding null1 = new GUIDBinding();
    GUIDBinding null2 = new GUIDBinding();

    Assert.assertTrue(null1.equals(null2));

    null2.setTtl(5);
    Assert.assertTrue(null1.equals(null2));
    Assert.assertTrue(null2.equals(null1));

    null2.setWeight(9);
    Assert.assertTrue(null1.equals(null2));
    Assert.assertTrue(null2.equals(null1));

    null2.setAddress(new NetworkAddress(null, null));
    Assert.assertFalse(null1.equals(null2));
    Assert.assertFalse(null2.equals(null1));
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.GUIDBinding#toString()}.
   */
  @Test
  public void testToString() {
    try {
      GUIDBinding bind1 = new GUIDBinding();
      NetworkAddress addr1 = IPv4UDPAddress.fromASCII("1.2.3.4:4444");
      bind1.setAddress(addr1);
      bind1.setTtl(5);
      bind1.setWeight(2);

      GUIDBinding bind2 = new GUIDBinding();
      NetworkAddress addr2 = IPv4UDPAddress.fromASCII("5.4.3.2:9191");
      bind2.setAddress(addr2);
      bind2.setTtl(4);
      bind2.setWeight(8);

      Assert.assertTrue("Bind (1.2.3.4:4444, T5/E0, 2)".equals(bind1.toString()));
      Assert.assertTrue("Bind (5.4.3.2:9191, T4/E0, 8)".equals(bind2.toString()));

    } catch (UnsupportedEncodingException e) {
      Assert.fail("Unable to encode network addresses.");
    }
  }

  @Test
  public void testEquals() {
    
    GUIDBinding bind1 = new GUIDBinding();
    NetworkAddress addr1 = IPv4UDPAddress.fromInteger(1);
    bind1.setAddress(addr1);
    bind1.setTtl(5);
    bind1.setWeight(2);
    
    GUIDBinding bind2 = new GUIDBinding();
    NetworkAddress addr2 = IPv4UDPAddress.fromInteger(2);
    bind2.setAddress(addr2);
    bind2.setTtl(4);
    bind2.setWeight(8);
    
    Assert.assertTrue(bind1.equals(bind1));
    Assert.assertFalse(bind1.equals(bind2));
    Assert.assertFalse(bind2.equals(bind1));
    
    Assert.assertFalse(bind1.equals(bind1.toString()));
  }

}
