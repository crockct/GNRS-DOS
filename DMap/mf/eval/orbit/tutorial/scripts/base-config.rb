# MobilityFirst ORBIT Tutorial - Base Config
# -----------------------------------------
#
# This file contains the base configuration required for all 3 exercises
# of MobilityFirst ORBIT tutorial
#
# It sets up the software components for router and host nodes and the 
# topology used in the tutorial exercises. The base 4-node topology is:
#
# Host1 ------- MFR1 ---- MFR2 ------- Host2
#
# The script also shows how to setup software groups, configure networking on
# the experiment nodes, and to bring up the corresponding software modules.

#We need an application to enable the monitor that reports oml stats
defApplication('MF-Router', 'router') {|app|
	app.shortDescription = "Click-based MobilityFirst Access Router"
	app.path = "/usr/local/src/mobilityfirst/eval/orbit/tutorial/scripts/ARWrapper.sh" 
	# click options
	app.defProperty('num_threads', 'number of threads', "-t",{:type => :integer, :mandatory => true, :default => 4, :order => 1})
	app.defProperty('ctrl_port', 'port for Click control socket', "-c",{:type => :integer, :order => 2})
	# click config file 
	app.defProperty('config_file', 'Click configuration file', "-C",{:type => :string,:mandatory=> true})
	# keyword parameters used in click config file
	app.defProperty('my_GUID', 'router GUID', "-m",{:type => :string, :mandatory => true})
	app.defProperty('topo_file', 'path to topology file', "-f",{:type => :string, :mandatory => true})
	app.defProperty('core_dev', 'core network interface', "-d",{:type => :string,:mandatory => true})
	app.defProperty('GNRS_server_ip', 'IP of local GNRS server', "-s",{:type => :string,:mandatory => true})
	app.defProperty('GNRS_server_port', 'Port of GNRS server', "-p",{:type => :string,:mandatory => true})
	app.defProperty('GNRS_listen_ip', 'IP to listen for GNRS response', "-i",{:type => :string,:default => "0.0.0.0"})
	app.defProperty('GNRS_listen_port', 'port to listen for GNRS response', "-P",{:type => :string,:default => "10001"})
	app.defProperty('edge_dev', 'edge network interface', "-D",{:type => :string,:mandatory => true})
	app.defProperty('edge_dev_ip', 'IP assigned to edge interface', "-I",{:type => :string,:mandatory => true})
}

defApplication('MF-GNRS', 'gnrs') {|app|
	app.shortDescription = "GNRS Server"
	app.path = "/usr/local/src/mobilityfirst/eval/orbit/tutorial/scripts/GNRSWrapper.sh" 
	app.defProperty('log4j_config_file', 'log 4j configuration file', "-d",{:type => :string, :order => 1})
	app.defProperty('jar_file', 'server jar file with all dependencies', "-j" ,{:type => :string, :mandatory=> true, :default => "/usr/local/src/mobilityfirst/gnrs/jserver/target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar", :order => 2})
	app.defProperty('config_file', 'server configuration file', "-c",{:type => :string, :mandatory=> true, :order => 3})
}

#Enable OML reporting by default
defApplication('MF-HostStack', 'hoststack') {|app|
	app.shortDescription = "MF host network stack"
	app.path = "/usr/local/bin/mfstack" 
	app.defProperty('log_level', 'log level', nil,{:type => :string, :mandatory => true, :order => 1, :default => "-e"}) # default is 'error'
	app.defProperty('config_file', 'stack configuration file', nil,{:type => :string, :mandatory => true, :order => 2})
}

num_routers = 2
num_hosts = 2

#Update with topology (talk with Ivan)
# all experiment nodes 
defTopology("universe") do|t|
  t.addNode('node20-1.grid.orbit-lab.org')
  t.addNode('node20-2.grid.orbit-lab.org')
  t.addNode('node19-1.grid.orbit-lab.org')
  t.addNode('node19-2.grid.orbit-lab.org')
end
allTopo = Topology['universe']

# router nodes
defTopology('router_universe') do |t|
  t.addNode('node19-1.grid.orbit-lab.org')
  t.addNode('node19-2.grid.orbit-lab.org')
end
routersTopo = Topology['router_universe']

# host nodes
defTopology('host_universe') do |t|
  t.addNode('node20-1.grid.orbit-lab.org')
  t.addNode('node20-2.grid.orbit-lab.org')
end
hostsTopo = Topology['host_universe']

#Update with data interface making sure that routers are in spoofing mode
# router configurations
router_guid = Array["1", "2"]
router_GNRS_if_netmask = '192.168.100.1'
router_threads = 4
GNRS_listen_ip = '0.0.0.0'
GNRS_listen_port = "10001"

router_ether_if_ip = Array['192.168.1.1', '192.168.2.1']

# GNRS configuration
GNRS_server_port = "5001"
GNRS_server_ip = '192.168.100.1'

# GNRS configuration file
GNRS_log_file = 'file:/usr/local/src/mobilityfirst/eval/orbit/tutorial/conf/gnrs/log4j.xml'
GNRS_conf_file = '/usr/local/src/mobilityfirst/eval/orbit/tutorial/conf/gnrs/server.xml'
GNRS_jar_file = "/usr/local/src/mobilityfirst/gnrs/jserver/target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar"

# host configurations
host_guid = Array["101","102"]
log_level = '-e'

data_netmask = '255.255.255.0'

#GUID-based connectivity graph; enforced within the Click router
rtr_topo_file = '/usr/local/src/mobilityfirst/eval/orbit/tutorial/conf/rtr_topo.tp'
hoststack_conf_file = ['/usr/local/src/mobilityfirst/eval/orbit/tutorial/conf/hoststack1.conf', '/usr/local/src/mobilityfirst/eval/orbit/tutorial/conf/hoststack2.conf']

#Click configuration file for MobilityFirst Access Router
click_conf = '/usr/local/src/mobilityfirst/router/click/conf/MF_IPAccessMultiRouter.click'

#interface Click router listens on
core_dev = 'eth0'
edge_dev = 'wlan0'

#Create router groups
for i in 1..num_routers
	defTopology("topo:router_#{i}") { |t|
		aNode = routersTopo.getNodeByIndex(i-1)
		t.addNode(aNode)
		info "Adding node: ", aNode, " router with GUID: #{i}\n"
	}
  
	defGroup("router_#{i}", "topo:router_#{i}") {|node|
		node.addApplication('MF-Router') {|app|
			app.setProperty('num_threads', router_threads)
			app.setProperty('config_file', click_conf)
			app.setProperty('my_GUID', router_guid[i-1])
			app.setProperty('topo_file', rtr_topo_file)
			app.setProperty('core_dev', core_dev)
			app.setProperty('GNRS_server_ip', GNRS_server_ip)
			app.setProperty('GNRS_server_port', GNRS_server_port)
			app.setProperty('GNRS_listen_ip', "192.168.100.#{i}")
			app.setProperty('GNRS_listen_port', GNRS_listen_port)
			app.setProperty('edge_dev', edge_dev)
			app.setProperty('edge_dev_ip', router_ether_if_ip[i-1])
		}

	  #If is the first router add the GNRS
	  if i == 1
      info "Router with GUID #{i} will also run the gnrs\n"
		  node.addApplication('MF-GNRS') {|app|
		    app.setProperty('log4j_config_file', GNRS_log_file)
		    app.setProperty('jar_file', GNRS_jar_file)
		    app.setProperty('config_file', GNRS_conf_file)
		  }
	  end
	
	  node.net.e0.ip = "192.168.100.#{i}"
	  node.net.e0.netmask = '255.255.255.0'
    
    node.net.w0.mode = "adhoc"
    node.net.w0.type = 'g'
    node.net.w0.channel = "11"
    node.net.w0.essid = "SSID_group_#{i}"
    node.net.w0.ip = "192.168.#{i}.1"
	}
end

#Create host groups
for i in 1..num_hosts
	defTopology("topo:host_#{i}") { |t|
		aNode = hostsTopo.getNodeByIndex(i-1)
		t.addNode(aNode)
		info "Adding node: ", aNode, " as client\n"
	}
  
	defGroup("host_#{i}", "topo:host_#{i}") {|node|
		node.addApplication('MF-HostStack') {|app|
			app.setProperty('config_file', hoststack_conf_file[i-1])
			app.setProperty('log_level', log_level)
		}
	  
    #node.net.e0.ip = "192.168.#{i}.#{i+100}"
	  #node.net.e0.netmask = '255.255.255.0'
    
    node.net.w0.mode = "adhoc"
    node.net.w0.type = 'g'
    node.net.w0.channel = "11"
    node.net.w0.essid = "SSID_group_#{i}"
    node.net.w0.ip = "192.168.#{i}.2"
	}
end

info 'Definition of resources completed'