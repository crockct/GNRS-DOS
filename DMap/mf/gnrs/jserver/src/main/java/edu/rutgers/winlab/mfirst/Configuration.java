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
package edu.rutgers.winlab.mfirst;

/**
 * Configuration class for GNRS Server.
 * 
 * @author Robert Moore
 */
public class Configuration {

  /**
   * Minimum number of worker threads to utilize.
   */
  private int numWorkerThreads = 1;

  /**
   * Whether or not the server should collect and output statistics to the log.
   */
  private boolean collectStatistics = false;

  /**
   * The number of replicas to use when inserting or retrieving bindings from
   * the network.
   */
  private int numReplicas = 5;

  /**
   * The type of networking to use. This value determines the
   * NetworkAccessObject (NAO) the server will utilize.
   */
  private String networkType = "ipv4udp";

  /**
   * Network access object configuration file. Should be changed together with
   * networkType.
   */
  private String networkConfiguration = "net-ipv4.xml";

  /**
   * GUID mapping object configuration file. Should be changed together with
   * networkType.
   */
  private String mappingConfiguration = "map-ipv4.xml";

  /**
   * Type of GUID storage to use.
   */
  private String storeType = "simple";

  /**
   * Configuration filename for the GUID storage object.
   */
  private String storeConfiguration = "berkeleydb.xml";

  /**
   * Timeout period (in milliseconds) for messages sent to remote servers.
   */
  private long timeoutMillis = 500l;

  /**
   * Number of attempts to make to a remote server before returning a "FAILURE"
   * response to clients.
   */
  private int numAttempts = 2;
  
  /**
   * Number of GUID values in the local cache.
   */
  private int cacheEntries = 0;
  
  /**
   * How long to keep bindings in the server when no expiration value is specified.
   */
  private long defaultExpiration = 900000;
  
  /**
   * How long to cache values in the server when no TTL value is specified.
   */
  private long defaultTtl = 60000;
  
  /**
   * Where to place statistics CSV files.
   */
  private String statsDirectory = "";
  
  /**
   * The type of replica selector to use.
   */
  private String replicaSelector = "random";

  /**
   * Gets the current number of worker threads used.
   * 
   * @return the number of worker threads in use.
   */
  public int getNumWorkerThreads() {
    return this.numWorkerThreads;
  }

  /**
   * Sets the number of worker threads to use.
   * 
   * @param minWorkerThreads
   *          the new number of worker threads.
   */
  public void setNumWorkerThreads(final int minWorkerThreads) {
    this.numWorkerThreads = minWorkerThreads;
  }

  /**
   * Flag to indicate if statistics are being collected.
   * 
   * @return {@code true} if they are being collected, else {@code false}.
   */
  public boolean isCollectStatistics() {
    return this.collectStatistics;
  }

  /**
   * Sets statistics collection on or off.
   * 
   * @param collectStatistics
   *          {@code true} to collect statistics.
   */
  public void setCollectStatistics(final boolean collectStatistics) {
    this.collectStatistics = collectStatistics;
  }

  /**
   * The number of replica GNRS servers to utilize.
   * 
   * @return the current number of replicas.
   */
  public int getNumReplicas() {
    return this.numReplicas;
  }

  /**
   * Sets the number of replica GNRS servers to utilize.
   * 
   * @param numReplicas
   *          the new number of replicas.
   */
  public void setNumReplicas(final int numReplicas) {
    this.numReplicas = numReplicas;
  }

  /**
   * Gets the network type for the server.
   * 
   * @return the network type value.
   */
  public String getNetworkType() {
    return this.networkType;
  }

  /**
   * Sets the network type for the server.
   * <p>
   * Currently-supported types:
   * </p>
   * <table>
   * <thead>
   * <th style="text-align: left">Value</th>
   * <th style="text-align: left">Description</th> </thead>
   * <tr>
   * <td><code>ipv4udp</code></td>
   * <td>Internet Protocol version 4 with UDP.</td>
   * </tr>
   * </table>
   * 
   * @param networkType
   *          the new network type value.
   */
  public void setNetworkType(final String networkType) {
    this.networkType = networkType;
  }

  /**
   * Get the name of the network configuration file.
   * 
   * @return the network configuration filename.
   */
  public String getNetworkConfiguration() {
    return this.networkConfiguration;
  }

  /**
   * Sets the network configuration filename.
   * 
   * @param netConfig
   *          the new network configuration filename.
   */
  public void setNetworkConfiguration(final String netConfig) {
    this.networkConfiguration = netConfig;
  }

  /**
   * Gets the GUID mapping configuration filename.
   * 
   * @return the GUID mapping configuration filename.
   */
  public String getMappingConfiguration() {
    return this.mappingConfiguration;
  }

  /**
   * Sets the GUID mapping configuration filename.
   * 
   * @param mapConfig
   *          the new filename.
   */
  public void setMappingConfiguration(final String mapConfig) {
    this.mappingConfiguration = mapConfig;
  }

  /**
   * Gets the GUID store type value.
   * 
   * @return the type of GUID store to use.
   */
  public String getStoreType() {
    return this.storeType;
  }

  /**
   * Sets the GUID store type value.
   * 
   * @param storeType
   *          the new type of GUID store to use.
   */
  public void setStoreType(final String storeType) {
    this.storeType = storeType;
  }

  /**
   * Gets the filename of the GUID store configuration.
   * 
   * @return the GUID store configuration filename.
   */
  public String getStoreConfiguration() {
    return this.storeConfiguration;
  }

  /**
   * Sets the GUID store configuration filename.
   * 
   * @param storeConfig
   *          the GUID store configuration filename.
   */
  public void setStoreConfiguration(final String storeConfig) {
    this.storeConfiguration = storeConfig;
  }

  /**
   * Get the timeout value for messages sent to remote servers. 
   * @return the timeout value in milliseconds.
   */
  public long getTimeoutMillis() {
    return this.timeoutMillis;
  }

  /**
   * Sets the timeout value for messages sent to remote servers.
   * @param timeoutMillis the new timeout value in milliseconds.
   */
  public void setTimeoutMillis(final long timeoutMillis) {
    this.timeoutMillis = timeoutMillis;
  }

  /**
   * Gets the number of retries made for timed-out messages.
   * @return the number of retries for each message.
   */
  public int getNumAttempts() {
    return this.numAttempts;
  }

  /**
   * Sets the number of retries made for each message.
   * @param numRetries the new number of retries. A negative value is interpreted as 0.
   */
  public void setNumAttempts(final int numRetries) {
    this.numAttempts = numRetries;
  }

  /**
   * Get the number of GUID values stored in the local cache.
   * @return the size of the cache, as the number of GUID entries.
   */
  public int getCacheEntries() {
    return this.cacheEntries;
  }

  /**
   * Sets the size of the cache, as the number of GUID entries.
   * @param cacheEntries the new size
   */
  public void setCacheEntries(final int cacheEntries) {
    this.cacheEntries = cacheEntries;
  }

  /**
   * Returns the default binding expiration value.
   * @return the expiration value, in milliseconds.
   */
  public long getDefaultExpiration() {
    return this.defaultExpiration;
  }

  /**
   * Sets the default binding expiration value.
   * @param defaultExpiration the new expiration value, in milliseconds.
   */
  public void setDefaultExpiration(final long defaultExpiration) {
    this.defaultExpiration = defaultExpiration;
  }

  /**
   * Gets the default TTL value.
   * @return the TTL value, in milliseconds.
   */
  public long getDefaultTtl() {
    return this.defaultTtl;
  }

  /**
   * Sets the default TTL value.
   * @param defaultTtl the new TTL value, in milliseconds.
   */
  public void setDefaultTtl(final long defaultTtl) {
    this.defaultTtl = defaultTtl;
  }

  /**
   * Gets the statistics output directory.
   * @return the statistics file output directory.
   */
  public String getStatsDirectory() {
    return this.statsDirectory;
  }

  /**
   * Sets the statistics output directory.
   * @param statsDirectory the new directory path.
   */
  public void setStatsDirectory(final String statsDirectory) {
    this.statsDirectory = statsDirectory;
  }

  /**
   * Gets the replica selector type value.
   * @return the replica selector type.
   */
  public String getReplicaSelector() {
    return this.replicaSelector;
  }

  /**
   * Sets the replica selector type value.
   * @param replicaSelector the new type of the replica selector.
   */
  public void setReplicaSelector(final String replicaSelector) {
    this.replicaSelector = replicaSelector;
  }

}
