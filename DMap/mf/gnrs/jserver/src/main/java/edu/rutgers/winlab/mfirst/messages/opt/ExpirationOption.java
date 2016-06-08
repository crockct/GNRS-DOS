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
package edu.rutgers.winlab.mfirst.messages.opt;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Arrays;

/**
 * @author Robert Moore
 */
public class ExpirationOption extends Option {

  public static final byte TYPE = 0x01;

  private final transient long[] timestamp;

  public ExpirationOption(final long[] timestamp) {
    super(TYPE);
    if(timestamp != null){
      this.timestamp = Arrays.copyOf(timestamp,timestamp.length);
    }else{
      this.timestamp = null;
    }
    super.setLength((byte) (this.timestamp.length * 8));
  }

  @Override
  public Object getOption() {
    return this.timestamp;
  }

  @Override
  public byte[] getBytes() {
    ByteArrayOutputStream baos = new ByteArrayOutputStream(super.getLength());
    DataOutputStream dos = new DataOutputStream(baos);
    try {
      for (long ts : this.timestamp) {
        dos.writeLong(ts);
      }
      dos.flush();
    } catch (IOException ioe) {
      return null;
    }

    return baos.toByteArray();
  }

  /**
   * Returns the expiration timestamp value for this option.
   * 
   * @return the expiration timestamp, in milliseconds since the Unix epoch.
   */
  public long[] getExpiration() {
    long[] returned = null;
    if (this.timestamp != null) {
      returned = Arrays.copyOf(this.timestamp, this.timestamp.length);
    }

    return returned;
  }

}
