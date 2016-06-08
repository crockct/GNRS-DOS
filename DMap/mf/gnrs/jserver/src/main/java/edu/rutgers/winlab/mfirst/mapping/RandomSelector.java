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
package edu.rutgers.winlab.mfirst.mapping;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * @author Robert Moore
 */
public class RandomSelector implements ReplicaSelector {

  /**
   * The number of random servers to return.
   */
  private final transient int numSelected;
  /**
   * Random number generator.
   */
  private final transient ThreadLocal<Random> random;

  /**
   * Convenience constructor that will return at most 2 randomly-selected
   * servers,
   * using the current system time as the random seed.
   */
  public RandomSelector() {
    this(2, System.currentTimeMillis());
  }

  /**
   * Convenience constructor that uses the supplied number of servers to
   * select and the current system time as the random seed.
   * 
   * @param numSelected
   *          the maximum number of servers to return.
   */
  public RandomSelector(final int numSelected) {
    this(numSelected, System.currentTimeMillis());
  }

  /**
   * Constructs a randomized server selector that returns the specified number
   * of
   * servers (or fewer), and uses the specified seed value for the (pseudo-)
   * random
   * number generator.
   * 
   * @param numSelected
   *          the maximum number of servers to return.
   * @param randomSeed
   *          the seed value for the random number generator.
   */
  public RandomSelector(final int numSelected, final long randomSeed) {
    super();
    // Don't permit initialization below 1
    this.numSelected = numSelected < 1 ? 1 : numSelected;
    this.random = new ThreadLocal<Random>() {
      @Override
      protected Random initialValue() {
        return new Random(randomSeed);
      }
    };
  }

  /**
   * Selects a random set of the NetworkAddresses by sequentially eliminating
   * random NetworkAddress
   * values from the original Collection. Therefore, the worst-case running time
   * is O(n) with respect
   * to the number of servers in the input.
   */
  @Override
  public List<NetworkAddress> getContactList(
      final Collection<NetworkAddress> servers) {
    final List<NetworkAddress> returnedList = new LinkedList<NetworkAddress>();
    returnedList.addAll(servers);
    final int max = Math.min(this.numSelected, servers.size());
    final Random rand = this.random.get();
    while (returnedList.size() > max) {
      // Remove an element at random
      returnedList.remove(rand.nextInt(returnedList.size()));
    }
    return returnedList;
  }

}
