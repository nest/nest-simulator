BUILDPATH=build
BUILDWHEELPATH=build_wheel_env
mkdir $BUILDPATH
cd $BUILDPATH
cmake ..
cd ..
export NEST_CMAKE_BUILDWHEEL=ON
python extras/wheelbuild/prep_wheel_env.py $BUILDWHEELPATH
cd $BUILDWHEELPATH
python setup.py bdist_wheel
