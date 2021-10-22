#!/bin/bash
set -e -u -x

rm -rf /opt/python/cp35-cp35m
export NEST_CMAKE_BUILDWHEEL=ON

for PYBIN in /opt/python/*/bin; do
    "${PYBIN}/pip" install wheel cmake
    export PATH="${PYBIN}":$PATH
    which cmake
    "${PYBIN}/python" ./setup.py bdist_wheel
done

# Bundle external shared libraries into the wheels
/opt/python/cp39-cp39/bin/pip install auditwheel
for whl in ./dist/*.whl; do
    /opt/python/cp39-cp39/bin/auditwheel repair "$whl"
done
