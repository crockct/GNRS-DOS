# GNRS Java Server #
A Java-based implementation of the GUID Name Resolution Service server for the
Mobility First project.  It is intended as a prototype implementation and is
not suitable for commercial or enterprise use.

## Building ##
Be sure you have Java 1.6 and Maven 2 or higher installed.  On Ubuntu systems,
you can do this with apt-get:

    # Ubuntu 12.04
    sudo apt-get install openjdk-7-jdk maven

    # Ubuntu 10.04
    sudo apt-get install openjdk-6-jdk maven2

Next you have to install the external JAR dependencies. A handy Python script
is provided to automate this task for you. Just run:

    python install-to-project-repo.py -i

When prompted, the version is "0.6" (option 2) and the artifactId is
"patricia-trie" (option 1).  Once that's installed, you can compile and build
the .jar file with:

    mvn clean package

The first time you compile, Maven will automatically resolve and download all
dependencies (be sure you have internet access).  It will then compile the
code and assemble the JAR.  The output will be in the `target/` subdirectory.

## Running the Server ##
The server requires a main configuration file as the only command-line
argument.  A sample is provided in `src/main/resources/server.xml`.  This
XML-based configuration file has comments describing each component and will
also define the location of any other files used by the server.

To run the server directly from the JAR with the provided configuration file,
I will assume you are in the `jserver/` directory:

    java -jar target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar \\
      src/main/resources/server.xml

Since this is a bit cumbersome to remember, a launch script (Bash),
`gnrsd-local` has been provided in the base directory and will pass any
arguments directly to the server.  Using this launch script, the above startup
command becomes:

    ./gnrsd-local src/main/resources/server.xml

## Logging Configuration ##
The server uses logging through the Log4J logging library (indirectly through
SLF4J).  By default, it will send INFO, WARNING, and ERROR message directly to
the console.  It will also log everything, including DEBUG and TRACE messages,
to a set of 10 files called _gnrsd-debug.log_, _gnrsd-debug.log.1_, etc.

To change the logging behavior, you must edit the log configuration file in
`src/main/resources/log4j.xml` and recompile/rebuild the JAR file.  The most
common change would be disabling console output, for example if you were not
monitoring the server directly.

## Test Client ##
A test client is included and can be run from the same JAR by specifying the
correct class `edu.rutgers.winlab.mfirst.client.TraceClient`.  Once
again, this is a bit unwieldy, so a convenient Bash script, `gbench` has been
created to simplify things.

The client takes 3 command-line arguments: client config, trace file, request
interval.

* Client config - Contains all of the runtime configuration options for the
  client. This includes things like the server hostname and port, local
  sending port, and more.  The configuration file is written in XML and fully
  commented.
* Trace file - Contains a set of messages to send and to the server.  The
  format will be more formally specified at a future date.
* Request Interval - The interval between messages, in milliseconds.

Once you've gotten all of your files in order, you're ready to go.  Assuming
you've just finished building the JAR file with Maven, you can launch the
client (using the included example files) like this:

    java -cp target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar \\
      edu.rutgers.winlab.mfirst.client.TraceClient \\
      src/main/resources/client.xml src/main/resources/example.trace 1

Of course, who can remember all that?  Why not copy your configuration and
trace files into the current directory and use the shell script?

    ./gbench client.xml example.trace 1

## Attributions and Licensing ##
* LRUCache.java - Authored by Hank Gay on StackOverflow. 
  http://stackoverflow.com/users/4203/hank-gay

Unless otherwise noted in the application or in the source code, all 
other resources, including but not limited to source code, artwork, 
documentation, are copyright (C) 2012 Wireless Information Laboratory (WINLAB)
and Rutgers University.  All rights reserved.
