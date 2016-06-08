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
package edu.rutgers.winlab.mfirst.storage.bdb;

import java.io.File;
import java.util.TimerTask;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sleepycat.je.DatabaseException;
import com.sleepycat.je.DatabaseNotFoundException;
import com.sleepycat.je.Environment;
import com.sleepycat.je.EnvironmentConfig;
import com.sleepycat.persist.EntityStore;
import com.sleepycat.persist.PrimaryIndex;
import com.sleepycat.persist.StoreConfig;
import com.thoughtworks.xstream.XStream;

import edu.rutgers.winlab.mfirst.GNRSServer;
import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.storage.GNRSRecord;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;
import edu.rutgers.winlab.mfirst.storage.GUIDStore;

/**
 * Implementation of a GUID store using BerkeleyDB for both in-memory caching
 * and on-disk persistent storage.
 * 
 * @author Robert Moore
 */
public class BerkeleyDBStore implements GUIDStore {

  private static final String STORE_NAME = "GNRS GUIDStore";

  /**
   * TimerTask for reporting BerkeleyDB-related statistics.
   * 
   * @author Robert Moore
   */
  private static final class StatsTask extends TimerTask {
    /**
     * Logging for reporting statistics.
     */
    private static final Logger LOG_STATS = LoggerFactory
        .getLogger(StatsTask.class);

    /**
     * BerkeleyDB environment.
     */
    private final transient Environment env;

    /**
     * Previous cache miss value.
     */
    private transient long lastCacheMiss = 0l;

    /**
     * Creates a new task for the provided BerkeleyDB environment.
     * 
     * @param dbEnv
     *          the BDB environment.
     */
    public StatsTask(final Environment dbEnv) {
      super();
      this.env = dbEnv;
    }

    @Override
    public void run() {
      final long newCacheMiss = this.env.getStats(null).getNCacheMiss();
      LOG_STATS.info("BDB Cache Misses: {}",
          Long.valueOf(newCacheMiss - this.lastCacheMiss));
      this.lastCacheMiss = newCacheMiss;
    }
  }

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(BerkeleyDBStore.class);

  /**
   * Berkkeley DB environment for this instance.
   */
  private final transient Environment bdbEnvironment;

  /**
   * The Berkeley DB store object.
   */
  private final transient EntityStore bdbStore;

  /**
   * Reference back to the server instance.
   */
  private final transient GNRSServer server;

  /**
   * Timer task for recording BerkeleyDB-related statistics.
   */
  private transient StatsTask statsTask;

  /**
   * Primary index (GUID) for interacting with the BDB store.
   */
  private final transient PrimaryIndex<BDBGUID, BDBRecord> primaryIndex;

  /**
   * Flag to prevent accidental data destruction.
   */
  private boolean allowClear = false;

  /**
   * Creates a new GUID Store using a Berkeley DB to back it.
   * 
   * @param configFileName
   *          the name of the configuration file for this GUID store.
   * @param server
   *          reference to the server instance.
   * @throws DatabaseException
   *           if an exception occurs while opening the database.
   */
  public BerkeleyDBStore(final String configFileName, final GNRSServer server)
      throws DatabaseException {
    super();
    final XStream xStream = new XStream();
    final Configuration config = (Configuration) xStream.fromXML(new File(
        configFileName));
    final File bdbEnvDir = new File(config.getPathToFiles());

    if (!bdbEnvDir.exists()) {
      if (!bdbEnvDir.mkdirs()) {
        LOG.error("Unable to create some directories for BerkeleyDB environment.");
        throw new IllegalArgumentException(
            "Unable to create some directories for BerkeleyDB environment.");
      }
      if (!bdbEnvDir.canWrite()) {
        throw new IllegalArgumentException(
            "Unable write to directory for BerkeleyDB environment.");
      }
    } else if (bdbEnvDir.isFile()) {
      LOG.error(
          "Unable to create BerkeleyDB environment directory. File exists with same name: {}",
          configFileName);
      throw new IllegalArgumentException(
          String
              .format(
                  "Unable to create BerkeleyDB environment directory. File exists with same name: %s",
                  configFileName));

    }

    this.server = server;

    final EnvironmentConfig bdbEnvConfig = new EnvironmentConfig();
    bdbEnvConfig.setAllowCreate(true);
    bdbEnvConfig.setCacheSize(config.getCacheSizeMiB() * 1048576);

    // Open/create the DB environment
    this.bdbEnvironment = new Environment(bdbEnvDir, bdbEnvConfig);

    final StoreConfig bdbStoreConfig = new StoreConfig();
    bdbStoreConfig.setAllowCreate(true);

    // Open/create the actual data store
    this.bdbStore = new EntityStore(this.bdbEnvironment, STORE_NAME,
        bdbStoreConfig);

    // Retrieve the primary index
    this.primaryIndex = this.bdbStore.getPrimaryIndex(BDBGUID.class,
        BDBRecord.class);

  }

  @Override
  public GNRSRecord getBindings(final GUID guid) {
    //TODO temporary fix to not return expired entries
    long currentTime = System.currentTimeMillis();
    final BDBRecord record = this.primaryIndex.get(BDBGUID.fromGUID(guid));
    final GNRSRecord returnedRecord = new GNRSRecord(guid);
    if (record != null) {
      for (final BDBGUIDBinding binding : record.bindings) {
        GUIDBinding guidBinding = binding.toGUIDBinding();
        if (guidBinding.getExpiration() > currentTime) {
          LOG.debug("Binding {} expired at {}", guidBinding.getAddress(), guidBinding.getExpiration());
          returnedRecord.addBinding(guidBinding);
        }
        LOG.debug("Found valid binding {} expired at {}", guidBinding.getAddress(), guidBinding.getExpiration());
      }
    }
    return returnedRecord;
  }

  @Override
  public boolean appendBindings(final GUID guid, final GUIDBinding... bindings) {
    boolean success;
    final BDBGUID theGuid = BDBGUID.fromGUID(guid);
    // Retrieve the current value
    BDBRecord record = this.primaryIndex.get(theGuid);
    BDBGUIDBinding[] newBindings = null;

    // If the current value exists, extend it
    if (record == null || record.bindings == null) {
      newBindings = new BDBGUIDBinding[bindings.length];
    } else {
      newBindings = new BDBGUIDBinding[record.bindings.length + bindings.length];
      System.arraycopy(record.bindings, 0, newBindings, 0,
          record.bindings.length);
    }

    int offset = newBindings.length - bindings.length;

    for (int i = 0; i < bindings.length; ++i, ++offset) {
      newBindings[offset] = BDBGUIDBinding.fromGUIDBinding(bindings[i]);
    }

    // System.arraycopy(bindings, 0, newBindings, offset, bindings.length);

    // Wasn't even a record before, so make one.
    if (record == null) {
      record = new BDBRecord();
      record.guid = theGuid;
    }
    record.bindings = newBindings;
    try {
      this.primaryIndex.put(record);
      success = true;
    } catch (final DatabaseException dbe) {
      LOG.error("Unable to append binding in Berkeley DB store.", dbe);
      success = false;
    }
    return success;
  }

  @Override
  public void doInit() {
    if (this.server != null && this.server.isCollectStatistics()) {
      this.statsTask = new StatsTask(this.bdbEnvironment);
      this.server.getTimer()
          .scheduleAtFixedRate(this.statsTask, 10000l, 10000l);

    }

  }

  @Override
  public void doShutdown() {
    if (this.statsTask != null) {
      this.statsTask.cancel();
    }

    try {
      if (this.bdbStore != null) {
        this.bdbStore.close();
      }

    } catch (final DatabaseException dbe) {
      LOG.error("Unable to cleanly close Berkeley DB store.", dbe);
    }

    // Clean-up and close the environment
    try {
      if (this.bdbEnvironment != null) {
        this.bdbEnvironment.cleanLog(); // Clean the log before closing
        this.bdbEnvironment.close();
      }
    } catch (final DatabaseException dbe) {
      LOG.error("Unable to cleanly close Berkeley DB environment.", dbe);

    }
  }

  @Override
  public boolean replaceBindings(final GUID guid, final GUIDBinding... bindings) {
    boolean success;
    // Nothing to replace with, so just delete
    if (bindings == null) {
      this.primaryIndex.delete(BDBGUID.fromGUID(guid));
      success = true;
    } else {

      final BDBGUIDBinding[] newBindings = new BDBGUIDBinding[bindings.length];
      for (int i = 0; i < bindings.length; ++i) {
        newBindings[i] = BDBGUIDBinding.fromGUIDBinding(bindings[i]);
      }

      final BDBRecord record = new BDBRecord();
      record.guid = BDBGUID.fromGUID(guid);
      record.bindings = newBindings;

      try {
        this.primaryIndex.put(record);
        success = true;
      } catch (final DatabaseException dbe) {
        LOG.error("Unable to append binding in Berkeley DB store.", dbe);
        success = false;
      }
    }
    return success;
  }

  /**
   * Drops all data from persistent storage. DO NOT call this method unless you
   * want unrecoverable data loss.
   * <p>
   * Before calling this method, be sure to call {@link #enableClear()} first.
   * If {@code enableClear()} is not first invoked, then this method has no
   * effect.
   * </p>
   * 
   * @return {@code true} if the clear succeeded, else {@code false}.
   */
  public boolean clearStore() {
    boolean cleared = false;
    if (this.allowClear) {
      try {
        this.bdbEnvironment.truncateDatabase(null, STORE_NAME, false);
      } catch (DatabaseNotFoundException dnfe) {
        // Ignored, since this can occur if no write has taken place
      }
      cleared = true;
    }
    this.allowClear = false;
    return cleared;
  }

  /**
   * Permits the peristent store to be called by a subsequent call to
   * {@link #clearStore()}.
   */
  public void enableClear() {
    this.allowClear = true;
  }

}
