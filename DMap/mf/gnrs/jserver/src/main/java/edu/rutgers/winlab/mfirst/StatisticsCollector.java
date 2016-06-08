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
package edu.rutgers.winlab.mfirst;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Statistics collector for a server.
 * 
 * @author Robert Moore
 */
public class StatisticsCollector {

  /**
   * Logging for this class.
   */
  private static final Logger LOG = LoggerFactory
      .getLogger(StatisticsCollector.class);

  private static final transient ConcurrentHashMap<String, ConcurrentLinkedQueue<Float>> VALUE_MAP = new ConcurrentHashMap<String, ConcurrentLinkedQueue<Float>>();
  private static String path = "";

  public static void setPath(final String newPath) {
    if (newPath != null) {
      path = newPath;
      if (!(path.isEmpty() || path.endsWith(File.separator))) {
        path = path + File.separator;
      }
    }
  }

  public static void addValue(final String name, final float value) {
    ConcurrentLinkedQueue<Float> queue = VALUE_MAP.get(name);
    if (queue == null) {
      queue = new ConcurrentLinkedQueue<Float>();
      VALUE_MAP.put(name, queue);
    }
    queue.add(Float.valueOf(value));
  }

  public static void clear(final String name) {
    VALUE_MAP.remove(name);
  }

  public static void clearAll() {
    VALUE_MAP.clear();
  }

  public static void toFiles() {
    File directory = new File(path);
    if (!directory.exists()) {
      directory.mkdirs();
    }

    for (String name : VALUE_MAP.keySet()) {
      LOG.info("Creating {}", path + name + ".csv");
      try {
        final File outFile = new File(path + name + ".csv");

        outFile.createNewFile();
        if (!outFile.canWrite()) {
          LOG.error("Unable to write to file {}", name);
          continue;
        }

        PrintWriter out = new PrintWriter(new FileWriter(outFile));

        ConcurrentLinkedQueue<Float> values = VALUE_MAP.get(name);
        if (values == null) {
          LOG.error("Missing values for {}", name);
          continue;
        }
        int size = values.size();
        ArrayList<Float> valueList = new ArrayList<Float>(size);
        valueList.addAll(values);
        Collections.sort(valueList);

        for (int percentile = 1; percentile <= 100; ++percentile) {
          float valueAtPerc = valueList.get(
              (int) (Math.ceil((percentile / 100f) * size)) - 1).floatValue();
          out.printf("%.2f, %.2f\n", valueAtPerc, percentile / 100f);
        }

        out.flush();
        out.close();

      } catch (IOException ioe) {
        LOG.error("Cannot write CDF file.", ioe);
        continue;
      }
    }
  }
}
