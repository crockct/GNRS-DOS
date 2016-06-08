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

package edu.rutgers.winlab.mfirst.storage.cache;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Simple implementation of an LRU cache based on LinkedHashMap.  Idea provided by Hank Gay on StackOverflow.com
 * 
 * Sourced from http://stackoverflow.com/questions/221525/how-would-you-implement-an-lru-cache-in-java-6
 * 
 * @author <a href="http://stackoverflow.com/users/4203/hank-gay">Hank Gay</a>
 * @author Robert Moore II
 *
 * @param <K> the key value, must implement {@code hashCode()}
 * @param <V> the value to store.
 */
public class LRUCache<K, V> extends LinkedHashMap<K, V> {
  
  /**
   * To be updated when the class members change.
   */
  private static final long serialVersionUID = 5148706907508646895L;
 
  /**
   * Maximum capacity of the cache.
   */
  private final int capacity;
  
  /**
   * Creates a new LRU cache with the specified capacity.
   * @param capacity the maximum capacity for this cache.
   */
  public LRUCache(int capacity)
  {
    super(capacity+1, 1.0f, true);
    this.capacity = capacity;
  }

  @Override
  protected boolean removeEldestEntry(final Map.Entry<K, V> entry)
  {
    return super.size() > this.capacity;
  }
}
