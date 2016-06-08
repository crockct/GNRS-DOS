//GNRS insert/update/query handling
//requestor --> request Q --> gnrs client --> gnrs svc
//gnrs svc --> response Q --> gnrs client --> requestor
elementclass GNRS_Service {
	$my_GUID, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, $GNRS_listen_port |
	
	gnrs_reqQ::ThreadSafeQueue(100); //gnrs request queue - thread safe because of multiple requestors
	gnrs_rrh::GNRS_ReqRespHandler(MY_GUID $my_GUID, NET_ID "NA", RESP_LISTEN_IP $GNRS_listen_ip, 
	                              RESP_LISTEN_PORT $GNRS_listen_port, RESP_CACHE resp_cache); //gnrs client to interact with service
	gnrs_svc_sndr_lstnr::Socket(UDP, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, 
                                    $GNRS_listen_port, CLIENT true); //UDP request sender and response listener
	gnrs_respQ::Queue(100); //queue to hold responses from GNRS service

	//send requests to service
	input[0] -> gnrs_reqQ -> Unqueue -> [0]gnrs_rrh[0] -> [0]gnrs_svc_sndr_lstnr;  
	//recv & queue responses for processing & forwarding to requestors
	gnrs_svc_sndr_lstnr -> [1]gnrs_rrh[1] -> gnrs_respQ -> Unqueue -> [0]output;
}