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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.mina.core.filterchain.DefaultIoFilterChainBuilder;
import org.apache.mina.core.future.ConnectFuture;
import org.apache.mina.core.future.IoFutureListener;
import org.apache.mina.core.service.IoHandlerAdapter;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolCodecFilter;
import org.apache.mina.transport.socket.DatagramSessionConfig;
import org.apache.mina.transport.socket.nio.NioDatagramAcceptor;
import org.apache.mina.transport.socket.nio.NioDatagramConnector;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thoughtworks.xstream.XStream;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.StatisticsCollector;
import edu.rutgers.winlab.mfirst.messages.AbstractMessage;
import edu.rutgers.winlab.mfirst.messages.InsertMessage;
import edu.rutgers.winlab.mfirst.messages.InsertResponseMessage;
import edu.rutgers.winlab.mfirst.messages.LookupMessage;
import edu.rutgers.winlab.mfirst.messages.LookupResponseMessage;
import edu.rutgers.winlab.mfirst.messages.MessageType;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.opt.RecursiveRequestOption;
import edu.rutgers.winlab.mfirst.messages.opt.TTLOption;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.ipv4udp.GNRSProtocolCodecFactory;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;

/**
 * A simple GNRS client that sends GNRS messages based on a trace file.
 * 
 * @author Robert Moore
 */
public class TraceClient extends IoHandlerAdapter {

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory.getLogger(TraceClient.class);

  private final transient AtomicInteger numLookup = new AtomicInteger(0);

  private final transient AtomicInteger numLookupSuccess = new AtomicInteger(0);

  private final transient AtomicInteger numLookupFailures = new AtomicInteger(0);

  private final transient AtomicInteger numLookupHits = new AtomicInteger(0);

  private final Queue<Long> lookupRtts = new ConcurrentLinkedQueue<Long>();

  private final transient AtomicInteger numInsert = new AtomicInteger(0);

  private final transient AtomicInteger numInsertSuccess = new AtomicInteger(0);

  private final transient AtomicInteger numInsertFailures = new AtomicInteger(0);

  private final Queue<Long> insertRtts = new ConcurrentLinkedQueue<Long>();

  private final Map<Integer, Long> sendTimes = new ConcurrentHashMap<Integer, Long>();

  private final Map<Integer, AbstractMessage> sentMessages = new ConcurrentHashMap<Integer, AbstractMessage>();

  private transient long lastReceiveTime = System.currentTimeMillis();

  private transient boolean verbose = false;

  /**
   * Sends messages to a server based on a trace file.
   * 
   * @param args
   */
  public static void main(final String[] args) {
    if (args.length < 3) {
      printUsageInfo();
      return;
    }

    final XStream xStream = new XStream();

    final Configuration config = (Configuration) xStream.fromXML(new File(
        args[0]));
    LOG.debug("Loaded configuration file \"{}\".", args[0]);

    final File traceFile = new File(args[1]);

    LOG.debug("Loaded trace file \"{}\".", traceFile);

    final int delay = Integer.parseInt(args[2]);

    boolean verbose = false;
    if (args.length > 3) {
      for (int i = 3; i < args.length; ++i) {
        if ("-v".equalsIgnoreCase(args[i])) {
          verbose = true;
        }
      }
    }

    final TraceClient client = new TraceClient(config, traceFile, delay,
        verbose);

    LOG.debug("Configured trace client.");
    client.connect();
    LOG.debug("Finished main thread.");
  }

  /**
   * Information about how to call the application from the commandline.
   */
  public static void printUsageInfo() {
    System.out.println("Usage: <Config File> <Trace File> <Delay Value>");
  }

  /**
   * Acceptor for receiving messages from the server.
   */
  private final transient NioDatagramAcceptor acceptor;
  /**
   * Configuration for the client.
   */
  private final transient Configuration config;
  /**
   * Trace file containing the messages to send.
   */
  private final transient File traceFile;
  /**
   * How long to wait between messages (microseconds).
   */
  private final transient int delay;

  /**
   * Session for communicating with the server.
   */
  private transient IoSession session;

  /**
   * Flag to terminate early.
   */
  private transient boolean keepRunning = true;

  /**
   * Creates a new client with the specified configuration file, trace file, and
   * intermessage delay (microsecond).
   * 
   * @param config
   *          the configuration file for the client.
   * @param traceFile
   *          the set of messages to send.
   * @param delay
   *          how long to pause between messages, in microseconds.
   */
  public TraceClient(final Configuration config, final File traceFile,
      final int delay, boolean verbose) {
    super();

    this.verbose = verbose;
    this.config = config;
    this.traceFile = traceFile;
    this.delay = delay;

    this.acceptor = new NioDatagramAcceptor();
    this.acceptor.setHandler(this);
    DatagramSessionConfig sessionConfig = this.acceptor.getSessionConfig();
    sessionConfig.setReuseAddress(true);
    sessionConfig.setCloseOnPortUnreachable(false);
    DefaultIoFilterChainBuilder chain = this.acceptor.getFilterChain();
    chain.addLast("gnrs codec", new ProtocolCodecFilter(
        new GNRSProtocolCodecFactory()));
    Runtime.getRuntime().addShutdownHook(new Thread() {
      @Override
      public void run() {
        TraceClient.this.finishTrace();
      }
    });
  }

  /**
   * Sets-up the necessary networking components so that communication with the
   * server can begin.
   * 
   * @return {@code true} if everything goes well, else {@code false}.
   */
  public boolean connect() {
    boolean retValue = true;
    try {

      this.acceptor.bind(new InetSocketAddress(this.config.getClientHost(),
          this.config.getClientPort()));

      // LOG.debug("Creating connect future.");
      this.session = this.acceptor.newSession(
          new InetSocketAddress(this.config.getServerHost(), this.config
              .getServerPort()), this.acceptor.getLocalAddress());

      this.runTrace();

    } catch (IOException e) {
      LOG.error("Unable to bind to local port.", e);
      retValue = false;
    }

    return retValue;
  }

  /**
   * Reads the trace file and sends the messages it contains.
   * 
   * @param session
   *          the connection to the server.
   */
  public void runTrace() {
    LOG.info("Connected to {}", this.session);
    LOG.info("Starting trace from {}.", this.traceFile);

    String line = null;
    NetworkAddress fromAddress = null;
    try {
      final BufferedReader reader = new BufferedReader(new FileReader(
          this.traceFile));
      fromAddress = IPv4UDPAddress.fromASCII(this.config.getClientHost() + ":"
          + this.config.getClientPort());

      while (this.keepRunning && (line = reader.readLine()) != null) {
        line = line.trim();
        if (line.length() == 0 || line.charAt(0) == '#') {
          continue;
        }

        LOG.debug("FILE: {}", line);
        final AbstractMessage message = TraceClient.parseMessage(line);
        if (message == null) {
          LOG.warn("Unable to parse message from \"" + line + "\".");
          continue;
        }

        message.setOriginAddress(fromAddress);
        message.finalizeOptions();

        this.session.write(message);
        long sendTime = System.nanoTime();
        this.sentMessages.put(Integer.valueOf((int) message.getRequestId()),
            message);
        this.sendTimes.put(Integer.valueOf((int) message.getRequestId()),
            Long.valueOf(sendTime));
        if (message instanceof LookupMessage) {
          this.numLookup.incrementAndGet();
        } else if (message instanceof InsertMessage) {
          this.numInsert.incrementAndGet();
        }

        // try {
        java.util.concurrent.locks.LockSupport.parkNanos(this.delay * 1000l);
        // Thread.sleep(this.delay);
        // } catch (final InterruptedException ie) {
        // Ignored
        // }

      }

      reader.close();

    } catch (final UnsupportedEncodingException uee) {
      LOG.error(
          "Unable to parse local host name from configuration parameter.", uee);
      return;
    } catch (final IOException ioe) {
      LOG.error("Exception occurred while reading trace file.", ioe);

    }
    if (this.keepRunning) {
      this.finishTrace();
    }
  }

  protected void finishTrace() {
    this.keepRunning = false;
    LOG.info("Finished reading trace file. Waiting for outstanding messages.");
    while (this.lastReceiveTime > (System.currentTimeMillis() - 5000)) {
      try {
        Thread.sleep(500);
      } catch (final InterruptedException ie) {
        // Ignored
      }
    }
    this.session.close(true);
    this.acceptor.dispose(true);

    this.printStatistics();
  }

  private void printStatistics() {
    int length = this.insertRtts.size();
    ArrayList<Long> insList = new ArrayList<Long>(length);
    insList.addAll(this.insertRtts);
    Collections.sort(insList);

    long minInsRtt = insList.isEmpty() ? 0 : insList.get(0);
    long medInsRtt = insList.isEmpty() ? 0 : insList.get(length / 2);
    long maxInsRtt = insList.isEmpty() ? 0 : insList.get(length - 1);
    minInsRtt /= 1000;
    medInsRtt /= 1000;
    maxInsRtt /= 1000;

    int totalIns = this.numInsert.get();
    int succIns = this.numInsertSuccess.get();
    int failIns = this.numInsertFailures.get();
    int returnedIns = succIns + failIns;

    length = this.lookupRtts.size();
    ArrayList<Long> lookList = new ArrayList<Long>(length);
    lookList.addAll(this.lookupRtts);
    Collections.sort(lookList);

    long minLkpRtt = lookList.isEmpty() ? 0 : lookList.get(0);
    long medLkpRtt = lookList.isEmpty() ? 0 : lookList.get(length / 2);
    long maxLkpRtt = lookList.isEmpty() ? 0 : lookList.get(length - 1);

    minLkpRtt /= 1000;
    medLkpRtt /= 1000;
    maxLkpRtt /= 1000;

    int totalLkp = this.numLookup.get();
    int succLkp = this.numLookupSuccess.get();
    int failLkp = this.numLookupFailures.get();
    int returnedLkp = succLkp + failLkp;
    int hitLkp = this.numLookupHits.get();

    StatisticsCollector.setPath(this.config.getStatsDirectory());
    StatisticsCollector.toFiles();

    final String formatString = "\n==Insert==\n"
        + "Min: %,dus | Med: %,dus | Max: %,dus\n"
        + "Total: %,d  |  Success: %,d  |  Loss: %,d\n" + "==Lookup==\n"
        + "Min: %,dus | Med: %,dus | Max: %,dus\n"
        + "Total: %,d  |  Success: %,d  |  Bound: %,d  |  Loss: %,d\n";

    LOG.info(String.format(formatString, Long.valueOf(minInsRtt),
        Long.valueOf(medInsRtt), Long.valueOf(maxInsRtt),
        Integer.valueOf(totalIns), Integer.valueOf(succIns),
        Integer.valueOf(totalIns - returnedIns), Long.valueOf(minLkpRtt),
        Long.valueOf(medLkpRtt), Long.valueOf(maxLkpRtt),
        Integer.valueOf(totalLkp), Integer.valueOf(succLkp),
        Integer.valueOf(hitLkp), Integer.valueOf(totalLkp - returnedLkp)));

  }

  /**
   * Parses a message from the trace file.
   * 
   * @param asString
   *          a line from the trace file.
   * @return the parsed message, or {@code null} if none was parsed.
   */
  public static AbstractMessage parseMessage(final String asString) {
    LOG.debug("Parsing \"{}\"", asString);
    // Extract any comments and discard
    final String line = asString.split("#")[0];
    // final LinkedList<AbstractMessage> messages = new
    // LinkedList<AbstractMessage>();
    AbstractMessage message = null;
    final String[] generalComponents = line.split("\\s+");
    if (generalComponents.length >= 3) {

      // Sequence number
      final int sequenceNumber = Integer.parseInt(generalComponents[0]);
      // Type
      final MessageType type = MessageType.parseType(generalComponents[1]);
      // GUID
      GUID guid = null;
      try {
        guid = GUID.fromASCII(generalComponents[2]);

        switch (type) {
        case INSERT: {
          message = parseInsertMessage(guid, sequenceNumber, generalComponents);
          break;
        }
        case LOOKUP: {
          message = new LookupMessage();
          message.addOption(new RecursiveRequestOption(true));

          ((LookupMessage) message).setGuid(guid);
          message.setRequestId(sequenceNumber);
          break;
        }
        default:
          LOG.error("Unknown message type {}", type);
          break;
        }
      } catch (final UnsupportedEncodingException uee) {
        LOG.error("Unable to parse GUID value from string.", uee);
      }
    } else {
      LOG.error("Not enough components to parse from the line {}.",
          Integer.valueOf(generalComponents.length));
    }
    return message;
  }

  /**
   * Parses an Insert message from a trace file line.
   * 
   * @param guid
   *          the GUID for the message.
   * @param sequenceNumber
   *          the sequence number for the message
   * @param generalComponents
   *          the split line from the file.
   * @return an Insert Message parsed from the line, or {@code null} if parsing
   *         failed.
   */
  private static InsertMessage parseInsertMessage(final GUID guid,
      final int sequenceNumber, final String[] generalComponents) {

    InsertMessage insMsg = null;

    // Make sure there is something to split
    if (generalComponents.length < 4) {
      LOG.error("Missing GUID binding value.");

    } else {
      insMsg = new InsertMessage();
      final String[] bindingValues = generalComponents[3].split(",");
      if (bindingValues.length % 3 != 0) {
        LOG.error("Binding values are not a multiple of 3: {}",
            Integer.valueOf(bindingValues.length));
      }
      int numBindings = bindingValues.length / 3;
      NetworkAddress[] bindings = new NetworkAddress[numBindings];
      long[] ttlValues = new long[numBindings];
      // final NetworkAddress[] bindings = new NetworkAddress[];
      for (int i = 0; i < numBindings; ++i) {
        NetworkAddress netAddr = null;
        try {
          netAddr = IPv4UDPAddress.fromASCII(bindingValues[0]);
        } catch (final UnsupportedEncodingException uee) {
          LOG.error("Unable to parse network address from ASCII string.", uee);
          break;
        }
        // TODO: Need to figure-out how to send TTL values per-NA
        long ttl = Long.parseLong(bindingValues[1]);

        ttlValues[i] = ttl;
        bindings[i] = netAddr;
      }
      insMsg.setGuid(guid);
      insMsg.setRequestId(sequenceNumber);
      insMsg.setBindings(bindings);
      insMsg.addOption(new TTLOption(ttlValues));
      insMsg.addOption(new RecursiveRequestOption(true));

    }
    return insMsg;
  }

  @Override
  public void exceptionCaught(final IoSession session, final Throwable cause) {
    LOG.error("Caught unhandled exception.", cause);
  }

  @Override
  public void messageReceived(final IoSession session, final Object message) {
    final long rcvTime = System.nanoTime();
    this.lastReceiveTime = System.currentTimeMillis();
    // LOG.info("Received {} on {}", message, session);
    if (message instanceof LookupResponseMessage) {
      this.handleLookupResponse((LookupResponseMessage) message, rcvTime);
    } else if (message instanceof InsertResponseMessage) {
      this.handleInsertResponse((InsertResponseMessage) message, rcvTime);
    }
  }

  /**
   * Handles a response message from the server.
   * 
   * @param msg
   *          the response message.
   */
  public void handleLookupResponse(final LookupResponseMessage msg,
      final long recvNanos) {

    Long startNanos = this.sendTimes.remove(Integer.valueOf((int) msg
        .getRequestId()));
    if (startNanos != null) {
      long rtt = recvNanos - startNanos.longValue();
      StatisticsCollector.addValue("clt-lkp-rtt", rtt / 1000f);
      this.lookupRtts.add(Long.valueOf(rtt));

      if (this.verbose) {
        LookupMessage sentMessage = (LookupMessage) this.sentMessages
            .remove(Integer.valueOf((int) msg.getRequestId()));
        LOG.info(String.format("[%,dns] %s -> %s", rtt, sentMessage.getGuid(),
            msg));
      }
    }

    if (ResponseCode.SUCCESS.equals(msg.getResponseCode())) {
      this.numLookupSuccess.incrementAndGet();
      if (msg.getBindings() != null && msg.getBindings().length > 0) {
        this.numLookupHits.incrementAndGet();
      }
    } else {
      this.numLookupFailures.incrementAndGet();
    }
  }

  /**
   * Handles a response message from the server.
   * 
   * @param msg
   *          the response message.
   */
  public void handleInsertResponse(final InsertResponseMessage msg,
      final long recvNanos) {

    Long startNanos = this.sendTimes.remove(Integer.valueOf((int) msg
        .getRequestId()));
    if (startNanos != null) {

      long rtt = recvNanos - startNanos.longValue();
      StatisticsCollector.addValue("clt-ins-rtt", rtt / 1000f);
      this.insertRtts.add(Long.valueOf(rtt));

      if (this.verbose) {
        InsertMessage sentMessage = (InsertMessage) this.sentMessages
            .remove(Integer.valueOf((int) msg.getRequestId()));
        LOG.info(String.format("[%,dns] %s -> %s", rtt, sentMessage.getGuid(),
            msg));
      }
    }

    if (ResponseCode.SUCCESS.equals(msg.getResponseCode())) {
      this.numInsertSuccess.incrementAndGet();

    } else {
      this.numInsertFailures.incrementAndGet();
    }
  }

  @Override
  public void messageSent(final IoSession session, final Object message) {
    LOG.debug("[{}] Sent {}", session, message);
  }
}
