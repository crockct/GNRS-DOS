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

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * @author Robert Moore
 */
public class SimpleGUIDStoreTest {

  public SimpleGUIDStore store;

  @Before
  public void prepStore() {
    this.store = new SimpleGUIDStore();
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.SimpleGUIDStore#appendBindings(edu.rutgers.winlab.mfirst.GUID, edu.rutgers.winlab.mfirst.storage.GUIDBinding[])}
   * .
   */
  @Test
  public void testAppendBindings() {
    try {
      NetworkAddress addr1 = new NetworkAddress(AddressType.INET_4_UDP,
          new byte[] { 0, 0, 0, 0, 0, 0 });
      NetworkAddress addr2 = IPv4UDPAddress.fromInteger(7);

      GUID guid1 = GUID.fromASCII("1234");
      GUID guid2 = GUID.fromASCII("2345");

      this.store.appendBindings(guid1, (GUIDBinding[])null);

      GUIDBinding bind1 = new GUIDBinding();
      bind1.setAddress(addr1);
      bind1.setTtl(0l);
      bind1.setWeight(0);

      GUIDBinding bind2 = new GUIDBinding();
      bind2.setAddress(addr2);
      bind2.setWeight(1);
      bind2.setTtl(System.currentTimeMillis());

      Assert.assertTrue(this.store.appendBindings(guid1, bind1));

      GNRSRecord record = this.store.getBindings(guid1);
      Assert.assertTrue(guid1.equals(record.getGuid()));
      GUIDBinding[] bound1 = record.getBindings();
      Assert.assertTrue(bound1.length == 1);
      Assert.assertTrue(addr1.equals(bound1[0].getAddress()));

      Assert.assertTrue(this.store.appendBindings(guid1, bind1, bind2));
      Assert.assertTrue(this.store.appendBindings(guid2, bind2, bind1));

      record = this.store.getBindings(guid1);
      Assert.assertTrue(guid1.equals(record.getGuid()));
      bound1 = record.getBindings();
      Assert.assertTrue(bound1.length == 2);
      Assert.assertTrue( (addr1.equals(bound1[0].getAddress()) && addr2.equals(bound1[1].getAddress() ))
          || (addr2.equals(bound1[0].getAddress()) && addr1.equals(bound1[1].getAddress())));
      Assert.assertTrue(addr2.equals(bound1[1].getAddress()));

      record = this.store.getBindings(guid2);
      Assert.assertTrue(guid2.equals(record.getGuid()));
      GUIDBinding[] bound2 = record.getBindings();
      Assert.assertTrue(bound2.length == 2);
      Assert.assertTrue((addr1.equals(bound1[0].getAddress()) && addr2.equals(bound1[1].getAddress()))
          || (addr2.equals(bound1[0].getAddress()) && addr1.equals(bound1[1].getAddress())));

    } catch (UnsupportedEncodingException e) {
      Assert.fail();
    }
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.SimpleGUIDStore#replaceBindings(edu.rutgers.winlab.mfirst.GUID, edu.rutgers.winlab.mfirst.storage.GUIDBinding[])}
   * .
   */
  @Test
  public void testReplaceBindings() {
    try {
      NetworkAddress addr1 = new NetworkAddress(AddressType.INET_4_UDP,
          new byte[] { 0, 0, 0, 0, 0, 0 });
      NetworkAddress addr2 = IPv4UDPAddress.fromInteger(7);

      GUID guid1 = GUID.fromASCII("1234");
      GUID guid2 = GUID.fromASCII("2345");

      GUIDBinding bind1 = new GUIDBinding();
      bind1.setAddress(addr1);
      bind1.setTtl(0l);
      bind1.setWeight(0);

      GUIDBinding bind2 = new GUIDBinding();
      bind2.setAddress(addr2);
      bind2.setWeight(1);
      bind2.setTtl(System.currentTimeMillis());
      
      GNRSRecord record1 = this.store.getBindings(guid1);
      Assert.assertNull(record1);
      
      Assert.assertTrue(this.store.replaceBindings(guid1));
      Assert.assertTrue(this.store.replaceBindings(guid1,(GUIDBinding[])null));
      
      Assert.assertTrue(this.store.replaceBindings(guid1, bind1));
      GNRSRecord record = this.store.getBindings(guid1);
      Assert.assertTrue(guid1.equals(record.getGuid()));
      GUIDBinding[] bound1 = record.getBindings();
      Assert.assertTrue(bound1.length == 1);
      Assert.assertTrue(addr1.equals(bound1[0].getAddress()));
      
      Assert.assertTrue(this.store.replaceBindings(guid1,bind2, bind1));
      record = this.store.getBindings(guid1);
      Assert.assertTrue(guid1.equals(record.getGuid()));
      bound1 = record.getBindings();
      Assert.assertTrue(bound1.length == 2);
      Assert.assertTrue((addr1.equals(bound1[0].getAddress()) && addr2.equals(bound1[1].getAddress() ))
          || (addr2.equals(bound1[0].getAddress()) && addr1.equals(bound1[1].getAddress())));
      
    } catch (UnsupportedEncodingException e) {
      Assert.fail();
    }
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.SimpleGUIDStore#doInit()}.
   */
  @Test
  public void testDoInit() {
    this.store.doInit();
    Assert.assertTrue(true);
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.storage.SimpleGUIDStore#doShutdown()}.
   */
  @Test
  public void testDoShutdown() {
    this.store.doShutdown();
    Assert.assertTrue(true);
  }

}
