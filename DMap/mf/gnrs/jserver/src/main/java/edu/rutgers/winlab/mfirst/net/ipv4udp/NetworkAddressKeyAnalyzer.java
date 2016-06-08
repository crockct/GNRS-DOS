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

import org.ardverk.collection.ByteArrayKeyAnalyzer;
import org.ardverk.collection.KeyAnalyzer;

import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Patricia Trie key analyzer for NetworkAddress objects.
 * 
 * @author Robert Moore
 */
public final class NetworkAddressKeyAnalyzer implements
    KeyAnalyzer<NetworkAddress> {

  /**
   * Analyzer that does the actual work.
   */
  private final transient ByteArrayKeyAnalyzer byteAnalyzer;

  /**
   * Constructor kept private.
   * 
   * @param type
   *          the type of address this analyzer will process.
   */
  private NetworkAddressKeyAnalyzer(final AddressType type) {
    super();
    this.byteAnalyzer = ByteArrayKeyAnalyzer.create(type.getMaxLength()*8);
  }

  /**
   * Creates a new NetworkAddressKeyAnalyzer for addresses of the specified
   * length.
   * 
   * @param type
   *          the type of address to create an analyzer for.
   * @return some analyzer for addresses of the specified length.
   */
  public static NetworkAddressKeyAnalyzer create(final AddressType type) {
    return new NetworkAddressKeyAnalyzer(type);
  }

  @Override
  public int compare(final NetworkAddress arg0, final NetworkAddress arg1) {
    return this.byteAnalyzer.compare(arg0.getValue(), arg1.getValue());
  }

  @Override
  public int bitIndex(final NetworkAddress arg0, final NetworkAddress arg1) {
    return this.byteAnalyzer.bitIndex(arg0.getValue(), arg1.getValue());
  }

  @Override
  public boolean isBitSet(final NetworkAddress arg0, final int arg1) {
    return this.byteAnalyzer.isBitSet(arg0.getValue(), arg1);
  }

  @Override
  public boolean isPrefix(final NetworkAddress arg0, final NetworkAddress arg1) {
    return this.byteAnalyzer.isPrefix(arg0.getValue(), arg1.getValue());
  }

  @Override
  public int lengthInBits(final NetworkAddress arg0) {
    if (arg0.getValue() == null) {
      return 0;
    }
    return this.byteAnalyzer.lengthInBits(arg0.getValue());
  }

}
