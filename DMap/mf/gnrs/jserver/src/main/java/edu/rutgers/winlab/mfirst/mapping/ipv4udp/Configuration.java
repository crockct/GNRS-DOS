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
package edu.rutgers.winlab.mfirst.mapping.ipv4udp;

/**
 * Configuration object for IPv4+UDP GUID mapper.
 * 
 * @author Robert Moore
 * 
 */
public class Configuration {
  /**
   * The hashing algorithm to use when converting a GUID to a Network Address
   * for insert or retrieval in the GNRS.
   */
  private String hashAlgorithm = "MD5";

  // TODO: This is a short-term solution. Need something more flexible.
  /**
   * Filename for network address prefix->AS bindings.
   */
  private String prefixFile = "src/main/resources/prefixes.ipv4";

  // TODO: This is a short-term solution. Need something more flexible.
  /**
   * Filename for AS network binding values.
   */
  private String asBindingFile = "src/main/resources/as-binding.ipv4";

  /**
   * Gets the hash algorithm name.
   * 
   * @return the hash algorithm name.
   */
  public String getHashAlgorithm() {
    return this.hashAlgorithm;
  }

  /**
   * Sets the hash algorithm name.
   * 
   * @param hashAlgorithm
   *          the new algorithm name.
   */
  public void setHashAlgorithm(final String hashAlgorithm) {
    this.hashAlgorithm = hashAlgorithm;
  }

  /**
   * Gets the name of the file that contains the IP prefixes (BGP table).
   * 
   * @return the name of the IP prefix file.
   */
  public String getPrefixFile() {
    return this.prefixFile;
  }

  /**
   * Sets the name of the file that contains the IP prefixes.
   * 
   * @param prefixFile
   *          the new IP prefix filename.
   */
  public void setPrefixFile(final String prefixFile) {
    this.prefixFile = prefixFile;
  }

  /**
   * Gets the name of the Autonomous System (AS) bindings file. The file
   * contains the AS number and network address value for the GNRS server in
   * that system.
   * 
   * @return the name of the AS bindings file.
   */
  public String getAsBindingFile() {
    return this.asBindingFile;
  }

  /**
   * Sets the name of the Autonomous System (AS) bindings file. The file
   * contains the AS number and network address value for the GNRS server in
   * that system.
   * 
   * @param asBindingFile
   *          the name of the AS bindings file.
   */
  public void setAsBindingFile(final String asBindingFile) {
    this.asBindingFile = asBindingFile;
  }
}
