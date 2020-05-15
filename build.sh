rm -rf build
mkdir build
cd build
cmake .. -DEigen3_DIR=/home/edith/Software/eigen/build/install/share/eigen3/cmake -Dradical_DIR=/home/edith/Software/radical/build/install/lib/cmake/radical/ -DSophus_DIR=/home/edith/Software/Sophus/build/install/share/sophus/cmake -DSophus_INCLUDE_DIRS=/home/edith/Software/Sophus/build/install/include -Dradical_INCLUDE_DIRS=/home/edith/Software/radical/build/install/include
make -j8
