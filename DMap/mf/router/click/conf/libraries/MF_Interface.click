elementclass MF_Interface {
	$dev, $port, $arp_table |

	//L2 packet interfaces
	fd_in::FromDevice($dev, PROMISC false, SNIFFER true); //core port
	td_out::ToDevice($dev);


	//Core interface - port 0
	fd_in 
	        -> HostEtherFilter($core_dev, DROP_OWN true)
	        -> classifier::Classifier(12/27C0, -) //only IP-encap MF pkts, i.e.,  with IP Protocol ID = 0x05
	        -> SetTimestamp 
	        -> MF_Learn(IN_PORT 0, ARP_TABLE arp_tbl) // learn src eth, port
			-> Strip(14) // strip eth header; assumes no VLAN tag
			-> output
			
	classifier[1] -> Discard;
	

	outQ::Queue(65535); //L2 outgoing pkt queue for given port
	
	input
		-> MF_EtherEncap(0x27c0, $dev, ARP_TABLE arp_tbl) 
	    -> outQ 
	    -> td_out;
}