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

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * A message for inserting a GUID->NetworkAddress binding into the GNRS server.
 * 
 * @author Robert Moore
 */
public class InsertMessage extends AbstractMessage {
  /**
   * The GUID to insert.
   */
  private GUID guid;

  /**
   * The set of GUID&rarr;NetworkAddress bindings for this Insert message.
   */
  private NetworkAddress[] bindings;

  /**
   * Creates a new Insert message.
   */
  public InsertMessage() {
    super();
    super.type = MessageType.INSERT;
  }

  /**
   * Returns the GUID for this message.
   * 
   * @return the GUID for this message.
   */
  public GUID getGuid() {
    return this.guid;
  }

  /**
   * Sets the GUID for this message.
   * 
   * @param guid
   *          the new GUID for this message.
   */
  public void setGuid(final GUID guid) {
    this.guid = guid;
  }

  /**
   * Gets the set of GUID&rarr;Network Address bindings for this message.
   * 
   * @return the Network Address bindings for this message.
   */
  public NetworkAddress[] getBindings() {
    return this.bindings;
  }

  /**
   * Returns the number of bindings in this message.
   * 
   * @return the number of bindings.
   */
  public long getNumBindings() {
    return this.bindings == null ? 0 : this.bindings.length;
  }

  /**
   * Sets the GUID&rarr;Network Address bindings for this message.
   * 
   * @param bindings
   *          the new Network Address bindings for this message.
   */
  public void setBindings(final NetworkAddress[] bindings) {
    this.bindings = bindings;
  }

  @Override
  public String toString() {
    final StringBuilder sBuff = new StringBuilder("INS #");
    sBuff.append(this.getRequestId()).append(' ').append(this.guid)
        .append(" -> {");
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
  public int getPayloadLength() {
    //  num bindings -> 4, + bindings  
    int length = 4 + this.getBindingsLength();
    // GUID
    if (this.guid != null && this.guid.getBinaryForm() != null) {
      length += this.guid.getBinaryForm().length;
    }
    return length;
  }

  /**
   * The length (in bytes) of the bindings contained in this message. Only to be
   * used for network encoding.
   * 
   * @return the length (in bytes) of the bindings when encoded for the network
   *         protocol.
   */
  protected int getBindingsLength() {
    int length = 0;
    if (this.bindings != null) {
      for (final NetworkAddress addr : this.bindings) {
        length += (4 + addr.getLength());
      }
    }
    return length;
  }

}
