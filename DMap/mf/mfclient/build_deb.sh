#!/bin/bash

version="-0.1"
prefix="mfclient"

## build the .deb binary package
debuild -us -uc -b -i

