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
package edu.rutgers.winlab.mfirst.client;

/**
 * Configuration settings for GNRS clients.
 * 
 * <p>Contains common settings for simulation/testing clients.</p>
 * 
 * @author Robert Moore
 * 
 */
public class Configuration {

  /**
   * Server hostname.
   */
  private String serverHost = "localhost";

  /**
   * Server port.
   */
  private int serverPort = 5001;

  /**
   * Client hostname (locally).
   */
  private String clientHost = "localhost";

  /**
   * Client sending port.
   */
  // FIXME: Make sure this is used
  private int clientPort = 4001;
  
  /**
   * Seed value for random generation.
   */
  private long randomSeed = -1;
  
  /**
   * Where to output client statistics files.
   */
  private String statsDirectory = "";

  /**
   * Returns the server hostname for this configuration.
   * 
   * @return the server hostname.
   */
  public String getServerHost() {
    return this.serverHost;
  }

  /**
   * Sets the server hostname for this configuration.
   * 
   * @param serverHost
   *          the new server hostname.
   */
  public void setServerHost(final String serverHost) {
    this.serverHost = serverHost;
  }

  /**
   * Gets the server port number.
   * 
   * @return the server port.
   */
  public int getServerPort() {
    return this.serverPort;
  }

  /**
   * Sets the server port.
   * 
   * @param serverPort
   *          the new server port number.
   */
  public void setServerPort(final int serverPort) {
    this.serverPort = serverPort;
  }

  /**
   * Gets the client sending port.
   * 
   * @return the client sending port number.
   */
  public int getClientPort() {
    return this.clientPort;
  }

  /**
   * Sets the client sending port.
   * 
   * @param clientPort
   *          the new sending port.
   */
  public void setClientPort(final int clientPort) {
    this.clientPort = clientPort;
  }

  /**
   * Gets the client sending hostname.
   * 
   * @return the client hostname.
   */
  public String getClientHost() {
    return this.clientHost;
  }

  /**
   * Sets the client sending hostname.
   * 
   * @param clientHost
   *          the new client hostname.
   */
  public void setClientHost(final String clientHost) {
    this.clientHost = clientHost;
  }

  /**
   * Get the random number generator seed value.
   * @return the random seed value.
   */
  public long getRandomSeed() {
    return this.randomSeed;
  }

  /**
   * Sets the random number generator seed value.
   * @param randomSeed the new seed value.
   */
  public void setRandomSeed(final long randomSeed) {
    this.randomSeed = randomSeed;
  }

  /**
   * Gets the directory for statistics files.
   * @return the directory for statistics files.
   */
  public String getStatsDirectory() {
    return this.statsDirectory;
  }

  /**
   * Sets the directory for statistics files.
   * @param statsDirectory the new directory for statistics files.
   */
  public void setStatsDirectory(final String statsDirectory) {
    this.statsDirectory = statsDirectory;
  }

}
