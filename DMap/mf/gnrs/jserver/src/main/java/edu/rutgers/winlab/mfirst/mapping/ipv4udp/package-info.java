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

/**
 * Classes and utilities for GUID&rarr;Network Address (NA) for IPv4/UDP network
 * implementations.
 * 
 * <p>These classes are specific to the GUID&rarr;NA mapping functionality for IPv4/UDP
 * networking interfaces.  Authoritative GNRS replica servers will be mapped from IPv4 
 * address values based on network prefix announcements (via BGP routing tables).</p>
 * 
 * <p>GUID&rarr;NA mapping occurs using the following steps:
 *  <ol>
 *    <li>GUID is hashed using one of several hash functions (MD5, SHA-1, SHA-256) to generate
 *    a sequence of random byte values.</li>
 *    <li>The byte values are then mapped into <em>K</em> different IP addresses.  If the hash
 *    function does not generate enough bytes for the desired number of addresses, then the output
 *    of the previous hash value is appended to the GUID value, and this combined value is hashed.</li>
 *    <li>The random IP address vales are matched against the BGP routing information and
 *    AS responsible for the random IP address is determined. In the case of allocation "holes",
 *    then the IP address bytes are concatenated to the GUID value and rehashed.  This continues until
 *    a matching prefix is determined, or sufficient attempts have failed and a "nearest" prefix
 *    is used instead.</li>
 *    <li>The <em>k</em> authoritative replica servers are forwarded the LOOKUP, APPEND, or REPLACE 
 *    message from the client.  If one of the <em>k</em> replicas is the local server, then
 *    the LOOKUP, APPEND, or REPLACE is performed locally BEFORE forwarding to remote servers.</li>
 *    <li>The client receives a response message to indicate SUCCESS or FAILURE, depending on
 *    the options requested and result of the request.</li>
 *  </ol>
 * </p>
 */

package edu.rutgers.winlab.mfirst.mapping.ipv4udp;