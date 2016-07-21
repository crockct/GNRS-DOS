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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thoughtworks.xstream.XStream;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.mapping.GUIDMapper;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.NetworkAddressMapper;

/**
 * GUID Mapper for IPv4/UDP networking.
 * 
 * @author Robert Moore
 */
public class IPv4UDPGUIDMapper implements GUIDMapper {

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(IPv4UDPGUIDMapper.class);

  /**
   * Hashing object to compute a random network address.
   */
  private final transient GUIDHasher hasher;

  /**
   * Mapping network address prefixes to AS/addresses.
   */
  public final transient NetworkAddressMapper networkAddressMap = new NetworkAddressMapper(
      AddressType.INET_4_UDP);

  /**
   * Mapping of Autonomous System numbers to their network locations.
   */
  public final transient ConcurrentHashMap<Integer, InetSocketAddress> asAddresses = new ConcurrentHashMap<Integer, InetSocketAddress>();

  /**
   * Creates a new IPv4+UDP GUID mapper from the specified configuration
   * filename. The configuration file is opened, parsed, and this mapper is
   * configured.
   * 
   * @param configFile
   *          the name of the configuration file for this mapper.
   * @throws IOException
   *           if an IOException is thrown while reading the configuration file.
   */
  public IPv4UDPGUIDMapper(final String configFile) throws IOException {
    final Configuration config = this.loadConfiguration(configFile);

    // Load the network prefix announcements
    this.loadPrefixes(config.getPrefixFile());

    // Load the AS network address binding values
    this.loadAsNetworkBindings(config.getAsBindingFile());

    this.hasher = new MessageDigestHasher(config.getHashAlgorithm());

  }
  
  /** @author Colleen
   * return all ASs / server addresses for attack
   */
  public Collection<NetworkAddress> getAll() {
	  Collection<NetworkAddress> allAddrs = new ArrayList<NetworkAddress>();
	  for (InetSocketAddress ipAddr : this.asAddresses.values()){
		  allAddrs.add(IPv4UDPAddress.fromInetSocketAddress(ipAddr));
	  }
	  return allAddrs;
  }

  /**
   * Loads this Mapper's configuration file from the filename provided.
   * 
   * @param filename
   *          the name of the configuration file.
   * @return the configuration object
   */
  private Configuration loadConfiguration(final String filename) {
    final XStream xStream = new XStream();
    return (Configuration) xStream.fromXML(new File(filename));
  }

  /**
   * Loads network prefix mappings from a file.
   * 
   * @param prefixFilename
   *          the filename of the prefix mapping file.
   * @throws IOException
   *           if an exception occurs while reading the file.
   */
  private void loadPrefixes(final String prefixFilename) throws IOException {
    final File prefixFile = new File(prefixFilename);
    final BufferedReader lineReader = new BufferedReader(new FileReader(
        prefixFile));

    String line = lineReader.readLine();
    while (line != null) {
      // Eliminate leading/trailing whitespace
      line = line.trim();
      // Skip comments
      if (line.length() == 0 || line.charAt(0) == '#') {
        line = lineReader.readLine();
        continue;
      }

      // log.debug("Parsing \"{}\"", line);
      // Extract any comments and discard
      final String content = line.split("#")[0];

      final String[] generalComponents = content.split("\\s+");
      if (generalComponents.length < 2) {
        LOG.warn("Not enough components to parse the line \"{}\".", line);
        line = lineReader.readLine();
        continue;
      }
      // Extract the base address and prefix length
      final String[] prefixParts = generalComponents[0].split("/");
      final InetAddress addx = InetAddress.getByName(prefixParts[0]);
      final byte[] addxBytes = addx.getAddress();

      // Extract prefix length
      final int prefixLength = Integer.parseInt(prefixParts[1]);
      final int mask = prefixLength == 0 ? 0 : (0x80000000 >> (prefixLength-1));

      final int addxAsInt = ((((addxBytes[0] << 24) & 0xFF000000)
          | ((addxBytes[1] << 16) & 0xFF0000) | ((addxBytes[2] << 8) & 0xFF00) | ((addxBytes[3]) & 0xFF))
          & mask);
      
      final NetworkAddress netAddr = IPv4UDPAddress.fromInteger(addxAsInt);
      
      this.networkAddressMap.put(netAddr, generalComponents[1]);

      line = lineReader.readLine();
    }
    lineReader.close();
    LOG.info("Finished loading prefix map.");

  }

  /**
   * Loads the Autonomous System (AS) network bindings file.
   * 
   * @param asBindingFilename
   *          the name of the AS bindings file.
   * @throws IOException
   *           if an IOException is thrown while reading the bindings file.
   */
  private void loadAsNetworkBindings(final String asBindingFilename)
      throws IOException {
    final File asBindingFile = new File(asBindingFilename);
    final BufferedReader lineReader = new BufferedReader(new FileReader(
        asBindingFile));

    String line = lineReader.readLine();
    while (line != null) {
      // Eliminate leading/trailing whitespace
      line = line.trim();
      // Skip comments
      if (line.length() == 0 || line.charAt(0) == '#') {
        line = lineReader.readLine();
        continue;
      }

      // log.debug("Parsing \"{}\"", line);
      // Extract any comments and discard
      final String content = line.split("#")[0];

      // Extract the 3 parts (AS #, IP address, port)
      final String[] generalComponents = content.split("\\s+");
      if (generalComponents.length < 3) {
        LOG.warn("Not enough components to parse the line \"{}\".", line);
        line = lineReader.readLine();
        continue;
      }

      final Integer asNumber = Integer.valueOf(generalComponents[0]);
      final String ipAddrString = generalComponents[1];
      final int port = Integer.parseInt(generalComponents[2]);

      final InetSocketAddress sockAddx = new InetSocketAddress(ipAddrString,
          port);
      this.asAddresses.put(asNumber, sockAddx);

      line = lineReader.readLine();
    }
    lineReader.close();
    LOG.info("Finished loading AS network binding values.");

  }

  /*
   * (non-Javadoc)
   * 
   * @see
   * edu.rutgers.winlab.mfirst.mapping.GUIDMapper#getMapping(edu.rutgers.winlab
   * .mfirst.structures.GUID, edu.rutgers.winlab.mfirst.net.AddressType[])
   */
  @Override
  public Collection<NetworkAddress> getMapping(final GUID guid,
      final int numAddresses, final AddressType... types) {

    List<AddressType> returnedTypes;
    if (types == null || types.length == 0) {
      returnedTypes = new ArrayList<AddressType>();
      returnedTypes.add(this.getDefaultAddressType());
    }
    // Match-up returned types with supported types.
    else {
      returnedTypes = new LinkedList<AddressType>();
      final EnumSet<AddressType> supportedTypes = this.getTypes();
      for (final AddressType t : types) {
        if (supportedTypes.contains(t)) {
          returnedTypes.add(t);
        }
      }
      // No matching types found, so no mapping possible.
      // if (returnedTypes.isEmpty()) {
      // return null;
      // }
    }

    final Set<NetworkAddress> returnedAddresses = new HashSet<NetworkAddress>();

    for (final AddressType type : returnedTypes) {
      returnedAddresses
          .addAll(this.getAddressForType(type, guid, numAddresses));

    }

    if (returnedAddresses.isEmpty()) {
      return null;
    }
    return returnedAddresses;
  }

  /**
   * Returns a collection of mapped NetworkAddresses, randomly generated from
   * {@code guid} of the specified type.
   * 
   * @param type
   *          the type of address to create.
   * @param guid
   *          the GUID to use to generate the addresses.
   * @param numAddresses
   *          the number of addresses to create.
   * @return a collection containing the addresses, if they were able to be
   *         generated.
   */
  private Collection<NetworkAddress> getAddressForType(final AddressType type,
      final GUID guid, final int numAddresses) {
    final LinkedList<NetworkAddress> returnedAddresses = new LinkedList<NetworkAddress>();
    try {
      // Generate some addresses from the GUID
      final Collection<NetworkAddress> randomAddresses = this.hasher.hash(guid,
          type, numAddresses);

      // Map them to an AS
      for (final NetworkAddress netAddr : randomAddresses) {
        final NetworkAddress finalAddr = this.performMapping(netAddr);
        if (finalAddr == null) {
          LOG.error("Unable to map NetworkAddress for {}", netAddr);
        } else {
          returnedAddresses.add(finalAddr);
        }
      }

    } catch (final NoSuchAlgorithmException e) {
      LOG.error("Unable to hash GUID for type " + type, e);
    }
    return returnedAddresses;
  }

  /**
   * Maps a network address to an AS based on the announced prefix table.
   * 
   * @param netAddr
   *          a random network address.
   * @return the NetworkAddress of the GNRS server of the AS "responsible" for
   *         {@code netAddr}
   */
  private NetworkAddress performMapping(final NetworkAddress netAddr) {
    final String autonomousSystem = this.networkAddressMap.get(netAddr);
    NetworkAddress finalAddr;
    if (autonomousSystem == null) {
      // FIXME: Rehash?
      LOG.error("Found mapping hole for {}", netAddr);
      finalAddr = null;
    } else {
      final InetSocketAddress asGNRSAddr = this.asAddresses.get(Integer
          .decode(autonomousSystem));
      finalAddr = IPv4UDPAddress.fromInetSocketAddress(asGNRSAddr);
    }
    return finalAddr;

  }

  @Override
  public EnumSet<AddressType> getTypes() {
    return EnumSet.of(AddressType.INET_4_UDP);
  }

  @Override
  public AddressType getDefaultAddressType() {
    return AddressType.INET_4_UDP;
  }

}
