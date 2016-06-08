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

import java.util.Collection;
import java.util.List;
import java.util.concurrent.Callable;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.messages.LookupMessage;
import edu.rutgers.winlab.mfirst.messages.LookupResponseMessage;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.opt.ExpirationOption;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.messages.opt.RecursiveRequestOption;
import edu.rutgers.winlab.mfirst.messages.opt.TTLOption;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.SessionParameters;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;

/**
 * Task to handle Lookup messages within the server. Designed to operate
 * independently of any other messages.
 * 
 * @author Robert Moore
 */
public class LookupTask implements Callable<Object> {
  /**
   * Logging facility for this class.
   */
  private static final Logger LOG = LoggerFactory.getLogger(LookupTask.class);

  /**
   * The received lookup message.
   */
  private final transient LookupMessage message;

  /**
   * Session parameters received with the message.
   */
  private final transient SessionParameters params;
  /**
   * The server processing the message.
   */
  private final transient GNRSServer server;

  /**
   * Creates a new lookup task for the specified server and lookup message
   * container.
   * 
   * @param server
   *          the server that is handling the message.
   * @param params
   *          the session-specific metadata.
   * @param message
   *          the message to process.
   */
  public LookupTask(final GNRSServer server, final SessionParameters params,
      final LookupMessage message) {
    super();
    this.params = params;
    this.message = message;
    this.server = server;
  }

  @Override
  public Object call() {
    final long startProc = System.nanoTime();
    GNRSServer.NUM_LOOKUPS.incrementAndGet();
    LOG.debug("Received lookup for guid {}", this.message.getGuid());

    Collection<GUIDBinding> cachedBindings = this.server.getCached(this.message
        .getGuid());
    if (cachedBindings != null && !cachedBindings.isEmpty()) {
      LOG.info("Cache HIT: {}->{}",this.message.getGuid(),cachedBindings);
      LookupResponseMessage response = new LookupResponseMessage();
      NetworkAddress[] addxes = new NetworkAddress[cachedBindings.size()];
      int i = 0;
      for (GUIDBinding b : cachedBindings) {
        addxes[i++] = b.getAddress();
      }
      response.setBindings(addxes);
      response.setOriginAddress(this.server.getOriginAddress());
      response.setRequestId(this.message.getRequestId());
      response.setResponseCode(ResponseCode.SUCCESS);
      response.setVersion((byte) 0);
      this.server.sendMessage(response, this.message.getOriginAddress());
    } else {

      final Collection<NetworkAddress> allAddxes = this.server.getMappings(
          this.message.getGuid(), this.message.getOriginAddress().getType());
      

      final LookupResponseMessage response = new LookupResponseMessage();
      response.setRequestId(this.message.getRequestId());

      if (allAddxes == null || allAddxes.isEmpty()) {
        response.setResponseCode(ResponseCode.FAILED);
      }

      boolean resolvedLocally = false;
      if (allAddxes != null && !allAddxes.isEmpty()) {
        for (final NetworkAddress addx : allAddxes) {
          // Loopback? Then the local server should handle it.
          if (this.server.isLocalAddress(addx)) {
            resolvedLocally = true;
            break;
          }

        }
      }

      boolean recursive = false;
      List<Option> options = this.message.getOptions();
      if (!options.isEmpty()) {
        for (Option opt : options) {
          if (opt instanceof RecursiveRequestOption) {
            recursive = ((RecursiveRequestOption) opt).isRecursive();
            break;
          }
        }
      }
      
      final List<NetworkAddress> serverAddxes = this.server.getPreferredReplicas(allAddxes);

      // No bindings were for the local server
      if (recursive && !resolvedLocally) {
        // this.message.setRecursive(false);

        RelayInfo info = new RelayInfo();
        info.clientMessage = this.message;
        info.remainingServers.addAll(serverAddxes);

        int requestId = this.server.getNextRequestId();

        LookupMessage relayMessage = new LookupMessage();
        relayMessage.setGuid(this.message.getGuid());
        for (Option opt : this.message.getOptions()) {
          if (!(opt instanceof RecursiveRequestOption)) {
            relayMessage.addOption(opt);
          }
        }
        relayMessage.finalizeOptions();
        relayMessage.setOriginAddress(this.server.getOriginAddress());
        relayMessage.setVersion((byte) 0);
        relayMessage.setRequestId(requestId);

        this.server.addNeededServer(requestId, info);

        this.server.sendMessage(relayMessage,
            serverAddxes.toArray(new NetworkAddress[] {}));
        if (this.server.getConfig().isCollectStatistics()) {
          long endProc = System.nanoTime();
          this.message.processingNanos = endProc-startProc;
          this.message.queueNanos = startProc-this.message.createdNanos;
          this.message.forwardNanos = endProc;
        }
        info.markAttempt();
      }
      // Either resolved at this server or non-recursive
      else {
        // Resolved at this server
        if (resolvedLocally) {
          GUIDBinding[] bindings = this.server.getBindings(this.message
              .getGuid());
          if (bindings != null) {
            long[] ttls = new long[bindings.length];
            long[] expires = new long[bindings.length];
            NetworkAddress[] addxes = new NetworkAddress[bindings.length];
            for (int i = 0; i < bindings.length; ++i) {
              addxes[i] = bindings[i].getAddress();
              ttls[i] = bindings[i].getTtl();
              expires[i] = bindings[i].getExpiration();
            }
            response.setBindings(addxes);
            response.addOption(new TTLOption(ttls));
            response.addOption(new ExpirationOption(expires));
          }
          response.setResponseCode(ResponseCode.SUCCESS);
          response.finalizeOptions();

        }
        // Non-local but not recursive, so a problem with the remote host
        else {
          response.setResponseCode(ResponseCode.FAILED);
        }
        response.setOriginAddress(this.server.getOriginAddress());

        // log.debug("[{}] Writing {}", this.container.session, response);
        this.server.sendMessage(response, this.message.getOriginAddress());
        
        if (this.server.getConfig().isCollectStatistics()) {
          long endProc = System.nanoTime();
          GNRSServer.LOOKUP_STATS[GNRSServer.QUEUE_TIME_INDEX].addAndGet(startProc
              - this.message.createdNanos);
          GNRSServer.LOOKUP_STATS[GNRSServer.PROC_TIME_INDEX].addAndGet(endProc
              - startProc);
          GNRSServer.LOOKUP_STATS[GNRSServer.TOTAL_TIME_INDEX].addAndGet(endProc
              - this.message.createdNanos);

        }

      }

    }
    

    return null;
  }
}
