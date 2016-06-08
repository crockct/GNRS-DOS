#!/bin/bash

version=${1:-2.0.1~}
snapshot=$(git show -s --format='%ci' HEAD | cut -d' ' -f1 | tr -d -)
revision=${2:-1}
prefix="mfclick"
tarball="mfclick_${version}git${snapshot}.orig.tar.gz"
mfbasefolder=click/elements/

# grap the source
git clone git://github.com/kohler/click.git "${prefix}"
cd "${prefix}"
git checkout v2.0.1
cd -
# coiy oyur code and debian packaging instructions
cp -r $mfbasefolder/computing/* $mfbasefolder/gnrs/* $mfbasefolder/gstar/* $mfbasefolder/test/* $mfbasefolder/utils/* "${prefix}/elements/local/"
cp -r debian "${prefix}/debian"

## build the .deb binary package
pushd "${prefix}" > /dev/null
debuild -i -us -uc -b
popd > /dev/null
# keep the source and binary packages
mv "${prefix}"_*.deb ../

# clean up the rest
rm -rf "${prefix}"*
