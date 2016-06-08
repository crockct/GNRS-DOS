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
package edu.rutgers.winlab.mfirst.mapping.ipv4udp;

import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Locale;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Hashes GUID values based on Java-supported hashing algorithms. Incorporates
 * "chaining" in order to generate enough network address values. More
 * specifically, the GUID value is the input to the first digest value. When all
 * bytes of that digest have been consumed, then the GUID value is fed back into
 * the algorithm along with the MD5 output of the first hash. That digest is
 * then used, and so on until enough network addresses have been generated.
 * Please refer to the Java documentation for a <a href=
 * "http://docs.oracle.com/javase/6/docs/technotes/guides/security/StandardNames.html#MessageDigest"
 * >list of standard algorithms</a> available. Note that not all algorithms are
 * available on all JVM implementations.
 * 
 * @author Robert Moore
 */
public class MessageDigestHasher implements GUIDHasher {

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(MessageDigestHasher.class);

  /**
   * Name of the algorithm to use.
   */
  private final transient String algorithmName;

  /**
   * Thread-specific set of message digest objects.
   */
  private final transient ThreadLocal<MessageDigest> localDigest;

  /**
   * Thread-specific message digest for hashing.
   * @author Robert Moore
   *
   */
  private static final class MessageDigestThreadLocal extends
      ThreadLocal<MessageDigest> {
    
    /**
     * Error logger.
     */
    private static final Logger ERROR_LOG = LoggerFactory.getLogger(MessageDigestThreadLocal.class);
    /**
     * Message digest algorithm name.
     */
    private final transient String algorithmName;

    /**
     * The name of the message digest algorithm to use.
     * @param algorithmName
     */
    public MessageDigestThreadLocal(final String algorithmName) {
      super();
      this.algorithmName = algorithmName;
    }

    @Override
    public MessageDigest initialValue() {
      MessageDigest digest = null;
      try {
        digest = MessageDigest.getInstance(this.algorithmName);
      } catch (final NoSuchAlgorithmException nsae) {
        ERROR_LOG.error("Unable to create message digest from unsupported algorithm.", nsae);
      }
      return digest;
    }
  }

  /**
   * Creates a new message digest-based hashing object.
   * 
   * @param algorithmName
   *          the algorithm to use for hashing.
   */
  public MessageDigestHasher(final String algorithmName) {
    super();
    this.algorithmName = algorithmName.toUpperCase(Locale.getDefault());
    // Create a new MessageDigest for each thread.
    this.localDigest = new MessageDigestThreadLocal(this.algorithmName);
  }

  @Override
  public Collection<NetworkAddress> hash(final GUID guid,
      final AddressType type, final int numAddresses)
     {
    final ArrayList<NetworkAddress> addresses = new ArrayList<NetworkAddress>(
        numAddresses);
    final MessageDigest digest = this.localDigest.get();
    if (digest == null) {
      LOG.error("Unable to hash because \"{}\" is not supported.",
          this.algorithmName);
    } else {
      final int digestBytes = digest.getDigestLength();
      // Figure out how many bytes we need to buffer for our network addresses
      final int numberDigests = (int) Math.ceil((type.getMaxLength()
          * numAddresses * 1f)
          / digestBytes);
      final ByteBuffer buffer = ByteBuffer
          .allocate(numberDigests * digestBytes);

      // Initializes values to 0
      byte[] previousDigest = new byte[digestBytes];

      // Generate some bytes!
      for (int i = 0; i < numberDigests; ++i) {
        digest.update(guid.getBinaryForm());
        digest.update(previousDigest);
        previousDigest = digest.digest();
        buffer.put(previousDigest);
      }

      // Prepare the buffer for reading
      buffer.flip();

      // Generate the addresses and put them in the collection
      for (int i = 0; i < numAddresses; ++i) {

        final byte[] bytes = new byte[type.getMaxLength()];
        buffer.get(bytes);
        final NetworkAddress netAddr = new NetworkAddress(type, bytes);
        addresses.add(netAddr);
      }
    }
    return addresses;
  }

}
