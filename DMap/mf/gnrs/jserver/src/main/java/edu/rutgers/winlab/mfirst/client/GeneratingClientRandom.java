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

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Random;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.LockSupport;

import org.apache.mina.core.filterchain.DefaultIoFilterChainBuilder;
import org.apache.mina.core.future.ConnectFuture;
import org.apache.mina.core.future.IoFutureListener;
import org.apache.mina.core.future.WriteFuture;
import org.apache.mina.core.service.IoHandlerAdapter;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolCodecFilter;
import org.apache.mina.filter.executor.ExecutorFilter;
import org.apache.mina.transport.socket.DatagramSessionConfig;
import org.apache.mina.transport.socket.nio.NioDatagramAcceptor;
import org.apache.mina.transport.socket.nio.NioDatagramConnector;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thoughtworks.xstream.XStream;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.StatisticsCollector;
import edu.rutgers.winlab.mfirst.messages.LookupMessage;
import edu.rutgers.winlab.mfirst.messages.LookupResponseMessage;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.opt.RecursiveRequestOption;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.GNRSProtocolCodecFactory;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * A simple GNRS client that generates lots of lookup messages.
 * 
 * @author Robert Moore
 */
public class GeneratingClientRandom extends IoHandlerAdapter implements Runnable {

  /**
   * Logging facility for this class.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(GeneratingClientRandom.class);

  /**
   * Rough estimate of how precise the system nanosecond timer is.
   */
  // Linux supports a 50 microsecond precision
  private static final long SLEEP_PRECISION = System.getProperty("os.name")
      .equalsIgnoreCase("linux") ? 50000 : 1000000;

  private transient long lastReceiveTime = System.currentTimeMillis();

  /**
   * @param args
   * @throws InterruptedException
   *           if interrupted while waiting for the clients to finish.
   */
  public static void main(final String[] args) throws InterruptedException {
    if (args.length < 4) {
      printUsageInfo();
      return;
    }

    final XStream xStream = new XStream();

    final Configuration config = (Configuration) xStream.fromXML(new File(
        args[0]));
    LOG.debug("Loaded configuration file \"{}\".", args[0]);

    final int delay = Integer.parseInt(args[2]);
    final int numClients = Integer.parseInt(args[3]);
    final int numLookups = Integer.parseInt(args[1]);

    boolean verbose = false;
    if (args.length > 4) {
      for (int i = 4; i < args.length; ++i) {
        if ("-v".equalsIgnoreCase(args[i])) {
          verbose = true;
        }
      }
    }

    final GeneratingClientRandom[] clients = new GeneratingClientRandom[numClients];
    for (int i = 0; i < clients.length; ++i) {
      clients[i] = new GeneratingClientRandom(config, delay, numLookups, verbose);
    }

    final Thread[] threads = new Thread[numClients];

    for (int i = 0; i < clients.length; ++i) {
      threads[i] = new Thread(clients[i]);
    }

    LOG.info("Created {} clients.", Integer.valueOf(numClients));

    for (int i = 0; i < clients.length; ++i) {
      threads[i].start();
    }

    for (int i = 0; i < clients.length; ++i) {
      threads[i].join();
    }
  }

  /**
   * Prints out how to invoke this client from the command line.
   */
  public static void printUsageInfo() {
    System.out
        .println("Usage: <Config File> <Num Request> <Request Delay> <Num Clients> [-v]");
  }

  /**
   * Acceptor for accepting messages from the server.
   */
  private final transient NioDatagramAcceptor acceptor;

  /**
   * Configuration for this client.
   */
  private final transient Configuration config;

  /**
   * How long to wait between messages, in microseconds.
   */
  private final transient long delay;
  /**
   * Total number of lookup messages to generate.
   */
  private final transient int numLookups;

  /**
   * Total number of successes.
   */
  private final transient AtomicInteger numSuccess = new AtomicInteger(0);

  /**
   * Total number of responses with bindings.
   */
  private final transient AtomicInteger numHits = new AtomicInteger(0);

  /**
   * Total number of failures.
   */
  private final transient AtomicInteger numFailures = new AtomicInteger(0);

  private final Map<Integer, Long> sendTimes = new ConcurrentHashMap<Integer, Long>();

  private final Map<Integer, LookupMessage> sentMessages = new ConcurrentHashMap<Integer, LookupMessage>();

  private final Queue<Long> rtts = new ConcurrentLinkedQueue<Long>();

  /**
   * Flag for verbose (per-message) outputs.
   */
  private final transient boolean verbose;

  /**
   * Creates a new GeneratingClient with the configuration and delay values
   * provided.
   * 
   * @param config
   *          configuration for the client.
   * @param delay
   *          how long to wait (in microseconds) between messages.
   * @param numLookups
   *          total number of messages to send.
   * @param verbose
   *          flag to print each response to the log.
   */
  public GeneratingClientRandom(final Configuration config, final int delay,
      final int numLookups, boolean verbose) {
    super();
    this.verbose = verbose;
    this.config = config;
    this.delay = delay;
    this.numLookups = numLookups;

    this.acceptor = new NioDatagramAcceptor();
    this.acceptor.setHandler(this);
    DatagramSessionConfig sessionConfig = this.acceptor.getSessionConfig();
    sessionConfig.setReuseAddress(true);
    sessionConfig.setCloseOnPortUnreachable(false);
    DefaultIoFilterChainBuilder chain = this.acceptor.getFilterChain();
    chain.addLast("gnrs codec", new ProtocolCodecFilter(
        new GNRSProtocolCodecFactory()));

    LOG.info(String.format("Assuming timer precision of %,dns.",
        Long.valueOf(SLEEP_PRECISION)));

  }

  @Override
  public void run() {
    this.connect();
  }

  /**
   * Sets up the networking connection to the server.
   * 
   * @return {@code true} if everything goes well, else {@code false}.
   */
  public boolean connect() {
    boolean retValue = true;
    try {
      this.acceptor.bind(new InetSocketAddress(this.config.getClientHost(),
          this.config.getClientPort()));

      LOG.debug("Creating connect future.");
      IoSession session = this.acceptor.newSession(new InetSocketAddress(
          this.config.getServerHost(), this.config.getServerPort()),
          this.acceptor.getLocalAddress());

      GeneratingClientRandom.this.perform(session);

    } catch (IOException e) {
      LOG.error("Unable to bind to local port.", e);
      retValue = false;
    }
    return retValue;
  }

  /**
   * Sends the requests to the server.
   * 
   * @param session
   *          the session on which to send the messages.
   */
  public void perform(final IoSession session) {
    LOG.info("Connected to {}", session);
    this.generateLookups(session);

    while (this.lastReceiveTime > (System.currentTimeMillis() - 5000)) {
      try {
        Thread.sleep(500);
      } catch (final InterruptedException ie) {
        // Ignored
      }
    }
    this.printStats();
    session.close(true);
    this.acceptor.dispose(true);
  }

  private void printStats() {
    int length = this.rtts.size();
    ArrayList<Long> rttList = new ArrayList<Long>(length);
    rttList.addAll(this.rtts);

    Collections.sort(rttList);

    Long median = rttList.isEmpty() ? 0 : rttList.get(length / 2);
    if (!rttList.isEmpty()) {
      LOG.info(String.format("Min: %,dus | Med: %,dus | Max: %,dus",
          rttList.get(0) / 1000, median / 1000, rttList.get(length - 1) / 1000));
    }

    StatisticsCollector.setPath(this.config.getStatsDirectory());
    StatisticsCollector.toFiles();

    final int succ = GeneratingClientRandom.this.numSuccess.get();
    final int total = succ + GeneratingClientRandom.this.numFailures.get();

    LOG.info(String
        .format(
            "Sent: %,d  |  Received: %,d  |  Success: %,d  |  Bound: %,d  |  Loss: %,d",
            Integer.valueOf(numLookups), Integer.valueOf(total),
            Integer.valueOf(succ),
            Integer.valueOf(GeneratingClientRandom.this.numHits.get()),
            Integer.valueOf(GeneratingClientRandom.this.numLookups - total)));
  }

  /**
   * Creates a stream of lookup messages to send to the server.
   * 
   * @param session
   *          the session on which to send the messages.
   */
  private void generateLookups(final IoSession session) {
    LOG.info("Generating {} lookups.", Integer.valueOf(this.numLookups));

    try {
      final NetworkAddress clientAddress = IPv4UDPAddress.fromASCII(this.config
          .getClientHost() + ":" + this.config.getClientPort());

      LookupMessage message;
      long nextSend = System.nanoTime();
      long lastSend = 0l;
      final Random rand = new Random(this.config.getRandomSeed());

      for (int i = 0; i < this.numLookups; ++i) {

        message = new LookupMessage();
        message.addOption(new RecursiveRequestOption(true));
        message.finalizeOptions();

        message.setGuid(GUID.fromASCII("" + rand.nextInt(1000000)));
        message.setRequestId(i);
        message.setOriginAddress(clientAddress);
        lastSend = System.nanoTime();
        final WriteFuture future = session.write(message);
        this.sendTimes.put(Integer.valueOf(i), Long.valueOf(lastSend));
        if (this.verbose) {
          this.sentMessages.put(Integer.valueOf(i), message);
        }

        future.awaitUninterruptibly();

        nextSend = lastSend + (this.delay * 1000);
        final long waitTime = getNanoSleep(nextSend - System.nanoTime());

        if (waitTime > 0) {
          LockSupport.parkNanos(waitTime);
        }
      }
    } catch (final UnsupportedEncodingException uee) {
      LOG.error(
          "Unable to parse local host name from configuration parameter.", uee);
    }

  }

  /**
   * Determine how long to call on Lock.parkNanos(long) based on estimation of
   * the system sleep precision.
   * 
   * @param desiredSleep
   *          how long we would actually like to sleep.
   * @return the actual sleep time to use.
   */
  public static long getNanoSleep(final long desiredSleep) {
    final long halfPrecision = SLEEP_PRECISION / 4;
    final long roundHalf = desiredSleep / (halfPrecision);
    return roundHalf * (halfPrecision) - SLEEP_PRECISION;
  }

  @Override
  public void exceptionCaught(final IoSession session, final Throwable cause) {
    LOG.error("Caught unhandled exception.", cause);
  }

  @Override
  public void messageReceived(final IoSession session, final Object message) {
    final long rcvTime = System.nanoTime();
    this.lastReceiveTime = System.currentTimeMillis();
    if (message instanceof LookupResponseMessage) {
      this.handleResponse((LookupResponseMessage) message, rcvTime);
    }
  }

  /**
   * Handles a response message from the server.
   * 
   * @param msg
   *          the response message.
   */
  public void handleResponse(final LookupResponseMessage msg,
      final long recvNanos) {

    Long startNanos = this.sendTimes.remove(Integer.valueOf((int) msg
        .getRequestId()));
    if (startNanos != null) {
      long rtt = recvNanos - startNanos.longValue();

      this.rtts.add(Long.valueOf(rtt));

      StatisticsCollector.addValue("clt-lkp-rtt",
          rtt / 1000f);

      if (this.verbose) {
        LookupMessage sentMessage = this.sentMessages.remove(Integer
            .valueOf((int) msg.getRequestId()));
        LOG.info(String.format("[%,dns] %s -> %s", rtt, sentMessage.getGuid(),
            msg));
      }
    }
    if (ResponseCode.SUCCESS.equals(msg.getResponseCode())) {
      this.numSuccess.incrementAndGet();
      if (msg.getBindings() != null && msg.getBindings().length > 0) {
        this.numHits.incrementAndGet();
      }
    } else {
      this.numFailures.incrementAndGet();
    }
  }

  @Override
  public void sessionCreated(final IoSession session) {
    LOG.info("[{}] Session created.", session);
  }
}
