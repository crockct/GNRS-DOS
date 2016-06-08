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
package edu.rutgers.winlab.mfirst.mapping;

import java.util.Collection;
import java.util.EnumSet;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Simple interface for classes that map GUID values to the Network Addresses of the
 * authoritative replicas.
 * 
 * @author Robert Moore
 * 
 */
public interface GUIDMapper {

  /**
   * Returns the network-appropriate NetworkAddresses for the specified GUID.
   * The mapper should make a "best effort" to resolve as many of different
   * AddressTypes provided as possible. If no AddressType values are provided,
   * then the mapper should return a default address type (specified in
   * #getDefaultAddressType()).
   * 
   * @param guid
   *          the GUID to map
   * @param numAddresses
   *          the number of addresses of each type to map.
   * @param types
   *          the set of requested network address types to return mappings for.
   *          If specified, then all returned results should be one of these
   *          types. If not specified ({@code null}), then the type should be
   *          the default type for this mapper.
   * @return the mappings for the GUID according to the specified types and the
   *         available mapping implementations provided by the mapper.
   */
  public Collection<NetworkAddress> getMapping(GUID guid, int numAddresses,
      AddressType... types);

  /**
   * The set of AddressType values supported by this mapper.
   * 
   * @return the supported AddressTypes for this GUID mapper.
   */
  public EnumSet<AddressType> getTypes();

  /**
   * The default AddressType for the mapper. When no AddressType is provided to
   * {@link #getMapping(GUID, int, AddressType...)}, then this type will be
   * generated.
   * 
   * @return the default AddressType for this mapper.
   */
  public AddressType getDefaultAddressType();
}
