BUILDPATH=build
BUILDWHEELPATH=build_wheel_env
mkdir $BUILDPATH
cd $BUILDPATH
cmake ..
cd ..
python extras/wheelbuild/prep_wheel_env.py $BUILDWHEELPATH
export NEST_CMAKE_WHEELBUILD=wheelbuild
cd $BUILDWHEELPATH
python setup.py bdist_wheel
