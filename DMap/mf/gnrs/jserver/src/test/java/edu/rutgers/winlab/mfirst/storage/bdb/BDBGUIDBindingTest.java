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
package edu.rutgers.winlab.mfirst.storage.bdb;

import static org.junit.Assert.fail;

import java.io.UnsupportedEncodingException;

import org.junit.Assert;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;

/**
 * @author Robert Moore
 *
 */
public class BDBGUIDBindingTest {

  @Test
  public void test(){
    try {
      NetworkAddress netAddr = IPv4UDPAddress.fromASCII("127.0.0.1:22");
      GUIDBinding realBind = new GUIDBinding();
      realBind.setAddress(netAddr);
      realBind.setTtl(5);
      realBind.setWeight(3);
      BDBGUIDBinding binding = new BDBGUIDBinding();
      Assert.assertNull(binding.address);
      Assert.assertEquals(0,binding.weight);
      Assert.assertEquals(0,binding.ttl);
      binding = BDBGUIDBinding.fromGUIDBinding(realBind);
      
      GUIDBinding converted = binding.toGUIDBinding();
      Assert.assertTrue(realBind.equals(converted));
      
      realBind = new GUIDBinding();
      binding = BDBGUIDBinding.fromGUIDBinding(realBind);
      converted = binding.toGUIDBinding();
      Assert.assertTrue(realBind.equals(converted));
      
      realBind = new GUIDBinding();
      realBind.setAddress(new NetworkAddress(null,null));
      binding = BDBGUIDBinding.fromGUIDBinding(realBind);
      converted = binding.toGUIDBinding();
      Assert.assertTrue(realBind.equals(converted));
      
    } catch (UnsupportedEncodingException e) {
      fail("Unable to decode ASCII.");
    }
    
    
    
  }
}
