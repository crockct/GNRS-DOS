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

/**
 * Conifguration settings for the IPv4/UDP networking implementation.
 * 
 * <p>
 * Specific network configuration settings for a GNRS networking interface that
 * utilizes IPv4 and UDP for network communication. The major components are the
 * IP prefixes file (for BGP-like route announcements) and the AS bindings file,
 * which provides routable addresses for the GNRS servers within each Autonomous
 * System (AS).
 * </p>
 * 
 * @author Robert Moore
 * 
 */
public class Configuration {

  /**
   * IP address for the interface on which to listen for incoming messages.
   */
  private String bindAddress = "127.0.0.1";

  /**
   * UDP port on which to listen for incoming connections.
   */
  private int bindPort = 5001;
  
  /**
   * UDP port from which to send messages.
   */
  private int sendPort = 5001;
  

  /**
   * Flag to indicate whether or not writes should be asynchronous
   * (non-blocking).
   */
  private boolean asynchronousWrite = false;

  /**
   * The address to which the server should bind.
   * 
   * @return the address to bind on, or "" if the server should bind to any
   *         available interface.
   */
  public String getBindAddress() {
    return this.bindAddress;
  }

  /**
   * Sets the binding address for the server.
   * 
   * @param bindAddress
   *          the new binding address.
   */
  public void setBindAddress(final String bindAddress) {
    this.bindAddress = bindAddress;
  }

  /**
   * Gets the bound port for the server.
   * 
   * @return the UDP port to listen for messages.
   */
  public int getBindPort() {
    return this.bindPort;
  }

  /**
   * Sets the binding port for the server.
   * 
   * @param bindPort
   *          the new binding port.
   */
  public void setBindPort(final int bindPort) {
    this.bindPort = bindPort;
  }

  /**
   * Flag to configure (a)synchronous writes to the network interface.
   * 
   * @return {@code true} if network writes are asynchronous (non-blocking).
   */
  public boolean isAsynchronousWrite() {
    return this.asynchronousWrite;
  }

  /**
   * Sets the flag for (a)synchronous writes to the network.
   * 
   * @param asynchronousWrite
   *          {@code true} if network writes should be asynchronous
   *          (non-blocking), or {@code false} for synchronous (blocking)
   *          writes.
   */
  public void setAsynchronousWrite(final boolean asynchronousWrite) {
    this.asynchronousWrite = asynchronousWrite;
  }

  /**
   * Get the port from which to send messages.
   * @return the sending port.
   */
  public int getSendPort() {
    return this.sendPort;
  }

  /**
   * Sets the port from which to send message.
   * @param sendPort the new sending port value.
   */
  public void setSendPort(int sendPort) {
    this.sendPort = sendPort;
  }
}
