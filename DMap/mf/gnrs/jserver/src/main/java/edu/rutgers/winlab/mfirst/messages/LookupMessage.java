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

/**
 * A Lookup/Retrieval for a GUID value from the GNRS system.
 * <p>
 * Lookup/Retrieval messages specify a query GUID value to a GNRS server. The
 * response message, a {@link LookupResponseMessage}, is sent with the current
 * binding values (if present) back to the requester.
 * </p>
 * 
 * @author Robert Moore
 */
public class LookupMessage extends AbstractMessage {
  /**
   * The GUID to look up.
   */
  private GUID guid;

  /**
   * Creates a new Lookup message.
   */
  public LookupMessage() {
    super();
    super.type = MessageType.LOOKUP;

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

  @Override
  public String toString() {
    final StringBuilder buffer = new StringBuilder("LKP");
   
    buffer.append(" #").append(this.getRequestId()).append(" (");
    buffer.append(this.guid).append(")");
    return buffer.toString();
  }

  @Override
  protected int getPayloadLength() {
    int length = 0;
    // GUID
    if (this.guid != null && this.guid.getBinaryForm() != null) {
      length += this.guid.getBinaryForm().length;
    }
    return length;
  }

}
