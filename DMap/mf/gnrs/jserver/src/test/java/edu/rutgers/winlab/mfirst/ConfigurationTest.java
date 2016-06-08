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
package edu.rutgers.winlab.mfirst;

import junit.framework.Assert;

import org.junit.Test;

/**
 * @author Robert Moore
 */
public class ConfigurationTest {

  /**
   * A string for testing.
   */
  public static final String TEST_STRING = "/path/to/file.xml";

  /**
   * General test case for getters/setters in the Configuration class.
   */
  @Test
  public void test() {
    final Configuration config = new Configuration();

    config.setCollectStatistics(true);
    Assert.assertTrue(config.isCollectStatistics());
    config.setCollectStatistics(false);
    Assert.assertFalse(config.isCollectStatistics());

    config.setMappingConfiguration(TEST_STRING);
    Assert.assertEquals(TEST_STRING, config.getMappingConfiguration());

    config.setNetworkConfiguration(TEST_STRING);
    Assert.assertEquals(TEST_STRING, config.getNetworkConfiguration());

    config.setNetworkType(TEST_STRING);
    Assert.assertEquals(TEST_STRING, config.getNetworkType());
    
    config.setStoreConfiguration(TEST_STRING);
    Assert.assertEquals(TEST_STRING, config.getStoreConfiguration());
    
    config.setNumReplicas(5);
    Assert.assertEquals(5, config.getNumReplicas());
    config.setNumReplicas(-1);
    Assert.assertEquals(-1, config.getNumReplicas());
    
    config.setNumWorkerThreads(5);
    Assert.assertEquals(5,config.getNumWorkerThreads());
    config.setNumWorkerThreads(0);
    Assert.assertEquals(0, config.getNumWorkerThreads());
    
    config.setStoreType(TEST_STRING);
    Assert.assertEquals(TEST_STRING, config.getStoreType());
    
    config.setNumAttempts(5);
    Assert.assertEquals(5,config.getNumAttempts());
    
    config.setTimeoutMillis(3210l);
    Assert.assertEquals(3210l,config.getTimeoutMillis());
    
  }
}
