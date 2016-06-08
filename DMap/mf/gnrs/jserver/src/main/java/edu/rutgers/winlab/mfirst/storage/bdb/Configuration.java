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


/**
 * Configuration settings for the Berkeley DB implementation of the GUID
 * storage engine.
 * 
 * @author Robert Moore
 * 
 */
public class Configuration {

  /**
   * Location of on-disk files for Berkeley DB storage. Defaults to a
   * subdirectory of
   * 
   */
  private String pathToFiles = "bdb";

  /**
   * Size of the in-memory cache used for Berkeley DB, in mebibytes
   * (2<sup>20</sup> bytes).
   */
  private int cacheSizeMiB = 64;

  /**
   * The path to where the DB files should be stored.
   * 
   * @return the location of the DB files.
   */
  public String getPathToFiles() {
    return this.pathToFiles;
  }

  /**
   * Sets the path to where the DB files should be stored.
   * 
   * @param pathToFiles
   *          filesystem path to where the DB files should be stored.
   * 
   */
  public void setPathToFiles(final String pathToFiles) {
    this.pathToFiles = pathToFiles;
  }

  /**
   * Returns the size of the cache, in mebibytes.
   * @return the size of the cache, in mebibytes.
   */
  public int getCacheSizeMiB() {
    return this.cacheSizeMiB;
  }

  /**
   * Sets the size of the DB cache.
   * @param cacheSizeMB the new size of the cache, in mebibytes.
   */
  public void setCacheSizeMiB(final int cacheSizeMB) {
    this.cacheSizeMiB = cacheSizeMB;
  }
}
