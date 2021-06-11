# Copy the repository in a data folder, to be packaged by setuptools
rsync -av --progress . ./data --exclude data --exclude .git --exclude .github --exclude dist --exclude build
# Run CMake
mkdir build && cd build
pip install cython
sudo apt install -y \
  libgsl-dev \
  libltdl-dev \
  libncurses-dev \
  libreadline-dev
# Generate the `setup.py` build file
cmake .. -DCMAKE_INSTALL_PREFIX=$PWD/product
# Extract setup.py
mv pynest/setup.py ..
cd ..
# Extract the nest Python files
cp pynest/* .
# Use `setup.py` to package the data folder and nest code folder into a source
# distribution.
python setup.py sdist
