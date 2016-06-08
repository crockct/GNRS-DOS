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

import java.util.List;
import java.util.concurrent.Callable;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.messages.AbstractResponseMessage;
import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.messages.InsertResponseMessage;
import edu.rutgers.winlab.mfirst.messages.LookupMessage;
import edu.rutgers.winlab.mfirst.messages.LookupResponseMessage;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.messages.opt.RecursiveRequestOption;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Callable task for handling timeouts.
 * 
 * @author Robert Moore
 */
public class TimeoutTask implements Callable<Object> {
  /**
   * Logger for this class.
   */
  private static final transient Logger LOG = LoggerFactory
      .getLogger(TimeoutTask.class);

  /**
   * The information about the expired message.
   */
  private final transient RelayInfo info;

  /**
   * Maximum number of attempts before declaring a failure.
   */
  private final transient GNRSServer server;

  /**
   * Creates a new timeout task for the relay info.
   * 
   * @param server
   *          reference back to the server
   * @param info
   *          the information about the timed-out message
   */
  public TimeoutTask(final GNRSServer server, final RelayInfo info) {
    super();
    this.info = info;
    this.server = server;
  }

  @Override
  public Object call() throws Exception {
    // Too many failed attempts, respond with FAILURE
    if (this.info.getNumAttempts() >= this.server.getConfig().getNumAttempts()) {
      AbstractResponseMessage response = null;
      if (this.info.clientMessage instanceof LookupMessage) {
        response = new LookupResponseMessage();
        if (!this.info.responseAddresses.isEmpty()) {
          ((LookupResponseMessage) response)
              .setBindings(this.info.responseAddresses
                  .toArray(new NetworkAddress[] {}));
          response.setResponseCode(ResponseCode.SUCCESS);
        } else {
          LOG.info("One or more remote servers failed to respond for a Lookup Message {}.", this.info.clientMessage.getRequestId());
          response.setResponseCode(ResponseCode.FAILED);
        }
      } else if (this.info.clientMessage instanceof InsertMessage) {
        LOG.info("One or more remote servers failed to respond for an Insert Message {}.",this.info.clientMessage.getRequestId());
        response = new InsertResponseMessage();
        response.setResponseCode(ResponseCode.FAILED);
      }
      // We have a message to send
      if (response != null) {
        response.setOriginAddress(this.server.getOriginAddress());
        response.setRequestId(this.info.clientMessage.getRequestId());

        response.setVersion((byte) 0);

        this.server.sendMessage(response,
            this.info.clientMessage.getOriginAddress());
      }
    }
    // Can still reattempt the message
    else {
      // Lookup message
      if (this.info.clientMessage instanceof LookupMessage) {
        LookupMessage orig = (LookupMessage) this.info.clientMessage;

        Integer requestId = Integer.valueOf(this.server.getNextRequestId());

        LookupMessage relayMessage = new LookupMessage();
        relayMessage.setGuid(orig.getGuid());
        List<Option> options = orig.getOptions();
        if (!options.isEmpty()) {
          for (Option opt : options) {
            if (!(opt instanceof RecursiveRequestOption)) {
              relayMessage.addOption(opt);
            }
          }
        }
        relayMessage.finalizeOptions();
        relayMessage.setOriginAddress(this.server.getOriginAddress());
        relayMessage.setVersion((byte) 0);
        relayMessage.setRequestId(requestId.intValue());

        this.server.addNeededServer(requestId, this.info);
        this.info.markAttempt();
        this.server.sendMessage(relayMessage,
            this.info.remainingServers.toArray(new NetworkAddress[] {}));

      } else if (this.info.clientMessage instanceof InsertMessage) {
        InsertMessage orig = (InsertMessage) this.info.clientMessage;
        Integer requestId = Integer.valueOf(this.server.getNextRequestId());

        final InsertMessage relayMessage = new InsertMessage();
        relayMessage.setGuid(orig.getGuid());
        List<Option> options = orig.getOptions();
        if (!options.isEmpty()) {
          for (Option opt : options) {
            if (!(opt instanceof RecursiveRequestOption)) {
              relayMessage.addOption(opt);
            }
          }
        }
        relayMessage.finalizeOptions();
        relayMessage.setOriginAddress(this.server.getOriginAddress());
        relayMessage.setVersion((byte) 0);
        relayMessage.setRequestId(requestId.intValue());
        relayMessage.setBindings(orig.getBindings());

        this.server.addNeededServer(requestId, this.info);

        this.info.markAttempt();
        this.server.sendMessage(relayMessage,
            this.info.remainingServers.toArray(new NetworkAddress[] {}));

      } else {
        LOG.warn("Retry unhandled for type {}",
            this.info.clientMessage.getClass());
      }
    }
    return null;
  }
}
