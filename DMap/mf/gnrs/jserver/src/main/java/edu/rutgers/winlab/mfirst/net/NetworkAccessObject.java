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
package edu.rutgers.winlab.mfirst.net;

import edu.rutgers.winlab.mfirst.messages.AbstractMessage;

/**
 * Interface that provides network communication for the GNRS server.
 * 
 * @author Robert Moore
 */
public interface NetworkAccessObject {
  /**
   * Registers the listener to receive messages as they arrive from the network.
   * 
   * @param listener
   *          the listener to receive messages.
   */
  public void addMessageListener(final MessageListener listener);

  /**
   * Stops the listener from receiving messages as they arrive. Messages may
   * continue arrive after this method is called if they were buffered by the
   * NetworkAccessObject.
   * 
   * @param listener
   *          the listener to remove
   */
  public void removeMessageListener(final MessageListener listener);

  /**
   * Sends a message to the network.
   * 
   * @param parameters
   *          the parameters necessary to send the message
   * @param message
   *          the message to send
   */
//  public void actualSend(final SessionParameters parameters,
//      final AbstractMessage message);

  /**
   * Sends a message to the network.
   * 
   * @param destAddrs
   *          the destination addresses
   * @param message
   *          the message to send
   */
  public void sendMessage(
      final AbstractMessage message, final NetworkAddress... destAddrs);

  /**
   * Invoked when a session terminates and the NAO should clean-up any stored
   * state.
   * 
   * @param parameters
   *          session parameters.
   */
  public void endSession(final SessionParameters parameters);

  /**
   * Determine whether a specified network address object references the local
   * server.
   * 
   * @param address
   *          the address to check.
   * @return {@code true} if the NetworkAddress identifies this server, else
   *         {@code false}.
   */
  public boolean isLocal(final NetworkAddress address);

  /**
   * Returns this server's "origin" address as sent in request or response
   * messages.
   * 
   * @return the value of the "Origin Address" field for messages originating at
   *         this server.
   */
  public NetworkAddress getOriginAddress();

  /**
   * Called before the server exits so the network access can clean-up any
   * resources.
   */
  public void doShutdown();

}
