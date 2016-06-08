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

import org.ardverk.collection.PatriciaTrie;
import org.ardverk.collection.Trie;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Network Address -> hostname (String) mapping for GNRS servers.
 * 
 * @author Robert Moore
 * 
 */
public class NetworkAddressMapper {

  /**
   * Logging for this class.
   */
  @SuppressWarnings("unused")
  private static final Logger LOG = LoggerFactory
      .getLogger(NetworkAddressMapper.class);

  /**
   * Prefix trie used to find the closest match of a network address.
   */
  private final transient Trie<NetworkAddress, String> storageTrie;

  /**
   * Creates a new mapper for the specified NetworkAddress type.
   * 
   * @param type
   *          the AddressType for the mapper.
   */
  public NetworkAddressMapper(final AddressType type) {
    super();
    this.storageTrie = new PatriciaTrie<NetworkAddress, String>(
        NetworkAddressKeyAnalyzer.create(type));
  }

  /**
   * Inserts a binding into this mapper for the specified network address.
   * 
   * @param netAddr
   *          the network address to map.
   * @param hostname
   *          the mapping for the address.
   * @return the previous mapped value, or {@code null} if there was none.
   */
  public String put(final NetworkAddress netAddr, final String hostname) {
    final String previous = this.storageTrie.remove(netAddr);
    this.storageTrie.put(netAddr, hostname);
    return previous;
  }

  /**
   * Returns the closest mapped value for the network address. Network address
   * "distance" is based on the XOR value of the binary form of the network
   * address.
   * 
   * @param netAddr
   *          the address to retrieve the binding for.
   * @return the closest mapped value for the network address or {@code null} if
   *         none exists.
   */
  public String get(final NetworkAddress netAddr) {
//    String retValue;
//    if (this.storageTrie.isEmpty()) {
//      retValue = null;
//    }

    // FIXME: Issue #9
    // <https://bitbucket.org/romoore/gnrs/issue/9/ipv4-guid-mapping-should-rehash-then-find>

    return this.storageTrie.selectValue(netAddr);
  }
}
