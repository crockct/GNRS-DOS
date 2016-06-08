#!/bin/bash

export MF_CLICK_LOG_LEVEL=5

threads=0
ctr_port=""
config_file=""
my_GUID=0
topo_file=""
core_dev=""
GNRS_server_ip=""
GNRS_server_port=0
GNRS_listen_ip=""
GNRS_listen_port=0
edge_dev=""
edge_dev_ip=""

while getopts ":t:c:C:m:f:d:s:p:i:P:D:I:" o; do
    case "${o}" in
        t)
            threads=${OPTARG}
            ;;
        c)
            ctr_port="--port ${OPTARG}"
            ;;
        C)
            config_file="${OPTARG}"
            ;;
        m)
            my_GUID=${OPTARG}
            ;;
        f)
            topo_file=${OPTARG}
            ;;
        d)
            core_dev=${OPTARG}
            ;;
        s)
            GNRS_server_ip=${OPTARG}
            ;;
        p)
            GNRS_server_port=${OPTARG}
            ;;
        i)
            GNRS_listen_ip=${OPTARG}
            ;;
        P)
            GNRS_listen_port=${OPTARG}
            ;;
        D)
            edge_dev=${OPTARG}
            ;;
        I)
            edge_dev_ip=${OPTARG}
            ;;
        *)
            echo "Wrong option -$OPTARG !!!"
            ;;
    esac
done

#EXAMPLE
echo "/usr/local/bin/click --threads $threads $ctr_port $config_file my_GUID=$my_GUID topo_file=$topo_file core_dev=$core_dev GNRS_server_ip=$GNRS_server_ip GNRS_server_port=$GNRS_server_port GNRS_listen_ip=$GNRS_listen_ip GNRS_listen_port=$GNRS_listen_port edge_dev=$edge_dev edge_dev_ip=$edge_dev_ip"

/usr/local/bin/click --threads $threads $ctr_port $config_file my_GUID=$my_GUID topo_file=$topo_file core_dev=$core_dev GNRS_server_ip=$GNRS_server_ip GNRS_server_port=$GNRS_server_port GNRS_listen_ip=$GNRS_listen_ip GNRS_listen_port=$GNRS_listen_port edge_dev=$edge_dev edge_dev_ip=$edge_dev_ip
