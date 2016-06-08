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

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Robert Moore
 *
 */
public class ConfigurationTest {

  public Configuration config;
  
  @Before
  public void createConfig(){
    this.config = new Configuration();
  }
  
  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.ipv4udp.Configuration}.
   */
  @Test
  public void testBindAddress() {
    this.config.setBindAddress("127.0.0.1");
    Assert.assertEquals("127.0.0.1",this.config.getBindAddress());
    
    this.config.setBindAddress("1.2.3.4");
    Assert.assertEquals("1.2.3.4",this.config.getBindAddress());
    
  }

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.ipv4udp.Configuration}.
   */
  @Test
  public void testBindPort() {
    this.config.setBindPort(1234);
    Assert.assertEquals(1234,this.config.getBindPort());
    this.config.setBindPort(9999);
    Assert.assertEquals(9999,this.config.getBindPort());
  }

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.net.ipv4udp.Configuration}.
   */
  @Test
  public void testIsAscynchronousWrite() {
    this.config.setAsynchronousWrite(true);
    Assert.assertTrue(this.config.isAsynchronousWrite());
    this.config.setAsynchronousWrite(false);
  }

}
