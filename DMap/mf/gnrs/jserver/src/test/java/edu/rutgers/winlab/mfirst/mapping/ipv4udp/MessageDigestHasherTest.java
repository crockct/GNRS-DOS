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
package edu.rutgers.winlab.mfirst.mapping.ipv4udp;

import static org.junit.Assert.*;

import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * @author Robert Moore
 */
public class MessageDigestHasherTest {

  public GUID guid;
  public static final byte[] GUID_BYTE = new byte[] { 0x0, 0x1, 0x2, 0x3, 0x4,
      0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12,
      0x13 };

  public static final byte[] MD2_HASH_GUID = new byte[] { (byte) 0x87, 0x33,
      0x30, (byte) 0xe9, (byte) 0xf9, 0x40, (byte) 0xb9, 0x65, (byte) 0x9d,
      (byte) 0xb4, (byte) 0x8d, (byte) 0xfe, 0x4a, (byte) 0xd7, (byte) 0xc9,
      (byte) 0x96, 0x75, 0x63, (byte) 0xe4, 0x0f };

  public static final byte[] MD5_HASH_GUID = new byte[] { 0x71, (byte) 0xbe,
      0x22, 0x03, (byte) 0xe5, 0x22, (byte) 0x91, 0x6b, (byte) 0xdf, 0x05,
      (byte) 0xf6, 0x24, 0x73, 0x4f, (byte) 0x81, (byte) 0x88, (byte) 0xcf,
      (byte) 0xb0, 0x4c, 0x47 };

  public static final byte[] SHA1_HASH_GUID = new byte[] { (byte) 0xea,
      (byte) 0x94, (byte) 0xad, (byte) 0xed, 0x01, (byte) 0xd8, (byte) 0xe4,
      0x58, (byte) 0xc0, 0x77, (byte) 0xf6, (byte) 0xc7, (byte) 0xb4,
      (byte) 0x83, 0x45, 0x7c, 0x62, (byte) 0xcc, 0x5a, 0x4d };

  public static final byte[] SHA256_HASH_GUID = new byte[] { (byte) 0xc6, 0x0b,
      (byte) 0xcb, (byte) 0xae, 0x58, (byte) 0xb0, 0x08, (byte) 0xa7, 0x7d,
      0x69, 0x16, 0x74, (byte) 0xff, (byte) 0xf8, (byte) 0x8f, 0x5b,
      (byte) 0xe8, 0x3c, 0x57, 0x32 };

  public static final byte[] SHA384_HASH_GUID = new byte[] { 0x34, (byte) 0xf8, 0x71,
 0x03, 0x32, 0x2a, 0x59, 0x2f, 0x17, (byte) 0xf4, 0x0c, (byte) 0xb2, (byte) 0xe9, (byte) 0x98, (byte) 0xf5,
      (byte) 0xd5, (byte) 0x96, (byte) 0xb2, 0x57, 0x66 };

  public static final byte[] SHA512_HASH_GUID = new byte[] { (byte) 0x8f, 0x37,
      0x48, (byte) 0xeb, (byte) 0x8e, (byte) 0xfa, 0x63, (byte) 0xf3, 0x46,
      0x05, 0x58, 0x3d, (byte) 0xa7, (byte) 0x86, (byte) 0xb6, 0x6b, 0x6f,
      0x28, 0x70, (byte) 0xf8 };

  @Before
  public void setup() {
    this.guid = new GUID();
    this.guid.setBinaryForm(GUID_BYTE);
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.MessageDigestHasher#MessageDigestHasher(java.lang.String)}
   * .
   */
  @Test
  public void testMessageDigestHasher() {
    MessageDigestHasher hash = new MessageDigestHasher("MD5");

    hash = new MessageDigestHasher("SHA-1");
    hash = new MessageDigestHasher("SHA-256");
    hash = new MessageDigestHasher("SHA-512");
  }

  @Test
  public void testBadAlgorithm() {
    MessageDigestHasher hasher = new MessageDigestHasher("FOOBAR");
    Collection<NetworkAddress> addresses = hasher.hash(guid, AddressType.GUID,
        1);
    Assert.assertNotNull(addresses);
    Assert.assertTrue(addresses.isEmpty());
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.MessageDigestHasher#hash(edu.rutgers.winlab.mfirst.GUID, edu.rutgers.winlab.mfirst.net.AddressType, int)}
   * .
   */
  @Test
  public void testHash() {

    MessageDigestHasher hasher = new MessageDigestHasher("MD5");
    Collection<NetworkAddress> addresses = hasher.hash(this.guid,
        AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    Iterator<NetworkAddress> iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    NetworkAddress netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(MD5_HASH_GUID, netAddr.getValue()));

    hasher = new MessageDigestHasher("MD2");
    addresses = hasher.hash(this.guid, AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(MD2_HASH_GUID, netAddr.getValue()));

    hasher = new MessageDigestHasher("SHA-1");
    addresses = hasher.hash(this.guid, AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(SHA1_HASH_GUID, netAddr.getValue()));

    hasher = new MessageDigestHasher("SHA-256");
    addresses = hasher.hash(this.guid, AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(SHA256_HASH_GUID, netAddr.getValue()));

    hasher = new MessageDigestHasher("SHA-384");
    addresses = hasher.hash(this.guid, AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(SHA384_HASH_GUID, netAddr.getValue()));

    hasher = new MessageDigestHasher("SHA-512");
    addresses = hasher.hash(this.guid, AddressType.GUID, 1);
    Assert.assertEquals(1, addresses.size());
    iter = addresses.iterator();
    Assert.assertTrue(iter.hasNext());
    netAddr = iter.next();
    Assert.assertFalse(iter.hasNext());
    Assert.assertEquals(AddressType.GUID, netAddr.getType());
    Assert.assertEquals(AddressType.GUID.getMaxLength(), netAddr.getLength());
    Assert.assertTrue(Arrays.equals(SHA512_HASH_GUID, netAddr.getValue()));

  }

}
