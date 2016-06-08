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
import java.util.List;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Interface for classes that will select one or more of the K replicas
 * for contact by a local server.  The determination of how many, which, and
 * in what order, the servers are provided depends on the implementation of the
 * selector and its configuration.
 * 
 * @author Robert Moore
 *
 */
public interface ReplicaSelector {
  /**
   * From the Collection of provided NetworkAddress values, returns a List of ranked Network Address values.
   * The returned List is ordered from most preferred value (index 0) to least preferred value (index {@code size()-1}).
   * The returned List may only be a subset of the original Collection (it may have fewer members).
   * @param servers a Collection of GNRS servers to select from.
   * @return a List containing a subset of the servers, ordered from most preferred to least preferred. 
   */
  List<NetworkAddress> getContactList(Collection<NetworkAddress> servers);
}
