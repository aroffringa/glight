#!/bin/bash
# This script runs the commands to update the strings from a container and updates
# the Python docstring files in your repository to match with the C++ code.
# The clang mkdocs software can be a bit tricky to install in user environment, so
# running this in a container avoids requiring the tools locally.
scriptdir=`dirname $0`
glightdir=`realpath ${scriptdir}/../..`

sudo docker build -t glight-package-ubuntu24 -f Ubuntu24-build-package ${glightdir}
mkdir -p ${glightdir}/build/package
sudo docker run --mount src=${glightdir}/build/package,target=/package,type=bind glight-package-ubuntu24 /usr/bin/bash -c "mv /build/glight-0.9.0-Linux.deb /package/glight-0.9.0-Ubuntu24.deb"
echo Package was written to ${glightdir}/build/package/
sudo docker build -f Ubuntu24-test-deb ../..

sudo docker build -t glight-package-ubuntu22 -f Ubuntu22-build-package ${glightdir}
mkdir -p ${glightdir}/build/package
sudo docker run --mount src=${glightdir}/build/package,target=/package,type=bind glight-package-ubuntu22 /usr/bin/bash -c "mv /build/glight-0.9.0-Linux.deb /package/glight-0.9.0-Ubuntu22.deb"
echo Package was written to ${glightdir}/build/package/
sudo docker build -f Ubuntu22-test-deb ../..

