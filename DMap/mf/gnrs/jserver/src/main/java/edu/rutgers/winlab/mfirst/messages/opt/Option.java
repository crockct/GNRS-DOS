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

/**
 * A generic interface for message options to implement.
 * 
 * @author Robert Moore
 */
public abstract class Option {

  private static final byte FINAL_FLAG = (byte) 0x80;
  public static final transient byte USER_TYPES_FLAG = (byte) 0x7F;

  private transient byte type;
  private transient byte length;
  
  protected Option(final byte type){
    super();
    this.type = (byte)(type & USER_TYPES_FLAG);
    this.length = 0;
  }

  protected Option(final byte type, final byte length) {
    super();
    this.type = (byte) (type & USER_TYPES_FLAG);
    this.length = length;
  }

  public byte getType() {
    return this.type;
  }

  public byte getLength() {
    return this.length;
  }
  
  protected void setLength(final byte length){
    this.length = length;
  }

  public abstract Object getOption();
  
  public abstract byte[] getBytes();

  public boolean isFinal() {
    return (this.type & FINAL_FLAG) == FINAL_FLAG;
  }

  public void setFinal() {
    this.type |= FINAL_FLAG;
  }
  
  public static boolean isFinal(final byte type){
    return (type & FINAL_FLAG) == FINAL_FLAG;
  }
}
