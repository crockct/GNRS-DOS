#Here is a script that will create a diamond topology on eth0s i.e.
#                    ----- Node1-11 --
#                  /                            \
#                /                               \
#Node1-10                                    Node1-12
#                \                              /
#                   \                         /
#                      --- Node1-16--
#--------------------------
# setDataTrunk( "set of nodes", "array of VLANs", "native VLAN" )
setDataTrunk( ["node1-10.grid.orbit-lab.org"], [1001,1002], 8 )
setDataTrunk( ["node1-11.grid.orbit-lab.org"], [1001,1003], 8 )
setDataTrunk( ["node1-16.grid.orbit-lab.org"], [1002,1004], 8 )
setDataTrunk( ["node1-12.grid.orbit-lab.org"], [1003,1004], 8 )
Experiment.done
exit
#--------------------------

#Please make sure to use only VLAN range 1000-1100 (other VLANs might be used
#elsewhere and will cause problems if set on the data switches)