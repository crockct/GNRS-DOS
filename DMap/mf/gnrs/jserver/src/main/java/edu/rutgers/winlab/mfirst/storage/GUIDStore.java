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
package edu.rutgers.winlab.mfirst.storage;

import edu.rutgers.winlab.mfirst.GUID;

/**
 * Basic interface for storing and retrieving GUID bindings.
 * 
 * @author Robert Moore
 * 
 */
public interface GUIDStore {

  /**
   * Retrieves any current binding for the GUID value.
   * 
   * @param guid
   *          the GUID value.
   * @return any current binding this server has.
   */
  public GNRSRecord getBindings(final GUID guid);

  /**
   * Adds a GUID binding to this store.
   * 
   * @param guid
   *          the GUID to bind.
   * @param bindings
   *          the new bindings for the GUID.
   * @return {@code true} if the insert succeeds in all replicas
   */
  public boolean appendBindings(final GUID guid, final GUIDBinding... bindings);
  
  /**
   * Replaces all current bindings with the bindings provided.
   * @param guid the GUID to bind.
   * @param bindings the new bindings for the GUID 
   * @return {@code true} if the replace operation succeeds, else {@code false}. 
   */
  public boolean replaceBindings(final GUID guid, final GUIDBinding... bindings );

  /**
   * Must be called before the server starts accessing the storage class. Allows
   * separation of instantiation and initialization of the storage components.
   */
  public void doInit();

  /**
   * Must be called before the server exits so the storage class can perform any
   * necessary clean-up (closing file handles, writing back to disk, etc.).
   */
  public void doShutdown();

}
