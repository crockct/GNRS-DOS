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

import static org.junit.Assert.*;

import java.io.IOException;
import java.net.PortUnreachableException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * @author Robert Moore
 *
 */
public class InsertTaskTest {
  public GNRSServer server;
  
  @Before
  public void prepServer() {
    try {
      Configuration config = new Configuration();
      config.setMappingConfiguration("src/test/resources/map-ipv4.xml");
      config.setNetworkConfiguration("src/test/resources/net-ipv4.xml");
      config.setStoreConfiguration("src/test/resources/berkeleydb.xml");

      this.server = new GNRSServer(config);
    } catch (IOException e) {
      fail("Unable to create server");
    }
  }
  
  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.InsertTask#InsertTask(edu.rutgers.winlab.mfirst.GNRSServer, edu.rutgers.winlab.mfirst.net.SessionParameters, edu.rutgers.winlab.mfirst.messages.InsertMessage)}.
   */
  @Test
  public void testInsertTask() {
    try {
      InsertMessage msg = new InsertMessage();
      msg.setGuid(GUID.fromInt(5));
      msg.setOriginAddress(IPv4UDPAddress.fromASCII("127.0.0.1:9999"));
      msg.setBindings(new NetworkAddress[]{IPv4UDPAddress.fromASCII("127.0.0.1:1234")});
      msg.setRequestId(1);
      msg.setVersion((byte)0);
      InsertTask task = new InsertTask(server,null, msg);
     
      
    } catch (IOException e) {
      e.printStackTrace();
     fail(e.getMessage());
    }catch(Exception e1){
      e1.printStackTrace();
      fail(e1.getMessage());
    }
    Assert.assertTrue(true);
  }

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.InsertTask#call()}.
   */
  @Test//(expected=PortUnreachableException.class)
  public void testCall() {
    try {
      InsertMessage msg = new InsertMessage();
      msg.setGuid(GUID.fromInt(5));
      msg.setOriginAddress(IPv4UDPAddress.fromASCII("127.0.0.1:9999"));
      msg.setBindings(new NetworkAddress[]{IPv4UDPAddress.fromASCII("127.0.0.1:1234")});
      msg.setRequestId(1);
      msg.setVersion((byte)0);
      InsertTask task = new InsertTask(server,null, msg);
      task.call();
//      Assert.
    } catch (IOException e) {
      e.printStackTrace();
     fail(e.getMessage());
    }
  }

}
