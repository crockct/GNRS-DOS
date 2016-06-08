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

import java.io.UnsupportedEncodingException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.Arrays;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.GNRSServer;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * NetworkAddress convenience class for IPv4+UDP networking.
 * 
 * @author Robert Moore
 */
public class IPv4UDPAddress extends NetworkAddress {
  /**
   * Creates a new IPv4+UDP NetworkAddress from the specified value field.
   * 
   * @param value
   *          contains the binary form of the IPv4+UDP address (4-byte IP
   *          address, 2-byte UDP port)
   */
  public IPv4UDPAddress(final byte[] value) {
    super(AddressType.INET_4_UDP, value);
  }

  /**
   * Extension of NetworkAddress that provides IPv4/UDP-specific functionality.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(IPv4UDPAddress.class);

  /**
   * Converts the specified ASCII-encoded String to a Network Address value.
   * More specifically, the raw bytes of asString, when ASCII-encoded, are
   * stored in the bytes field. The resulting Network Address will be truncated
   * or padded with zeros as necessary.
   * 
   * @param asString
   *          the String to convert.
   * @return a Network Address with the value of the String
   * @throws UnsupportedEncodingException
   *           if the String cannot be decoded to ASCII characters
   */
  public static IPv4UDPAddress fromASCII(final String asString)
      throws UnsupportedEncodingException {
    IPv4UDPAddress address = null;
    if (asString == null || asString.length() == 0) {
      LOG.warn("Trying to create an IPv4/UDP address from an empty/null String.");
    } else {
      try {
        final String[] components = asString.split(":");

        InetAddress inet;

        inet = InetAddress.getByName(components[0]);

        int port = GNRSServer.DEFAULT_PORT;
        if (components.length > 1) {
          port = Short.parseShort(components[1]);
        }

        final byte[] newValue = new byte[AddressType.INET_4_UDP.getMaxLength()];
        System.arraycopy(inet.getAddress(), 0, newValue, 0, 4);
        newValue[newValue.length - 2] = (byte) (port >> 8);
        newValue[newValue.length - 1] = (byte) port;

        address = new IPv4UDPAddress(newValue);
      } catch (final UnknownHostException e) {
        LOG.error("Unable to parse IPv4 address.", e);
      }
    }
    return address;
  }

  /**
   * Creates a new NetworkAddress from the binary value of the integer. The 4
   * bytes of the integer value are copied into the high bytes (index 0-3) of
   * the network address.
   * 
   * @param intValue
   *          the integer to create an address from.
   * @return the created network address.
   */
  public static IPv4UDPAddress fromInteger(final int intValue) {

    final byte[] newValue = new byte[AddressType.INET_4_UDP.getMaxLength()];
    newValue[0] = (byte) (intValue >> 24);
    newValue[1] = (byte) (intValue >> 16);
    newValue[2] = (byte) (intValue >> 8);
    newValue[3] = (byte) (intValue);

    return new IPv4UDPAddress(newValue);
  }

  /**
   * Converts this NetworkAddress into an InetSocketAddress.
   * 
   * @param addr
   *          the network to convert
   * @return an InetSocketAddress representing the same IP and port combination
   *         as the NetworkAddress, or {@code null} if an error occurs during
   *         conversion.
   */
  public static InetSocketAddress toSocketAddr(final NetworkAddress addr) {
    InetSocketAddress retAddr = null;
    if (addr == null) {
      LOG.error("Unable to create InetSocketAddress from a null address.");
    } else {
      final byte[] value = addr.getValue();
      if (value == null) {
        LOG.error("Unable to create InetSocketAddress from null bytes.");
      } else {
        try {
          // Last two bytes are the port

          final int port = ((value[value.length - 2] << 8) | (value[value.length - 1]&0xFF)) & 0xFFFF;
          // First 4 bytes are the IP address
          retAddr = new InetSocketAddress(InetAddress.getByAddress(Arrays
              .copyOf(value, 4)), port);

        } catch (final UnknownHostException e) {
          LOG.error("Could not create InetSocketAddress from NetworkAddress.",
              e);

        }
      }
    }
    return retAddr;
  }

  /**
   * Creates a NetworkAddress object from an InetSocketAddress (IP address and
   * port).
   * 
   * @param addx
   *          the socket address to use
   * @return a new NetworkAddress object that represents the same address/port
   *         as the InetSocketAddress.
   */
  public static IPv4UDPAddress fromInetSocketAddress(
      final InetSocketAddress addx) {
    final byte[] fullAddx = new byte[6];
    System.arraycopy(addx.getAddress().getAddress(), 0, fullAddx, 0, 4);
    final int port = addx.getPort();
    fullAddx[4] = (byte) (port >> 8);
    fullAddx[5] = (byte) port;
    return new IPv4UDPAddress(fullAddx);

  }

  @Override
  public String toString() {
    final StringBuilder sBuff = new StringBuilder();
    int index = 0;
    int addxLength = Math.min(this.value.length, 4);
    for (; index < addxLength; ++index) {
      if (index > 0) {
        sBuff.append('.');
      }
      sBuff.append(Integer.toString(this.value[index] & 0xFF));
    }
    for (; index < 4; ++index) {
      sBuff.append('.').append('0');
    }
    if (this.value.length > 4) {
      sBuff.append(':');
      sBuff.append(Integer
          .toString(((this.value[4] << 8) | (this.value[5] & 0xFF)) & 0xFFFF));
    }
    return sBuff.toString();
  }
}
