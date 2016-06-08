elementclass MF_IPInterface {
	$dev, $dev_ip, $port, $arp_table |

	//L2 packet interfaces
	fd_in::FromDevice($dev, PROMISC false, SNIFFER true); //core port
	td_out::ToDevice($dev);


	fd_in 
	        -> HostEtherFilter($dev, DROP_OWN true)
	        -> classifier::Classifier(12/0800 23/05, -) //only IP-encap MF pkts, i.e.,  with IP Protocol ID = 0x05
	        -> SetTimestamp 
	        -> MF_Learn(IN_PORT $port, ARP_TABLE $arp_table) // learn src eth, port
			-> Strip(34) // strip eth and ip header; assumes no VLAN tag
			-> output
			
	classifier[1] -> Discard;
	
	outQ::Queue(65535); //L2 outgoing pkt queue for given port
	
	input
		//Use IP protocol ID=5 for encapsulating MF pkts  
		-> MF_IPEncap(PROTO 5, SRC $dev_ip, DST NXTHOP_ANNO, ARP_TABLE arp_tbl)
		-> MF_EtherEncap(0x0800, $edge_dev, ARP_TABLE arp_tbl) //set ethertype to IP 
	    -> outQ 
	    -> td_out;
}