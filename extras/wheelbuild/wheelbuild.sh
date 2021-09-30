BUILDPATH=build
BUILDWHEELPATH=build_wheel_env
mkdir $BUILDPATH
cd $BUILDPATH
cmake .. -DCMAKE_INSTALL_PREFIX=tmp_build_wheel_env
make install-nodoc
cd ..
export NEST_CMAKE_BUILDWHEEL=ON
python extras/wheelbuild/prep_wheel_env.py $BUILDWHEELPATH
rm -rf tmp_build_wheel_env
cd $BUILDWHEELPATH
