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
import java.io.FileWriter;
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
 * Makes Rainbow Table for first N GUIDs
 * Parameters are hard-coded and need to match those of the system you're generating for
 * 
 * @author Colleen Rock
 */
public class IPv4UDPGUID_RainbowTableMapTest {

  static final Logger LOG = LoggerFactory
      .getLogger(IPv4UDPGUID_RainbowTableMapTest.class);

  public GUID guid;
  public static final byte[] GUID_BYTE = new byte[] { 0x0, 0x1, 0x2, 0x3, 0x4,
      0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12,
      0x13 };

  public NetworkAddress ipv4Addr5;
  public NetworkAddress guidAddr;
  public int k = 2; // number of replicas for each GUID
  public int count_GUID_maps_less_k = 0; // count of how many GUIDs map to a set of less than k addresses
  public int N = 1;
  
  public String mapipv4file = "rock-configs/current/map-ipv4.xml";
  public String outfile = "rainbow_table.csv";

  @Rule
  public MethodRule watchman = new TestWatchman() {
    @Override
    public void starting(FrameworkMethod method) {
      LOG.info("{} being run...", method.getName());
      System.out.println(method.getName() + " being run...");
    }
  };
  

  /**
   * Test method for
   * {@link edu.rutgers.winlab.mfirst.mapping.ipv4udp.IPv4UDPGUIDMapper#getMapping(edu.rutgers.winlab.mfirst.GUID, int, edu.rutgers.winlab.mfirst.net.AddressType[])}
   * .
 * @throws IOException 
   */
  @Test
  public void testGetNMappings() throws IOException {
	  FileWriter writer = new FileWriter(outfile);
	  for (int i = 0; i < N; i++){
		  try{
			  this.guid = GUID.fromASCII(Integer.toString(i+1));
			  writer.append(this.guid.toString());
			  this.ipv4Addr5 = IPv4UDPAddress.fromASCII("127.0.0.1:5005"); //localhost
		} catch (UnsupportedEncodingException e) {
		  fail("Unable to decode from ASCII.");
		}
      this.guidAddr = new NetworkAddress(AddressType.GUID, new byte[] {});
		 
	    try {
	      IPv4UDPGUIDMapper mapper = new IPv4UDPGUIDMapper(mapipv4file);
	      Collection<NetworkAddress> mapped = mapper.getMapping(guid, k,
	          AddressType.INET_4_UDP);
	      Assert.assertNotNull(mapped);
	      Iterator<NetworkAddress> iter = mapped.iterator();
	      //Assert.assertEquals(k, mapped.size()); two valid IPs announced by same AS? 
	      if (mapped.size() < k){
	    	  count_GUID_maps_less_k++;
	      }
	      
	      while (iter.hasNext()){
	    	  NetworkAddress netAddr = iter.next();
	    	  writer.append(",");
	    	  writer.append(netAddr.toString());
	      }
	      writer.append('\n');
	    } catch (IOException e) {
	      // TODO Auto-generated catch block
	      e.printStackTrace();
	    }
	}
	writer.flush();
	writer.close();
	System.out.println(Integer.toString(count_GUID_maps_less_k) + "/" + Integer.toString(N) + " mapped to less than " + Integer.toString(k) + " ASs.");
  }
  
}


