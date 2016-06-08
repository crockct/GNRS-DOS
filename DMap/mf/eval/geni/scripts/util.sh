#!/bin/bash

# assumes following are defined:
# Netfilter

# Filter node interfaces by specified network filter
Corenet=${Netfilter:0:$[ $(expr index $Netfilter '/') - 1 ]}
Corenetmask=${Netfilter:$(expr index $Netfilter '/')}

##
# Calc binary (32-bit int) representations of the network part of filter 
# for comparison with IP address of node interfaces

Corenetbin=$(echo $Corenet.$Corenetmask | perl -n -a -F\\. -e 'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')
#echo "Corenet:$Corenet::Corenetmask:$Corenetmask::Corenetbin:$Corenetbin";


Edgenet=${Edgefilter:0:$[ $(expr index $Edgefilter '/') - 1 ]}
Edgenetmask=${Edgefilter:$(expr index $Edgefilter '/')}
Edgenetbin=$(echo ${Edgenet}.$Edgenetmask | perl -n -a -F\\. -e 'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')


function banner() {

    echo
    echo -e "  ******************************************************"
    echo -e "    $1"
    [[ $# -eq 2 ]] && echo -e "    $2"
    echo -e "  ******************************************************"
    echo

}

function subbanner() {

    echo
    echo -e "  $1"
    [[ $# -ge 2 ]] && echo -e "  $2"
    [[ $# -eq 3 ]] && echo -e "  $3"
    echo -e "  ------------------------------------------------------"
    echo
}
