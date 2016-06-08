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
import junit.framework.Assert;

import org.junit.Test;

/**
 * @author Robert Moore
 *
 */
public class AddressTypeTest {

  /**
   * String for IPv4/UDP network address type.
   */
  public static final String IPV4_UDP_STRING = "IPv4+UDP";
  /**
   * String for GUID type.
   */
  public static final String GUID_STRING = "GUID";
  
  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.AddressType}.
   */
  @Test
  public void test() {
    AddressType t1 = AddressType.INET_4_UDP;
    Assert.assertEquals(t1.value(), 0);
    Assert.assertEquals(t1.getMaxLength(),6);
    Assert.assertEquals(IPV4_UDP_STRING,t1.toString());
    
    
    AddressType t2 = AddressType.valueOf(0);
    Assert.assertEquals(t2.value(), 0);
    Assert.assertEquals(t2.getMaxLength(),6);
    Assert.assertEquals(IPV4_UDP_STRING,t2.toString());
    
    AddressType t3 = AddressType.GUID;
    Assert.assertEquals(t3.value(), 1);
    Assert.assertEquals(t3.getMaxLength(),20);
    Assert.assertEquals(GUID_STRING,t3.toString());
    
    AddressType t4 = AddressType.valueOf(1);
    Assert.assertEquals(t4.value(), 1);
    Assert.assertEquals(t4.getMaxLength(),20);
    Assert.assertEquals(GUID_STRING,t4.toString());
    
    Assert.assertTrue(t1.equals(t2));
    Assert.assertTrue(t2.equals(t1));
    Assert.assertTrue(t3.equals(t4));
    Assert.assertTrue(t4.equals(t3));
    Assert.assertFalse(t1.equals(t4));
    Assert.assertFalse(t3.equals(t2));
    
    
    AddressType nullType = AddressType.valueOf(-1);
    Assert.assertNull(nullType);
  }

 

}
