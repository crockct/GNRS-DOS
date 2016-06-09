# GNRS-DOS

This contains the code from DMap in the DMap/mf folder, with some tests and malicious clients that I've added. 

Code I wrote: 
bash_set_IP.sh - sets /root/mf/gnrs/jserver/rock-configs/baseline/net-ipv4.xml to have the current machine (ORBIT node) 's IP address. 
Can be run on startup by including in rc.local on the node image. 

generate_mobility.py - generates discrete time steps (15 minute) of mobility using Sookhyun's transition probability matrix from 
"Measurement and Modeling of User Transitioning Among Networks". After adding a random distribution across each 15-minute time step, 
this could be used to populate a client trace for experiments. 

DMap/mf/gnrs/jserver/rock-configs
    - prefixes.ipv4 associates IP space each AS is responsible for to AS number (128)
    - as-binding.ipv4 associates AS number to IP and port
    - server.xml - uses k=3 replicas, selects one at random for lookup, no caching enabled, 
        default GUID binding expiration 900000 ms = 15 min 

DMap/mf/gnrs/jserver/src/test/java/edu/rutgers/winlab/mfirst/mapping/ipv4udp/Rock_IPv4UDPGUIDMapTest - can be used to print out 
NAs for a GUID - ie. generate a Rainbow Table. Note that this assumes as-binding.ipv4 doesn't change based on which nodes are cooperating. 
