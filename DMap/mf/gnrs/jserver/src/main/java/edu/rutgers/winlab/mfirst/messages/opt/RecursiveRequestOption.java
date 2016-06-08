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
package edu.rutgers.winlab.mfirst.messages.opt;

/**
 * Message option for recursive requests.
 * 
 * @author Robert Moore
 */
public class RecursiveRequestOption extends Option {

  /**
   * Type value for this option.
   */
  public static final byte TYPE = 0x00;

  /**
   * Length for this option.
   */
  public static final byte LENGTH = 0x02;

  private static final byte[][] BYTES = new byte[][] { { 0, 0 }, { 0, 1 } };

  /**
   * Flag to indicate recursion.
   */
  private final transient boolean recursive;

  /**
   * Creates a new option with the specified recursion value.
   * 
   * @param recursive
   *          {@code true} if a recursion is requested, else {@code false} for a
   *          non-recursive request.
   */
  public RecursiveRequestOption(final boolean recursive) {
    super(TYPE, LENGTH);
    this.recursive = recursive;
  }

  /**
   * A Boolean object indicating whether the message should be processed in a
   * recursive manner (from a client) or not (from another server).
   */
  @Override
  public Object getOption() {
    return Boolean.valueOf(this.recursive);
  }

  @Override
  public byte[] getBytes() {
    return this.recursive ? BYTES[1] : BYTES[0];
  }

  /**
   * Returns whether or not this message is recursive or not.
   * 
   * @return {@code true} if the message is recursive, else {@code false}.
   */
  public boolean isRecursive() {
    return this.recursive;
  }

}
