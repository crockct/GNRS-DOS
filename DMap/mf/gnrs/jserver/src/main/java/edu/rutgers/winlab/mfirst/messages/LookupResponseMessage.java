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

import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * A Lookup Response message for GNRS.
 * 
 * @author Robert Moore
 * 
 */
public class LookupResponseMessage extends AbstractResponseMessage {

  /**
   * The bindings for this message.
   */
  private NetworkAddress[] bindings;

  /**
   * Creates a new Lookup Response message.
   */
  public LookupResponseMessage() {
    super();
    super.type = MessageType.LOOKUP_RESPONSE;
  }

  /**
   * Gets the bindings for this message.
   * 
   * @return the bindings for this message, or {@code null} if there are none.
   */
  public NetworkAddress[] getBindings() {
    return this.bindings;
  }

  /**
   * Sets the bindings for this message.
   * 
   * @param bindings
   *          the new bindings for this message.
   */
  public void setBindings(final NetworkAddress[] bindings) {
    this.bindings = bindings;
  }

  @Override
  public String toString() {
    final StringBuilder sBuff = new StringBuilder("LKR #").append(this.getRequestId())
        .append("/").append(this.responseCode).append(" {");
    if (this.bindings != null) {
      for (int i = 0; i < this.bindings.length; ++i) {
        if (i > 0) {
          sBuff.append(", ");
        }
        sBuff.append(this.bindings[i]);
      }
    }
    sBuff.append("}");
    return sBuff.toString();
  }

  @Override
  protected int getResponsePayloadLength() {
    // Num bindings, bindings
    return 4 + this.getBindingsLength();

  }

  /**
   * The length of the bindings in this message. Used to compute the message
   * length.
   * 
   * @return the length, in bytes, of the bindings in this message.
   */
  protected int getBindingsLength() {
    int length = 0;
    if (this.bindings != null) {
      for (final NetworkAddress addx : this.bindings) {
        // Type, length, value
        length += (addx.getLength() + 4);
      }
    }
    return length;
  }

  /**
   * Returns the number of bindings in this message.
   * 
   * @return the number of bindings in this message.
   */
  public long getNumBindings() {
    return this.bindings == null ? 0 : this.bindings.length;
  }
}
