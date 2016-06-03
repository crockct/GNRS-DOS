#!/bin/bash
BEFORE_IP=$"<edu.rutgers.winlab.mfirst.net.ipv4udp.Configuration>
<!-- Port number for incoming UDP packets -->\n
  <bindPort>5000</bindPort>\n

  <!-- Interface IP address to bind to. Leave blank for 'any'. -->\n
  <bindAddress>"
AFTER_IP=$"</bindAddress>\n

  <!-- Flag to indicate if writes should be asynchronous (non-blocking) -->\n
  <asynchronousWrite>false</asynchronousWrite>\n
</edu.rutgers.winlab.mfirst.net.ipv4udp.Configuration>"
IP=`hostname -i`
printf "$BEFORE_IP$IP$AFTER_IP" > net-ipv4.xml

