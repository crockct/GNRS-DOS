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

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.mina.core.future.WriteFuture;
import org.apache.mina.filter.codec.ProtocolEncoderOutput;
import org.junit.Assert;
import org.junit.Test;

/**
 * @author Robert Moore
 *
 */
public class ProtocolEncoderTest implements ProtocolEncoderOutput {

  private final Queue<Object> queue = new LinkedBlockingQueue<Object>();
  
  @Override
  public WriteFuture flush() {
    // TODO Auto-generated method stub
    return null;
  }

  
  @Override
  public void mergeAll() {
    // TODO Auto-generated method stub

  }

 
  @Override
  public void write(Object arg0) {
    this.queue.offer(arg0);
  }
  
  public Object next(){
    return this.queue.poll();
  }
  
  @Test
  public void fakeTest(){
    Assert.assertTrue(true);
  }

}
