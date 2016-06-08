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
package edu.rutgers.winlab.mfirst.storage.bdb.cache;

import static org.junit.Assert.*;

import java.util.Collection;
import java.util.LinkedList;

import junit.framework.Assert;

import org.junit.Before;
import org.junit.Test;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.ipv4udp.IPv4UDPAddress;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;
import edu.rutgers.winlab.mfirst.storage.cache.CacheEntry;
import edu.rutgers.winlab.mfirst.storage.cache.CacheOrigin;
import edu.rutgers.winlab.mfirst.storage.cache.SimpleCache;

/**
 * @author Robert Moore
 */
public class SimpleCacheTest {
  
  private static final long TTL_VALUE = 1000;
  private static final long EXPIRE_VALUE = 5000;

  LinkedList<GUIDBinding> bindings = new LinkedList<GUIDBinding>();
  LinkedList<GUID> guids = new LinkedList<GUID>();
  SimpleCache cache = null;

  @Before
  public void resetBindings() {
    this.bindings.clear();
    this.guids.clear();

    long now = System.currentTimeMillis();

    for (int i = 0; i < 5; ++i) {
      GUIDBinding bind = new GUIDBinding();

      bind.setAddress(IPv4UDPAddress.fromInteger(100+i));
      bind.setExpiration(now + EXPIRE_VALUE);
      bind.setTtl(now + TTL_VALUE);
      bind.setWeight(0);
      this.bindings.add(bind);
      
      GUID guid = GUID.fromInt(i);
      this.guids.add(guid);
    }
  }

  @Test
  public void testSimplePutGet() {
    this.cache = new SimpleCache(this.guids.size());
    for(int i = 0; i < this.guids.size(); ++i){
      this.cache.put(this.guids.get(i),CacheOrigin.INSERT, this.bindings.get(i));
    }
    
    for(int i = 0; i < this.guids.size(); ++i){
      Collection<GUIDBinding> bound = this.cache.get(this.guids.get(i));
      Assert.assertNotNull(bound);
      Assert.assertFalse(bound.isEmpty());
      Assert.assertTrue(bound.size() == 1);
      Assert.assertTrue(bound.contains(this.bindings.get(i)));
    }
  }
  
  @Test
  public void testTtlValues(){
    this.cache = new SimpleCache(this.guids.size());
    for(int i = 0; i < this.guids.size(); ++i){
      this.cache.put(this.guids.get(i),CacheOrigin.INSERT,this.bindings.get(i));
    }
    
    try {
      Thread.sleep(TTL_VALUE+100);
    }catch(InterruptedException ie){
      Assert.fail("Unable to sleep required duration of time.");
    }
    
    for(int i = 0; i < this.guids.size(); ++i){
      Collection<GUIDBinding> bound = this.cache.get(this.guids.get(i));
      Assert.assertNull("Cache value did not get ejected (TTL) after " + TTL_VALUE +"ms.",bound);
    }
  }
  
  @Test
  public void testExpirationValues(){
    long now = System.currentTimeMillis();
    for(GUIDBinding bind : this.bindings){
      bind.setTtl(now + EXPIRE_VALUE*2);
    }
    
    this.cache = new SimpleCache(this.guids.size());
    for(int i = 0; i < this.guids.size(); ++i){
      this.cache.put(this.guids.get(i),CacheOrigin.INSERT,this.bindings.get(i));
    }
    
    try {
      Thread.sleep(TTL_VALUE+100);
    }catch(InterruptedException ie){
      Assert.fail("Unable to sleep required duration of time.");
    }
    
    for(int i = 0; i < this.guids.size(); ++i){
      Collection<GUIDBinding> bound = this.cache.get(this.guids.get(i));
      Assert.assertNotNull(bound);
      Assert.assertFalse(bound.isEmpty());
      Assert.assertTrue(bound.size() == 1);
      Assert.assertTrue(bound.contains(this.bindings.get(i)));
    }
    
    
    try {
      Thread.sleep(EXPIRE_VALUE+100);
    }catch(InterruptedException ie){
      Assert.fail("Unable to sleep required duration of time.");
    }
    

    for(int i = 0; i < this.guids.size(); ++i){
      Collection<GUIDBinding> bound = this.cache.get(this.guids.get(i));
      Assert.assertNull("Cache value did not get ejected (TTL) after " + TTL_VALUE +"ms.",bound);
    }
  }

  @Test
  public void testCacheSize(){
    this.cache = new SimpleCache(this.guids.size()-1);
    
    for(int i = 0; i < this.guids.size(); ++i){
      this.cache.put(this.guids.get(i),CacheOrigin.INSERT,this.bindings.get(i));
    }
    
    Collection<GUIDBinding> ejected = this.cache.get(this.guids.get(0));
    Assert.assertNull(ejected);
   
    for(int i = 1; i < this.guids.size(); ++i){
      Collection<GUIDBinding> bound = this.cache.get(this.guids.get(i));
      Assert.assertNotNull(bound);
      Assert.assertFalse(bound.isEmpty());
      Assert.assertTrue(bound.size() == 1);
      Assert.assertTrue(bound.contains(this.bindings.get(i)));
    }
  }
  
  @Test
  public void expireSome(){
    this.cache = new SimpleCache(1);
    GUIDBinding bound = this.bindings.get(0);
    bound.setTtl(System.currentTimeMillis()+EXPIRE_VALUE);
    
    this.cache.put(this.guids.get(0),CacheOrigin.INSERT,this.bindings.get(0),this.bindings.get(1));
    
    try {
      Thread.sleep(TTL_VALUE+100);
    }catch(InterruptedException ie){
      Assert.fail("Unable to sleep required duration of time.");
    }
    
    Collection<GUIDBinding> remaining = this.cache.get(this.guids.get(0));
    Assert.assertNotNull(remaining);
    Assert.assertFalse(remaining.isEmpty());
    Assert.assertTrue(remaining.size() == 1);
    Assert.assertTrue(remaining.contains(this.bindings.get(0)));
  }
  
}
