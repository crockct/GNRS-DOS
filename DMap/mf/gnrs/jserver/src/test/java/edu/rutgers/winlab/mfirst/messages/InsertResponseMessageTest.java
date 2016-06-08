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
package edu.rutgers.winlab.mfirst.messages;

import static org.junit.Assert.*;

import org.junit.Assert;
import org.junit.Test;

/**
 * @author Robert Moore
 *
 */
public class InsertResponseMessageTest {

  /**
   * Test method for {@link edu.rutgers.winlab.mfirst.messages.InsertResponseMessage}.
   */
  @Test
  public void test() {
    InsertResponseMessage msg = new InsertResponseMessage();
    Assert.assertEquals(4,msg.getPayloadLength());
    Assert.assertEquals(0,msg.getResponsePayloadLength());
    Assert.assertEquals(16,msg.getMessageLength());
    Assert.assertEquals("INR #0/null",msg.toString());
    
    
    msg.setResponseCode(ResponseCode.FAILED);
    Assert.assertTrue(ResponseCode.FAILED.equals(msg.getResponseCode()));
    msg.setResponseCode(ResponseCode.SUCCESS);
    Assert.assertTrue(ResponseCode.SUCCESS.equals(msg.getResponseCode()));
  }

 
}
