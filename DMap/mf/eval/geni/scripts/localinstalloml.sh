#!/bin/bash

# This will install the OML server and client binaries and libraries. 
# Note that the packages are available for specific distributions and
# need to be downloaded from particular locations 

# This script will also install Apache web server and php modules 

# The following will install OML modules on Ubuntu 12.04 LTS

sudo sh -c "echo 'deb http://download.opensuse.org/repositories/home:/cdwertmann:/oml/xUbuntu_12.04/ /' >> /etc/apt/sources.list.d/oml2.list"

wget -O /tmp/oml-release.key "http://download.opensuse.org/repositories/home:cdwertmann:oml/xUbuntu_12.04/Release.key"
sudo apt-key add - < /tmp/oml-release.key  

sudo apt-get -qq -y update
sudo apt-get -qq -y install oml2-server liboml2-dev


sudo apt-get -qq -y install apache2 php5 libapache2-mod-php5 php5-mysql php5-sqlite

sudo /etc/init.d/apache2 restart
