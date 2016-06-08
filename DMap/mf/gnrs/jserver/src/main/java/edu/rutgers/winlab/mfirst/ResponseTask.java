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

import java.util.Iterator;
import java.util.concurrent.Callable;

import edu.rutgers.winlab.mfirst.messages.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.SessionParameters;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;
import edu.rutgers.winlab.mfirst.storage.cache.CacheOrigin;

/**
 * @author Robert Moore
 */
public class ResponseTask implements Callable<Object> {

  private static final Logger LOG = LoggerFactory.getLogger(ResponseTask.class);

  private final transient GNRSServer server;
  private final transient SessionParameters params;
  private final transient AbstractResponseMessage message;

  public ResponseTask(final GNRSServer server, final SessionParameters params,
      final AbstractResponseMessage message) {
    super();
    this.server = server;
    this.params = params;
    this.message = message;
  }

  @Override
  public Object call() throws Exception {
    final long startProc = System.nanoTime();
    GNRSServer.NUM_RESPONSES.incrementAndGet();
    // LOG.info("Using relay info for {}", respMsg);
    Integer reqId = Integer.valueOf((int) this.message.getRequestId());
    RelayInfo info = this.server.awaitingResponse.get(reqId);
    // LOG.info("[{}]Using relay info for {}", respMsg, info.clientMessage);
    // We are actually expecting this response
    if (info != null) {
      // LOG.info("Retrieved relay info");
      // This is a server we need a response from
      if (info.remainingServers.remove(this.message.getOriginAddress())) {
        // LOG.info("Removed {} from servers", respMsg.getOriginAddress());
        // Add the bindings (if any)
        if (this.message instanceof LookupResponseMessage) {
          LookupResponseMessage lrm = (LookupResponseMessage) this.message;
          for (NetworkAddress netAddr : lrm.getBindings()) {
            // LOG.info("Adding {} to LKR bindings.", lrm.getBindings());
            info.responseAddresses.add(netAddr);
          }
        }
        // If this was the last server, reply to the client
        if (info.remainingServers.isEmpty()) {
          // LOG.info("All servers have replied.");
          this.server.awaitingResponse.remove(reqId);

          if (info.clientMessage instanceof LookupMessage) {
            LookupResponseMessage lrm = new LookupResponseMessage();
            LookupMessage clientMsg = (LookupMessage) info.clientMessage;
            // Issue #13 Handle cache insert
            GUID guid = clientMsg.getGuid();

            GUIDBinding[] cachedBind = new GUIDBinding[info.responseAddresses
                .size()];
            int i = 0;
            long now = System.currentTimeMillis();
            long defaultTtl = now + this.server.getConfig().getDefaultTtl();
            long defaultExpire = now
                + this.server.getConfig().getDefaultExpiration();
            for (Iterator<NetworkAddress> iter = info.responseAddresses
                .iterator(); iter.hasNext(); ++i) {
              NetworkAddress addx = iter.next();
              cachedBind[i] = new GUIDBinding();
              cachedBind[i].setAddress(addx);
              // TODO: Proper TTL/expiration for responses
              cachedBind[i].setTtl(defaultTtl);
              cachedBind[i].setExpiration(defaultExpire);
            }

            this.server.addToCache(guid, CacheOrigin.LOOKUP_RESPONSE,
                cachedBind);

            lrm.setRequestId(info.clientMessage.getRequestId());
            lrm.setOriginAddress(this.server.getOriginAddress());
            lrm.setResponseCode(ResponseCode.SUCCESS);
            lrm.setVersion((byte) 0x0);
            lrm.setBindings(info.responseAddresses
                .toArray(new NetworkAddress[] {}));
            // LOG.info("Going to send reply back to client: {}", lrm);
            this.server.sendMessage(lrm, info.clientMessage.getOriginAddress());
            if (this.server.getConfig().isCollectStatistics()) {
              long endProc = System.nanoTime();
              GNRSServer.LOOKUP_STATS[GNRSServer.QUEUE_TIME_INDEX]
                  .addAndGet((startProc - this.message.createdNanos)
                      + info.clientMessage.queueNanos);
              GNRSServer.LOOKUP_STATS[GNRSServer.PROC_TIME_INDEX]
                  .addAndGet(info.clientMessage.processingNanos);
              GNRSServer.LOOKUP_STATS[GNRSServer.REMOTE_TIME_INDEX]
                  .addAndGet(this.message.createdNanos
                      - info.clientMessage.forwardNanos);
              GNRSServer.LOOKUP_STATS[GNRSServer.RESP_PROC_TIME_INDEX]
                  .addAndGet(endProc - startProc);
              GNRSServer.LOOKUP_STATS[GNRSServer.TOTAL_TIME_INDEX]
                  .addAndGet((endProc - this.message.createdNanos)
                      + info.clientMessage.queueNanos
                      + info.clientMessage.processingNanos
                      + (this.message.createdNanos - info.clientMessage.forwardNanos));

            }
          } else if (info.clientMessage instanceof InsertMessage) {
            InsertResponseMessage irm = new InsertResponseMessage();
            irm.setRequestId(info.clientMessage.getRequestId());
            irm.setOriginAddress(this.server.getOriginAddress());
            irm.setResponseCode(ResponseCode.SUCCESS);
            irm.setVersion((byte) 0);
            this.server.sendMessage(irm, info.clientMessage.getOriginAddress());
            if (this.server.getConfig().isCollectStatistics()) {
              long endProc = System.nanoTime();
              GNRSServer.INSERT_STATS[GNRSServer.QUEUE_TIME_INDEX]
                  .addAndGet((startProc - this.message.createdNanos)
                      + info.clientMessage.queueNanos);
              GNRSServer.INSERT_STATS[GNRSServer.PROC_TIME_INDEX]
                  .addAndGet(info.clientMessage.processingNanos);
              GNRSServer.INSERT_STATS[GNRSServer.REMOTE_TIME_INDEX]
                  .addAndGet(this.message.createdNanos
                      - info.clientMessage.forwardNanos);
              GNRSServer.INSERT_STATS[GNRSServer.RESP_PROC_TIME_INDEX]
                  .addAndGet(endProc - startProc);
              GNRSServer.INSERT_STATS[GNRSServer.TOTAL_TIME_INDEX]
                  .addAndGet((endProc - this.message.createdNanos)
                      + info.clientMessage.queueNanos
                      + info.clientMessage.processingNanos
                      + (this.message.createdNanos - info.clientMessage.forwardNanos));

            }
          } else if (info.clientMessage instanceof UpdateMessage) {
              UpdateResponseMessage urm = new UpdateResponseMessage();
              urm.setRequestId(info.clientMessage.getRequestId());
              urm.setOriginAddress(this.server.getOriginAddress());
              urm.setResponseCode(ResponseCode.SUCCESS);
              urm.setVersion((byte) 0);
              this.server.sendMessage(urm, info.clientMessage.getOriginAddress());
              //TODO: for now not collecting UPDATE statistics
//              if (this.server.getConfig().isCollectStatistics()) {
//                  long endProc = System.nanoTime();
//                  GNRSServer.INSERT_STATS[GNRSServer.QUEUE_TIME_INDEX]
//                          .addAndGet((startProc - this.message.createdNanos)
//                                  + info.clientMessage.queueNanos);
//                  GNRSServer.INSERT_STATS[GNRSServer.PROC_TIME_INDEX]
//                          .addAndGet(info.clientMessage.processingNanos);
//                  GNRSServer.INSERT_STATS[GNRSServer.REMOTE_TIME_INDEX]
//                          .addAndGet(this.message.createdNanos
//                                  - info.clientMessage.forwardNanos);
//                  GNRSServer.INSERT_STATS[GNRSServer.RESP_PROC_TIME_INDEX]
//                          .addAndGet(endProc - startProc);
//                  GNRSServer.INSERT_STATS[GNRSServer.TOTAL_TIME_INDEX]
//                          .addAndGet((endProc - this.message.createdNanos)
//                                  + info.clientMessage.queueNanos
//                                  + info.clientMessage.processingNanos
//                                  + (this.message.createdNanos - info.clientMessage.forwardNanos));
//
//              }
          }else {
            LOG.error("Unsupported message received?");
          }
        } else {
          // LOG.info("Awaiting servers: {}", info.remainingServers);
        }
      } else {
        LOG.warn("Unable to find relay info for {}", this.message);
      }
    }

    return null;
  }

}
