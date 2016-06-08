// Parameters:
// my_GUID
// GNRS_server_ip - listening ip on server
// GNRS_server_port - listening port on server
// GNRS_listen_ip - IP assoc w/ interface GNRS listens on
// GNRS_listen_port - response listening port for gnrs clients

resp_cache::GNRS_RespCache();

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

gnrs_svc::GNRS_Service($my_GUID, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, $GNRS_listen_port); 

//Paint incoming packets based on incoming port
// OFFSET 8 is the offset of GNRS_SERVICE_ANNO
in_port_painter::InPortPainter(OFFSET 8);
in_port_painter -> gnrs_svc;
//Switch that uses painted annotation to deliver to right requestor
out_port_switch::PaintSwitch(ANNO 8);
gnrs_svc -> out_port_switch;


gnrs_request_generator::GNRS_ReqGen(PERIOD 1, DELAY 1);

//Requestor 1: 
gnrs_request_generator[0] -> [0]in_port_painter; 
out_port_switch[0] -> [0]gnrs_request_generator;

gnrs_request_generator2::GNRS_ReqGen(PERIOD 1, DELAY 1, OFFSET 1000);

//Requestor 2: 
gnrs_request_generator2[0] -> [1]in_port_painter; 
out_port_switch[1] -> [0]gnrs_request_generator2;

gnrs_request_generator3::GNRS_ReqGen(PERIOD 1, DELAY 1, OFFSET 2000);

//Requestor 3: 
gnrs_request_generator3[0] -> [2]in_port_painter; 
out_port_switch[2] -> [0]gnrs_request_generator3;

