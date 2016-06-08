#!/bin/bash

USAGE="Usage:`basename $0` [<build_opts>={clean}]"

[[ $# -gt 1 ]] && echo $USAGE && exit 1

buildopt=""
if [ $# -gt 0 ]; then
    buildopt=$1
fi

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "ERROR: Can't find 'config'; abort" && exit 1
[[ ! -e $scriptdir/util.sh ]] && echo "ERROR: Can't find 'util.sh'; abort" && exit 1
source $scriptdir/config
source $scriptdir/util.sh

if [ ! "$buildopt" == "clean" ]; then
    banner "Installing system packages"
    sudo apt-get -qq -y update 

    #misc. tools required during installation only
    sudo apt-get -qq -y install git git-gui curl

    banner "Installing MF dependencies" 
    sudo apt-get -qq -y install libpcap-dev libpopt-dev openjdk-7-jdk maven 
fi

function install_oml() {

    banner "Installing OML client library"

    if [ -z "$(grep cdwertmann /etc/apt/sources.list)" ]; then
        sudo sh -c 'echo "deb http://download.opensuse.org/repositories/home:/cdwertmann:/oml/xUbuntu_12.04/ ./" >> /etc/apt/sources.list'
    fi
    curl -s "http://download.opensuse.org/repositories/home:/cdwertmann:/oml/xUbuntu_12.04/Release.key" | sudo apt-key add -
    sudo apt-get -qq -y update
    sudo apt-get -qq -y install liboml2 liboml2-dev
}

function pull_mfsrc_git() {
	#Deprecated: new git repo requires a private key to be distributed. Not usable until secure solution is implemented

    banner "Installing MF sources"

    #git config --global core.askpass /usr/lib/git-core/git-gui--askpass
    git config --global --unset core.askpass 

    if [ ! -d "$r_mfsrcdir/.git" ]; then
        mkdir -p $r_mfsrcdir
        git clone $mfgitorigin $r_mfsrcdir
        cd $r_mfsrcdir
        git checkout $mfbranch
    else 
        cd $r_mfsrcdir
        git fetch origin
        git checkout $mfbranch
        git merge "origin/$mfbranch"
    fi
}

function pull_mfsrc() {

    banner "Installing MF sources"

    if [ ! -d "$r_mfsrcdir" ]; then
        mkdir -p $r_mfsrcdir
        wget $mf_webaddr
		tar xvf mobilityfirst-latest.tar.gz
		mf_ver=`tar tzf mobilityfirst-latest.tar.gz | sed -e 's@/.*@@' | uniq`
		cp -r $mf_ver/* $r_mfsrcdir/
                rm -r $mf_ver mobilityfirst-latest.tar.gz

    else 
        wget $mf_webaddr
		tar xvf mobilityfirst-latest.tar.gz
		mf_ver=`tar tzf mobilityfirst-latest.tar.gz | sed -e 's@/.*@@' | uniq`
		cp -r $mf_ver/* $r_mfsrcdir/
		rm -r $mf_ver mobilityfirst-latest.tar.gz
    fi
}

function pull_clicksrc() {

    banner "Installing Click sources"

    if [ ! -d "$r_clicksrcdir/.git" ]; then
        mkdir -p $r_clicksrcdir
        git clone git://github.com/kohler/click.git $r_clicksrcdir
        cd $r_clicksrcdir
        git checkout $clickversion
    else
        cd $r_clicksrcdir
        #git checkout master
        #git pull 
        git checkout $clickversion
    fi
}

function buildinstall_click(){

    banner "Building Click with MF elements"

    if [ "$buildopt" == "clean" ]; then
        #remove prev mf sources and do a clean mf build
        rm -rf $r_clicksrcdir/elements/local/mf* 
        rm -rf $r_clicksrcdir/elements/local/gnrs*
    else
        cd "$r_mfsrcdir/router/click/elements"
        # copy mf elements' sources
        cp gstar/* gnrs/* test/* utils/* $r_clicksrcdir/elements/local/
        # copy common headers 
        cp $r_mfsrcdir/common/include/* $r_clicksrcdir/elements/local
    fi

    cd $r_clicksrcdir
    # this is for clean rebuild of click
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        ./configure --disable-linuxmodule --enable-local --enable-user-multithread 
        make elemlist
        make
        sudo make install
    fi
}

function buildinstall_gnrs() {

    banner "Building GNRS" 

    export JAVA_HOME="$r_java_home"
    cd $r_mfsrcdir/gnrs/jserver
    echo -e -n "2\n1\n" | python install-to-project-repo.py -i 
    if [ "$buildopt" == "clean" ]; then
        mvn -q clean
        #rm log and data files generated during build tests
        rm -rf gnrsd.log* repo/
    else
        mvn -q package
        cp target/*.jar $r_jardir/
    fi
}

function buildinstall_gnrsapi() {

    banner "Building GNRS API" 

    cd $r_mfsrcdir/mfclient/gnrsapi/cpp
    export MF_HOME=$r_mfsrcdir
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make
        sudo make install
    fi
    
    #JNI library
    export JAVA_HOME="$r_java_home"
    cd $r_mfsrcdir/mfclient/gnrsapi/jni
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make
        sudo make install
    fi

    #build java api
    cd $r_mfsrcdir/mfclient/gnrsapi/java
    if [ "$buildopt" == "clean" ]; then
        mvn -q clean
    else
        mvn -q package
        cp target/*.jar $r_jardir/
    fi
}

function buildinstall_monitors() {

    banner "Building Click monitor" 

    cd $r_mfsrcdir/router/click/monitor
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make 
        sudo make install
    fi

    banner "Building Node monitor"

    cd $r_mfsrcdir/tools/node-mon
    if [ "$buildopt" == "clean" ]; then
        make clean
    else 
        make 
        sudo make install
    fi
}

function buildinstall_hoststack() {

    banner "Building MF host stack"

    cd $r_mfsrcdir/mfclient/hoststack/src
    export MF_HOME=$r_mfsrcdir
    #export OML=1
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make 
        sudo make install
    fi
}

function buildinstall_netapi() {

    banner "Building NetAPI" 

    cd $r_mfsrcdir/mfclient/netapi/c
    export MF_HOME=$r_mfsrcdir
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make
        sudo make install
    fi
    
    #JNI library
    export JAVA_HOME="$r_java_home"
    cd $r_mfsrcdir/mfclient/netapi/c/src/jni
    if [ "$buildopt" == "clean" ]; then
        make clean
    else
        make
        sudo make install
    fi

    #build java api
    cd $r_mfsrcdir/mfclient/netapi/java
    if [ "$buildopt" == "clean" ]; then
        mvn -q clean
    else
        mvn -q package
        cp target/*.jar $r_jardir/
    fi
}

install_oml
pull_mfsrc
pull_clicksrc

#build tasks that can be executed in parallel
ptasks=( buildinstall_click buildinstall_gnrs buildinstall_gnrsapi buildinstall_monitors buildinstall_hoststack buildinstall_netapi )

#keep an associative array to track the pids of the build tasks
declare -A pids

for task in ${ptasks[@]}
do

    $task > "/tmp/${task}.log" 2>&1 &
    pids[$task]=$!
done

banner "Building modules ..." "(may take several minutes, logs follow)"

for task in ${!pids[@]}
do
    wait ${pids[$task]} > /dev/null 2>&1
    tmplog="/tmp/${task}.log"
    echo
    cat $tmplog
    echo
    rm $tmplog
done
