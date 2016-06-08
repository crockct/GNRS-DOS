package edu.rutgers.winlab.mfirst;

import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.UpdateMessage;
import edu.rutgers.winlab.mfirst.messages.UpdateResponseMessage;
import edu.rutgers.winlab.mfirst.messages.opt.ExpirationOption;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.messages.opt.RecursiveRequestOption;
import edu.rutgers.winlab.mfirst.messages.opt.TTLOption;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.net.SessionParameters;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;
import edu.rutgers.winlab.mfirst.storage.cache.CacheOrigin;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.Callable;

/**
 * Created by wontoniii on 4/26/15.
 */
public class UpdateTask implements Callable<Object> {

    /**
     * Logging for this class.
     */
    @SuppressWarnings("unused")
    private static final Logger LOG = LoggerFactory.getLogger(InsertTask.class);

    /**
     * The message and some metadata for this task.
     */
    private final transient SessionParameters params;

    /**
     * The received insert message.
     */
    private final transient UpdateMessage message;

    /**
     * The server that is handling the message.
     */
    private final transient GNRSServer server;

    /**
     * Creates a new InsertTask for the specified server and message container.
     *
     * @param server
     *          the server that received or is handling the message.
     * @param params
     *          the session-related metadata.
     * @param message
     *          the message to process
     */
    public UpdateTask(final GNRSServer server, final SessionParameters params,
                      final UpdateMessage message) {
        super();
        this.server = server;
        this.params = params;
        this.message = message;
    }

    @Override
    public Object call() {
        final long startProc = System.nanoTime();
//        GNRSServer.NUM_INSERTS.incrementAndGet();


        final Collection<NetworkAddress> serverAddxes = this.server.getMappings(
                this.message.getGuid(), this.message.getOriginAddress().getType());

        // Determine if the local server should also handle this insert
        boolean resolvedLocally = false;
        if (serverAddxes != null && !serverAddxes.isEmpty()) {
            for (Iterator<NetworkAddress> iter = serverAddxes.iterator(); iter
                    .hasNext();) {
                NetworkAddress addx = iter.next();
                // Loopback? Then the local server should handle it.
                if (this.server.isLocalAddress(addx)) {
                    iter.remove();
                    resolvedLocally = true;
                }
            }
        }
        boolean localSuccess = false;

        boolean recursive = false;
        List<Option> options = this.message.getOptions();
        long[] expirationTimes = null;
        long[] ttlValues = null;
        if (!options.isEmpty()) {
            for (Option opt : options) {
                if (opt instanceof RecursiveRequestOption) {
                    recursive = ((RecursiveRequestOption) opt).isRecursive();
                } else if (opt instanceof ExpirationOption) {
                    expirationTimes = ((ExpirationOption) opt).getExpiration();
                } else if (opt instanceof TTLOption) {
                    ttlValues = ((TTLOption) opt).getTtl();
                }
            }
        }

        // Insert to local server if mapped to it.
        if (resolvedLocally) {
//      LOG.info("Resolved locally.");
//            localSuccess = this.server.appendBindings(this.message.getGuid(),
//                    this.message.getBindings());
            //TODO make default expiration in settings, now 10 minutes
            long defaultExpiration = 600000 + System.currentTimeMillis();
            NetworkAddress []addresses = this.message.getBindings();
            GUIDBinding []bindings = new GUIDBinding[addresses.length];
            GUIDBinding binding = null;
            LOG.debug("Received update for guid {}", message.getGuid());
            int i = 0;
            for (final NetworkAddress a : addresses){
                binding = new GUIDBinding();
                binding.setAddress(a);
                if (ttlValues != null && ttlValues.length >= i) {
                    binding.setTtl(ttlValues[i]);
                } else {
                    binding.setTtl(0);
                }
                if (expirationTimes != null && expirationTimes.length >=i) {
                    binding.setExpiration(expirationTimes[i]);
                } else {
                    binding.setExpiration(defaultExpiration);
                }
                binding.setWeight(0);
                bindings[i] = binding;
                LOG.debug("containing na {} and expiration {}", binding.getAddress(), binding.getExpiration());
                i++;
            }
            localSuccess = this.server.replaceBindings(this.message.getGuid(), bindings);
        }
        // Insert into the cache if the insert came from a local client.
        if (!resolvedLocally && recursive && this.message.getBindings() != null) {
            final long now = System.currentTimeMillis();
            final long defaultTtl = now + this.server.getConfig().getDefaultTtl();
            final long defaultExpire = now + this.server.getConfig().getDefaultExpiration();
            GUIDBinding[] bindings = new GUIDBinding[this.message.getBindings().length];

            for (int i = 0; i < bindings.length; ++i) {
                NetworkAddress netAddr = this.message.getBindings()[i];
                GUIDBinding bind = new GUIDBinding();
                bind.setAddress(netAddr);
                if (expirationTimes != null) {
                    bind.setExpiration(expirationTimes[i]);
                }else{
                    bind.setExpiration(defaultExpire);
                }
                if (ttlValues != null) {
                    bind.setTtl(now+ttlValues[i]);
                }else{
                    bind.setTtl(defaultTtl);
                }
                bindings[i] = bind;
            }

            this.server.addToCache(this.message.getGuid(), CacheOrigin.INSERT,bindings);
        }

        // Corner case for only a single server.
        if (resolvedLocally && serverAddxes.isEmpty()) {
            recursive = false;
        }

        // Now send to the remote servers
        if (recursive) {
            // this.message.setRecursive(false);

            final RelayInfo info = new RelayInfo();
            info.clientMessage = this.message;
            info.remainingServers.addAll(serverAddxes);

            final int requestId = this.server.getNextRequestId();

            final UpdateMessage relayMessage = new UpdateMessage();
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
            relayMessage.setBindings(this.message.getBindings());

            if (serverAddxes != null) {
                this.server.addNeededServer(Integer.valueOf(requestId), info);
                this.server.sendMessage(relayMessage,
                        serverAddxes.toArray(new NetworkAddress[] {}));
                if (this.server.getConfig().isCollectStatistics()) {
                    long endProc = System.nanoTime();
                    this.message.processingNanos = endProc-startProc;
                    this.message.queueNanos = startProc-this.message.createdNanos;
                    this.message.forwardNanos = endProc;
                }
                info.markAttempt();
            } else {
                LOG.error("Invalid server addresses.  Cannot forward.");
            }
        }

        // Send a response back if there are no remote servers contacted
        if (resolvedLocally && !recursive) {
//      LOG.info("Received {}", this.message);
            UpdateResponseMessage response = new UpdateResponseMessage();
            response.setOriginAddress(this.server.getOriginAddress());
            response.setRequestId(this.message.getRequestId());
            response.setResponseCode(localSuccess ? ResponseCode.SUCCESS
                    : ResponseCode.FAILED);
            response.setVersion((byte) 0);

            this.server.sendMessage(response, this.message.getOriginAddress());

        }

        // Statistics

        if (this.server.getConfig().isCollectStatistics()) {
            long endProc = System.nanoTime();
            GNRSServer.INSERT_STATS[GNRSServer.QUEUE_TIME_INDEX].addAndGet(startProc
                    - this.message.createdNanos);
            GNRSServer.INSERT_STATS[GNRSServer.PROC_TIME_INDEX].addAndGet(endProc
                    - startProc);
            GNRSServer.INSERT_STATS[GNRSServer.TOTAL_TIME_INDEX].addAndGet(endProc
                    - this.message.createdNanos);

        }

        return null;
    }

}