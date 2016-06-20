#!/bin/bash
cd /root/GNRS-DOS/DMap/mf/gnrs/jserver
java -Dlog4j.configuration=file:rock-configs/current/log4j.xml -jar target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar rock-configs/current/server.xml
