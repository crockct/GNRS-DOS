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
package edu.rutgers.winlab.mfirst.storage.bdb;

import java.io.UnsupportedEncodingException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sleepycat.persist.model.KeyField;
import com.sleepycat.persist.model.Persistent;

import edu.rutgers.winlab.mfirst.GUID;

/**
 * BerkeleyDB-compatible version of a GUID.
 * 
 * @author Robert Moore
 * 
 */
@Persistent
public class BDBGUID {
  
  /**
   * Error logger.
   */
  private static final transient Logger LOG = LoggerFactory.getLogger(BDBGUID.class);

  /**
   * String-encoded form of the GUID value. Encoded as a UTF-16 (Big Endian)
   * String.
   */
  @KeyField(1)
  public String guid;

//  /**
//   * Creates a new, empty GUID.
//   */
//  public BDBGUID() {
//    super();
//  }

  /**
   * Creates a BDB GUID value from a GUID.
   * 
   * @param guid
   *          the GUID to construct the new BDBGUID from.
   * @return a new BDBGUID representing the GUID, or {@code null} if UTF-16 is
   *         not supported.
   */
  public static BDBGUID fromGUID(final GUID guid) {
    BDBGUID newBDB = null;
    try {
      final String asString = new String(guid.getBinaryForm(), "UTF-16BE");
      newBDB = new BDBGUID();
      newBDB.guid = asString;
    } catch (final UnsupportedEncodingException e) {
      LOG.error("Unable to create string from GUID bytes.", e);
    }
    return newBDB;
  }

  /**
   * Returns a GUID with the same value as this BDBGUID.
   * 
   * @return the GUID with the same value, or {@code null} if UTF-16 is not
   *         supported.
   */
  public GUID toGUID() {
    GUID newGuid = null;
    try {
      final byte[] bytes = this.guid.getBytes("UTF-16BE");
      newGuid = new GUID();
      newGuid.setBinaryForm(bytes);
    } catch (final UnsupportedEncodingException e) {
      LOG.error("Unable to create GUID from String form.",e);
    }
    return newGuid;
  }

  /**
   * Returns the String form of this GUID.
   * @return this GUID as a String.
   */
  public String getGuid() {
    return this.guid;
  }

  /**
   * Sets the String form of this GUID.
   * @param guid the new GUID.
   */
  public void setGuid(final String guid) {
    this.guid = guid;
  }
}
