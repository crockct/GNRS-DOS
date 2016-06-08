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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Random;
import java.util.TreeMap;
import java.util.concurrent.ConcurrentHashMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.MethodRule;
import org.junit.rules.TestWatchman;
import org.junit.runners.model.FrameworkMethod;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.NetworkAddressMapper;

/**
 * @author Robert Moore
 */
public class IPv4UDPGUIDMapperTest {

  static final Logger LOG = LoggerFactory
      .getLogger(IPv4UDPGUIDMapperTest.class);

  public GUID guid;
  public static final byte[] GUID_BYTE = new byte[] { 0x0, 0x1, 0x2, 0x3, 0x4,
      0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12,
      0x13 };

  public NetworkAddress ipv4Addr5;
  public NetworkAddress guidAddr;

  @Rule
  public MethodRule watchman = new TestWatchman() {
    @Override
    public void starting(FrameworkMethod method) {
      LOG.info("{} being run...", method.getName());
      System.out.println(method.getName() + " being run...");
    }
  };

  @Before
  public void setup() {
    this.guid = new GUID();
    this.guid.setBinaryForm(GUID_BYTE);

    try {
      this.ipv4Addr5 = IPv4UDPAddress.fromASCII("127.0.0.1:5005");
    } catch (UnsupportedEncodingException e) {
      fail("Unable to decode from ASCII.");
    }
    this.guidAddr = new NetworkAddress(AddressType.GUID, new byte[] {});

  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.IPv4UDPGUIDMapper#IPv4UDPGUIDMapper(java.lang.String)}
   * .
   */
  @Test
  public void testBadHashingAlgorithm() {
    try {
      IPv4UDPGUIDMapper mapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-bad-ipv4.xml");
      Collection<NetworkAddress> nullList = mapper.getMapping(guid, 1,
          AddressType.INET_4_UDP);
      Assert.assertNull(nullList);
      // Assert.assertTrue(nullList.isEmpty());
    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.IPv4UDPGUIDMapper#getMapping(edu.rutgers.winlab.mfirst.GUID, int, edu.rutgers.winlab.mfirst.net.AddressType[])}
   * .
   */
  @Test
  public void testGetMapping() {
    try {
      IPv4UDPGUIDMapper mapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-ipv4.xml");
      Collection<NetworkAddress> mapped = mapper.getMapping(guid, 1,
          AddressType.INET_4_UDP);
      Assert.assertNotNull(mapped);
      Iterator<NetworkAddress> iter = mapped.iterator();
      Assert.assertEquals(1, mapped.size());
      Assert.assertTrue(iter.hasNext());
      NetworkAddress netAddr = iter.next();
      Assert.assertFalse(iter.hasNext());
      Assert.assertTrue(this.ipv4Addr5.equals(netAddr));

      mapped = mapper.getMapping(guid, 1);
      Assert.assertNotNull(mapped);
      iter = mapped.iterator();
      Assert.assertEquals(1, mapped.size());
      Assert.assertTrue(iter.hasNext());
      netAddr = iter.next();
      Assert.assertFalse(iter.hasNext());
      Assert.assertTrue(this.ipv4Addr5.equals(netAddr));

      mapped = mapper.getMapping(guid, 1, null);
      Assert.assertNotNull(mapped);
      iter = mapped.iterator();
      Assert.assertEquals(1, mapped.size());
      Assert.assertTrue(iter.hasNext());
      netAddr = iter.next();
      Assert.assertFalse(iter.hasNext());
      Assert.assertTrue(this.ipv4Addr5.equals(netAddr));

      mapped = mapper.getMapping(guid, 1, AddressType.GUID);
      Assert.assertNull(mapped);

      // Assert.assertTrue(nullList.isEmpty());
    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  /**
   * Test method that ensures {@code String} to {@code byte[]} conversions are
   * handled correctly.
   */
  @Test
  public void testLoadPrefixes() {
    /* Prefixes from file
    1.2.3.0/24 1
    1.2.3.255/25 2
    1.2.3.255/26 3
    1.0.0.0/8 4
    192.168.231.44/13 5
    */
    LOG.info("Testing prefixes.");
    try {
      IPv4UDPGUIDMapper guidMapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-ipv4.xml");
      NetworkAddressMapper naMapper = guidMapper.networkAddressMap;
      // 1.2.3.0/24 1
      NetworkAddress addr = IPv4UDPAddress
          .fromInetSocketAddress(new InetSocketAddress("1.2.3.4", 123));
      Assert.assertEquals("1", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.0", 777));
      Assert.assertEquals("1", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.7", 777));
      Assert.assertEquals("1", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.127", 777));
      Assert.assertEquals("1", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.64", 777));
      Assert.assertEquals("1", naMapper.get(addr));

      // 1.2.3.255/25 2
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.128", 1244));
      Assert.assertEquals("2", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.133", 777));
      Assert.assertEquals("2", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.150", 777));
      Assert.assertEquals("2", naMapper.get(addr));

      // 1.2.3.255/26 3
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.192", 3));
      Assert.assertEquals("3", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.222", 777));
      Assert.assertEquals("3", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.2.3.255", 777));
      Assert.assertEquals("3", naMapper.get(addr));

      // 1.0.0.0/8 4
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "0.0.0.0", 1244));
      Assert.assertEquals("4", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.0.0.1", 1244));
      Assert.assertEquals("4", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "1.16.0.0", 1244));
      Assert.assertEquals("4", naMapper.get(addr));

      // 192.168.231.44/13 5
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "192.168.231.44", 777));
      Assert.assertEquals("5", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "192.168.2.3", 777));
      Assert.assertEquals("5", naMapper.get(addr));
      addr = IPv4UDPAddress.fromInetSocketAddress(new InetSocketAddress(
          "192.168.99.98", 777));
      Assert.assertEquals("5", naMapper.get(addr));

    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  @Test
  public void testDistribution() {
    try {
      IPv4UDPGUIDMapper guidMapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-even-ipv4.xml");

      // Generate a bunch of GUIDs, hash them, then make sure we're around 25%
      // in each AS
      // guidMapper.g
      HashMap<NetworkAddress, Integer> counts = new HashMap<NetworkAddress, Integer>();
      Random r = new Random(0);
      for (int i = 0; i < 10000; ++i) {
        GUID guid = GUID.fromInt(r.nextInt());
        Collection<NetworkAddress> mappedAddx = guidMapper.getMapping(guid, 1,
            AddressType.INET_4_UDP);
        if (mappedAddx.isEmpty() || mappedAddx.size() != 1) {
          Assert.fail("Mapping did not return a single AS address.");
          return;
        }
        NetworkAddress addx = mappedAddx.iterator().next();
        Integer count = counts.get(addx);
        if (count == null) {
          count = Integer.valueOf(0);
        }
        counts.put(addx, Integer.valueOf(count.intValue() + 1));
      }
      /*
       *  127.0.0.1:5008: 1272 hits
      [14:21:03.852] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5004: 1270 hits
      [14:21:03.852] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5005: 1245 hits
      [14:21:03.852] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5006: 1244 hits
      [14:21:03.852] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5007: 1116 hits
      [14:21:03.852] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5001: 1264 hits
      [14:21:03.853] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5002: 1288 hits
      [14:21:03.853] - INFO - main/IPv4UDPGUIDMapperTest - 127.0.0.1:5003: 1301 hits

       */

      for (NetworkAddress addx : counts.keySet()) {
        Assert.assertTrue(counts.get(addx).intValue() > 1100);
      }

      // "Gap" checks

      /*
      32.0.0.0/4 1
      96.0.0.0/4 2
      160.0.0.0/4 3
      224.0.0.0/4 4
      */
      guidMapper = new IPv4UDPGUIDMapper("src/test/resources/map-gaps-ipv4.xml");

      // Generate a bunch of GUIDs, hash them, then make sure we're around 25%
      // in each AS
      // guidMapper.g
      counts = new HashMap<NetworkAddress, Integer>();
      r = new Random(0);
      for (int i = 0; i < 10000; ++i) {
        GUID guid = GUID.fromInt(r.nextInt());
        Collection<NetworkAddress> mappedAddx = guidMapper.getMapping(guid, 1,
            AddressType.INET_4_UDP);
        if (mappedAddx.isEmpty() || mappedAddx.size() != 1) {
          Assert.fail("Mapping did not return a single AS address.");
          return;
        }
        NetworkAddress addx = mappedAddx.iterator().next();
        Integer count = counts.get(addx);
        if (count == null) {
          count = Integer.valueOf(0);
        }
        counts.put(addx, Integer.valueOf(count.intValue() + 1));
      }

      for (NetworkAddress addx : counts.keySet()) {
        Assert.assertTrue(counts.get(addx).intValue() > 2400);
      }

      // Point-checking the "gaps"
      NetworkAddressMapper naMapper = guidMapper.networkAddressMap;

      // AS "1"
      IPv4UDPAddress testAddr = IPv4UDPAddress.fromASCII("0.0.0.0:123");
      Assert.assertEquals("1", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("16.0.0.1:123");
      Assert.assertEquals("1", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("24.0.0.1:123");
      Assert.assertEquals("1", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("48.0.0.1:123");
      Assert.assertEquals("1", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("59.0.0.1:123");
      Assert.assertEquals("1", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("63.255.255.255:123");
      Assert.assertEquals("1", naMapper.get(testAddr));

      // AS "2"
      testAddr = IPv4UDPAddress.fromASCII("64.0.0.0:123");
      Assert.assertEquals("2", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("80.2.0.19:123");
      Assert.assertEquals("2", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("92.3.20.0:123");
      Assert.assertEquals("2", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("100.99.0.0:123");
      Assert.assertEquals("2", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("127.255.255.255:123");
      Assert.assertEquals("2", naMapper.get(testAddr));

      // AS "3"
      testAddr = IPv4UDPAddress.fromASCII("128.0.0.49:123");
      Assert.assertEquals("3", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("148.0.6.0:123");
      Assert.assertEquals("3", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("168.0.111.0:123");
      Assert.assertEquals("3", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("191.255.255.255:123");
      Assert.assertEquals("3", naMapper.get(testAddr));

      // AS "4"
      testAddr = IPv4UDPAddress.fromASCII("192.0.22.0:123");
      Assert.assertEquals("4", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("202.42.0.0:123");
      Assert.assertEquals("4", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("232.0.0.112:123");
      Assert.assertEquals("4", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("252.0.55.0:123");
      Assert.assertEquals("4", naMapper.get(testAddr));
      testAddr = IPv4UDPAddress.fromASCII("255.255.255.255:123");
      Assert.assertEquals("4", naMapper.get(testAddr));

    } catch (IOException ioe) {
      Assert.fail(ioe.getMessage());
      ioe.printStackTrace();
    }
  }

  private static void testPrefix(final String filename) {
    try {
      IPv4UDPGUIDMapper guidMapper = new IPv4UDPGUIDMapper(filename);
      ConcurrentHashMap<Integer, InetSocketAddress> asAddresses = guidMapper.asAddresses;
      

      // Generate a bunch of GUIDs, hash them, then make sure we're around 25%
      // in each AS
      // guidMapper.g
      HashMap<NetworkAddress, Integer> counts = new HashMap<NetworkAddress, Integer>();
      for(InetSocketAddress a : asAddresses.values()){
        counts.put(IPv4UDPAddress.fromInetSocketAddress(a),Integer.valueOf(0));
      }
      Random r = new Random(0);
      for (int i = 0; i < 2000000; ++i) {
        GUID guid = GUID.fromInt(r.nextInt());
        Collection<NetworkAddress> mappedAddx = guidMapper.getMapping(guid, 1,
            AddressType.INET_4_UDP);
        if (mappedAddx.isEmpty() || mappedAddx.size() != 1) {
          Assert.fail("Mapping did not return a single AS address.");
          return;
        }
        NetworkAddress addx = mappedAddx.iterator().next();
        Integer count = counts.get(addx);
        if (count == null) {
          count = Integer.valueOf(0);
        }
        counts.put(addx, Integer.valueOf(count.intValue() + 1));
      }

      TreeMap<Integer, LinkedList<NetworkAddress>> histoMap = new TreeMap<Integer, LinkedList<NetworkAddress>>();

      for (NetworkAddress addx : counts.keySet()) {
        Integer count = counts.get(addx);
        if (count == null) {
          count = Integer.valueOf(0);
        }
        LinkedList<NetworkAddress> addxes = histoMap.get(count);
        if (addxes == null) {
          addxes = new LinkedList<NetworkAddress>();
          histoMap.put(count, addxes);
        }
        addxes.add(addx);
      }
      
      LOG.info("{} ASes", counts.size());

      for (Integer count : histoMap.keySet()) {
        StringBuilder sb = new StringBuilder(count.toString());
        sb.append(": ");
        LinkedList<NetworkAddress> list = histoMap.get(count);
        for (Iterator<NetworkAddress> iter = list.iterator(); iter.hasNext();) {
          NetworkAddress addx = iter.next();
          sb.append(addx);
          if (iter.hasNext()) {
            sb.append(", ");
          }
        }
        LOG.info(sb.toString());
      }
    } catch (IOException ioe) {
      ioe.printStackTrace();
    }
  }

  @Test
  public void testExamplePrefixes0() {
    testPrefix("src/test/resources/map-example0-ipv4.xml");
  }

  @Test
  public void testExamplePrefixes1() {
    testPrefix("src/test/resources/map-example1-ipv4.xml");
  }

  @Test
  public void testExamplePrefixes3() {
    testPrefix("src/test/resources/map-example3-ipv4.xml");
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.IPv4UDPGUIDMapper#getTypes()}
   * .
   */
  @Test
  public void testGetTypes() {
    try {
      IPv4UDPGUIDMapper mapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-ipv4.xml");
      EnumSet<AddressType> types = mapper.getTypes();
      Assert.assertEquals(1, types.size());
      Iterator<AddressType> iter = types.iterator();
      Assert.assertTrue(iter.hasNext());
      AddressType type = iter.next();
      Assert.assertFalse(iter.hasNext());
      Assert.assertEquals(AddressType.INET_4_UDP, type);
    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.IPv4UDPGUIDMapper#getDefaultAddressType()}
   * .
   */
  @Test
  public void testGetDefaultAddressType() {
    try {
      IPv4UDPGUIDMapper mapper = new IPv4UDPGUIDMapper(
          "src/test/resources/map-ipv4.xml");
      AddressType type = mapper.getDefaultAddressType();
      Assert.assertEquals(AddressType.INET_4_UDP, type);

    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

}
