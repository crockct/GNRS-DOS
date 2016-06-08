<?php
class Udpipv4_endpoint {
        protected  $host;//should be a network address class
	
	protected  $port;
        
        protected  $socket;

	public   $clientport;

	public function __construct($host, $port){
	 $this->host = $host;
         $this->port = $port;
	// create the udp socket
	 $this->socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
        socket_bind( $this->socket,$this->host);
	socket_getsockname($this->socket,$this->host,$this->port);
	
	}

	function udp_send($buf){
	 
        // send message
	if(false!==($rc=socket_sendto($this->socket, $buf, strlen($buf), 0x100, $this->host, $this->port))){
		echo "Send $rc bytes by socket_sendto(). Closing socket";
	}else{
		echo "UDP Send Error! Reason: ". socket_strerror(socket_last_error()) ."\n";
	
	}

	 return $rc;
	}
	function udp_recv(){

	
	// receive message
	$buf;
	if(false !==($rn=socket_recv($this->socket, $buf, 30, MSG_WAITALL))){
		echo "Read $rn bytes from socket_recv(). Closing socket...";
		
                
	} else {
		echo "UDP Recv Error! Reason: ". socket_strerror(socket_last_error()) ."\n";
		
	}


	 return $buf;

	}

}

?>
