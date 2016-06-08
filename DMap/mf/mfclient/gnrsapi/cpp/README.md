# GNRS C++ API #
A C++ interface providing messaging based-access to the Global Name 
Resolution Service which provides a fast distributed directory to store 
and retrieve GUID-to-locator mappings for the MobilityFirst network 
architecture. It is intended as a prototype implementation and is
not suitable for commercial or enterprise use.

## API ##

Here's an example showing basic use of the api:

	/* required header files */
	#include "guid.h"
	#include "net_addr.h"
	#include "gnrs_cxx.h"

	.....

	int main() {

            /* local and server network endpoint addresses */
	    string server_addr_s("192.168.1.1:5001");
	    string local_addr_s("192.168.1.2:3001");

            /* Note: addresses can also be constructued as binary encoded
             * byte arrays.
             */
	    NetAddr server_addr(NET_ADDR_TYPE_IPV4_PORT, server_addr_s);
	    NetAddr local_addr(NET_ADDR_TYPE_IPV4_PORT, local_addr_s); 

            try {
                /* GNRS service constructor */
                Gnrs gnrs(server_addr, local_addr);

                /* use helper functions to prepare GUID */
                Guid guid = Guid::from_unsigned_int(23483098); 

                /* insert operation: stores id->locator mapping in GNRS */
                list<NetAddr> addrs;
                /* add list of locators (network addresses) to the GUID */
                addrs.push_back(local_addr);

                /* 
                 * The 'add' method appends specifed locators to set of other 
                 * locators previously mapped to the GUID. 
                 * To replace existing mappings, use 'replace' method (to be
                 * implemented).
                 */
                gnrs.add(guid, addrs);

                /*
                 * The 'lookup' operation retrieves current set of locator 
                 * mappings for a given GUID.
                 */
                list<NetAddr> lkup_addrs = gnrs.lookup(guid);

                for (list<NetAddr>::const_iterator it = lkup_addrs.begin(); 
                    it != lkup_addrs.end(); it++) {
                            cout << (*it).value << endl;
                }
            } catch (NetException &ne) {
                cerr << "ERROR: " << ne.what() << endl;
            }
		return 0;
	}

## Sample Clients ##
A few test apps/clients are included to test the API in folder test-apps/
- add_mappings.cpp, lookup.cpp, robustness_test.cpp

Each app requires the following primary arguments

	usage: ./app <srvr_ip:port> <lcl_ip:port> ...

* server address - This is the network address at which one of the service
  instances can be reached. Presently the API supports only IPv4-UDP endpoints 
  for messaging. Example server address: 192.168.1.1:5001. Obtain server 
  endpoint information from GNRS service operator.
* local address - Specify a local IPv4-UDP endpoint for receiving response
  messages from the server. E.g., 192.168.1.1:3000

Note the above address configurations indicate the client can be run on
the same machine as the server.

Unless otherwise noted in the application or in the source code, all 
other resources, including but not limited to source code, artwork, 
documentation, are copyright (C) 2013 Wireless Information Laboratory (WINLAB)
and Rutgers University.  All rights reserved.
