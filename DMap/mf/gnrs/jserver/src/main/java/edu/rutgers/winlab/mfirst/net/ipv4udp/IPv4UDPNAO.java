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

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.PortUnreachableException;
import java.net.SocketAddress;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.mina.core.filterchain.DefaultIoFilterChainBuilder;
import org.apache.mina.core.future.ConnectFuture;
import org.apache.mina.core.future.IoFutureListener;
import org.apache.mina.core.future.WriteFuture;
import org.apache.mina.core.service.IoHandlerAdapter;
import org.apache.mina.core.session.IdleStatus;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.core.write.WriteToClosedSessionException;
import org.apache.mina.filter.codec.ProtocolCodecFilter;
import org.apache.mina.transport.socket.DatagramSessionConfig;
import org.apache.mina.transport.socket.nio.NioDatagramAcceptor;
import org.apache.mina.transport.socket.nio.NioDatagramConnector;
import org.apache.mina.util.ConcurrentHashSet;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thoughtworks.xstream.XStream;

import edu.rutgers.winlab.mfirst.client.GeneratingClient;
import edu.rutgers.winlab.mfirst.messages.AbstractMessage;
import edu.rutgers.winlab.mfirst.messages.AbstractResponseMessage;
import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.messages.LookupMessage;
import edu.rutgers.winlab.mfirst.messages.LookupResponseMessage;
import edu.rutgers.winlab.mfirst.messages.MessageType;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.net.MessageListener;
import edu.rutgers.winlab.mfirst.net.NetworkAccessObject;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.SessionParameters;

/**
 * Network Access Object (NAO) implementation for IPv4/UDP GNRS networking
 * implementations.
 * 
 * @author Robert Moore
 */
public class IPv4UDPNAO extends IoHandlerAdapter implements NetworkAccessObject
// ,IoFutureListener<ConnectFuture>
{

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory.getLogger(IPv4UDPNAO.class);

  /**
   * Set of listeners for this NAO.
   */
  private final transient Set<MessageListener> listeners = new ConcurrentHashSet<MessageListener>();

  /**
   * Configuration options for this NAO.
   */
  private final transient Configuration config;

  private final transient Map<IoSession, NetworkAddress> sessions = new ConcurrentHashMap<IoSession, NetworkAddress>();

  private final transient Map<NetworkAddress, IPv4UDPParameters> connections = new ConcurrentHashMap<NetworkAddress, IPv4UDPParameters>();

  /**
   * Messages that are awaiting connections.
   */
  // private final transient Map<ConnectFuture, RelayInfo> awaitingConnect = new
  // ConcurrentHashMap<ConnectFuture, RelayInfo>();

  /**
   * Incoming datagram acceptor.
   */
  private final transient NioDatagramAcceptor acceptor;

  /**
   * For sending datagrams.
   */
  // private final transient NioDatagramConnector connector;

  private final transient IPv4UDPAddress listenAddress;

  private final transient InetSocketAddress listenSockAddr;

  // private final transient IPv4UDPAddress sendAddress;
  //
  // private final transient InetSocketAddress sendSockAddr;

  /**
   * Creates a new instance of network access object for IPv4/UDP networking.
   * 
   * @param configFilename
   *          name of configuration file for this NAO.
   * @throws IOException
   *           if an IOException occurs while binding to the listen port.
   */
  public IPv4UDPNAO(final String configFilename) throws IOException {
    super();
    this.config = this.loadConfiguration(configFilename);
    if (this.config == null) {
      throw new IllegalArgumentException("Unable to load configuration file \""
          + configFilename + "\".");
    }
    this.listenAddress = IPv4UDPAddress.fromASCII(this.config.getBindAddress()
        + ":" + this.config.getBindPort());

    this.listenSockAddr = new InetSocketAddress(this.config.getBindAddress(),
        this.config.getBindPort());

    // this.sendAddress = IPv4UDPAddress.fromASCII(this.config.getBindAddress()
    // + ":" + this.config.getSendPort());
    // this.sendSockAddr = IPv4UDPAddress.toSocketAddr(this.sendAddress);

    // Incoming messages (and direct replies)
    this.acceptor = new NioDatagramAcceptor();
    this.configureAcceptor();
    // Outgoing messages
    // this.connector = new NioDatagramConnector();
    // this.configureConnector();

  }

  private void configureAcceptor() throws IOException {

    this.acceptor.setHandler(this);

    final DefaultIoFilterChainBuilder chain = this.acceptor.getFilterChain();
    // For encoding/decoding our messages
    chain.addLast("gnrs codec", new ProtocolCodecFilter(
        new GNRSProtocolCodecFactory()));

    final DatagramSessionConfig sessionConfig = this.acceptor
        .getSessionConfig();
    sessionConfig.setReuseAddress(true);
    sessionConfig.setCloseOnPortUnreachable(true);
    sessionConfig.setIdleTime(IdleStatus.BOTH_IDLE, 1);
    sessionConfig.setIdleTime(IdleStatus.BOTH_IDLE, 1);

    // Bind to wildcard (all) interface
    if (this.config.getBindAddress() == null
        || this.config.getBindAddress().trim().length() == 0) {
      this.acceptor.bind(new InetSocketAddress(this.config.getBindPort()));
    }
    // Bind to a specific IP address.
    else {
      this.acceptor.bind(this.listenSockAddr);
    }
  }

  /**
   * Sets up the outgoing messages connector.
   * 
   * @throws IOException
   */
  // private void configureConnector() throws IOException {
  // this.connector.setHandler(this);
  // final DatagramSessionConfig sessionConfig = this.connector
  // .getSessionConfig();
  // sessionConfig.setReuseAddress(true);
  // sessionConfig.setCloseOnPortUnreachable(false);
  // sessionConfig.setIdleTime(IdleStatus.BOTH_IDLE, 1);
  // sessionConfig.setIdleTime(IdleStatus.BOTH_IDLE, 1);
  //
  // final DefaultIoFilterChainBuilder chain = this.connector.getFilterChain();
  // chain.addLast("gnrs codec", new ProtocolCodecFilter(
  // new GNRSProtocolCodecFactory()));
  // }

  /**
   * Loads this NAOs configuration file from the filename provided.
   * 
   * @param filename
   *          the name of the configuration file.
   * @return the loaded configuration.
   */
  private Configuration loadConfiguration(final String filename) {
    final XStream xStream = new XStream();
    return (Configuration) xStream.fromXML(new File(filename));
  }

  @Override
  public void addMessageListener(final MessageListener listener) {
    this.listeners.add(listener);
  }

  protected void actualSend(final SessionParameters parameters,
      final AbstractMessage message) {

    if (!(parameters instanceof IPv4UDPParameters)) {
      throw new IllegalArgumentException(
          "Not an instance of IPv4UDP networking parameters: " + parameters);
    }

    final IPv4UDPParameters params = (IPv4UDPParameters) parameters;
    final WriteFuture future = params.session.write(message);
    // LOG.info("Awaiting actual write of {} to {}", message, params.session);
    // if (!this.config.isAsynchronousWrite()) {
    // future.awaitUninterruptibly();
    // }
    // LOG.info("Wrote {} to {}", message, params.session);
  }

  @Override
  public void sendMessage(final AbstractMessage message,
      final NetworkAddress... destAddrs) {
    // LOG.info("Sending {} to {}", message, destAddrs);
    for (NetworkAddress destAddr : destAddrs) {
      IPv4UDPParameters params = this.connections.get(destAddr);

      if (params == null) {
        params = new IPv4UDPParameters();
        params.session = this.acceptor.newSession(
            IPv4UDPAddress.toSocketAddr(destAddr), this.listenSockAddr);
        this.sessions.put(params.session, destAddr);
        this.connections.put(destAddr, params);
        // LOG.info("Establishing connection to {}",
        // IPv4UDPAddress.toSocketAddr(destAddr));
        // final ConnectFuture future = this.connector.connect(
        // IPv4UDPAddress.toSocketAddr(destAddr), this.listenSockAddr);
        // RelayInfo info = new RelayInfo();
        // info.clientMessage = message;
        // info.remoteAddress = destAddr;
        // this.awaitingConnect.put(future, info);
        // future.addListener(this);
        // FIXME: Must call awaitUninterruptably. This is a known issue in MINA
        // <https://issues.apache.org/jira/browse/DIRMINA-911>
        // future.awaitUninterruptibly();
      }
      // Have an existing connection
      // else {

      this.actualSend(params, message);
      // }
    }

  }

  @Override
  public void endSession(final SessionParameters parameters) {
    if (!(parameters instanceof IPv4UDPParameters)) {
      throw new IllegalArgumentException(
          "Not an instance of IPv4UDP networking parameters: " + parameters);
    }
    final IPv4UDPParameters params = (IPv4UDPParameters) parameters;
    params.session.close(true);
  }

  @Override
  public void removeMessageListener(final MessageListener listener) {
    this.listeners.remove(listener);
  }

  @Override
  public boolean isLocal(final NetworkAddress netAddr) {

    return this.listenAddress.equals(netAddr);
  }

  @Override
  public NetworkAddress getOriginAddress() {
    return this.listenAddress;
  }

  /*
   * MINA stuff
   */

  @Override
  public void messageReceived(final IoSession session, final Object message) {
    if (message instanceof AbstractMessage) {
      AbstractMessage msg = (AbstractMessage) message;
      NetworkAddress origin = msg.getOriginAddress();
      // LOG.info("Received {} from {}", message, session);
      for (final MessageListener listener : this.listeners) {
        listener.messageReceived(null, (AbstractMessage) message);
      }

    } else {
      LOG.error("Received non-message object: {}", message);
    }
  }

  @Override
  public void sessionOpened(final IoSession session) {

  }

  @Override
  public void sessionIdle(final IoSession session, final IdleStatus idleStatus) {
    NetworkAddress netAddr = this.sessions.remove(session);
    if (netAddr != null) {
      this.connections.remove(netAddr);
    }
    session.close(true);
  }

  @Override
  public void sessionClosed(final IoSession session) {
    NetworkAddress addr = this.sessions.remove(session);
    if (addr != null) {
      this.connections.remove(addr);
    }

  }

  @Override
  public void exceptionCaught(final IoSession session, final Throwable cause) {
    if (!(cause instanceof PortUnreachableException || cause instanceof WriteToClosedSessionException)) {
      LOG.error("Caught exception for " + session + ".", cause);
    }

    NetworkAddress addr = this.sessions.remove(session);
    if (addr != null) {
      this.connections.remove(addr);
    }
  }

  @Override
  public void doShutdown() {
    this.acceptor.dispose(true);
  }

  // @Override
  // public void operationComplete(ConnectFuture future) {
  // RelayInfo info = this.awaitingConnect.remove(future);
  // if (info == null) {
  // LOG.error("Unable to find correct message for {}", future);
  // } else {
  // IPv4UDPParameters params = new IPv4UDPParameters();
  // params.session = future.getSession();
  // info.serverParams = params;
  // this.connections.put(info.remoteAddress, info.serverParams);
  // this.sessions.put(params.session, info.remoteAddress);
  // this.sendMessage(info.clientMessage, info.remoteAddress);
  // }
  // }

}
