#!/bin/bash

log_config=""
jar_file=""
config_file=""

while getopts ":d:j:c:" o; do
    case "${o}" in
        d)
            log_config="-Dlog4j.configuration=${OPTARG}"
            ;;
        j)
            jar_file="-jar ${OPTARG}"
            ;;
        c)
            config_file="${OPTARG}"
            ;;
        *)
            echo "Wrong option -$OPTARG !!!"
            ;;
    esac
done

echo "/usr/lib/jvm/java-7-openjdk-i386/bin/java $log_config $jar_file $config_file"

/usr/lib/jvm/java-7-openjdk-i386/bin/java $log_config $jar_file $config_file
