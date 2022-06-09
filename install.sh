
set -e


rm -rf lib
rm -rf fmt
rm -rf cppunit
rm -rf include

mkdir lib
mkdir include

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=$(dirname "$SCRIPT")
echo $SCRIPTPATH

function fmt {
  git clone https://github.com/fmtlib/fmt.git fmt

  cd fmt
  git checkout 48b7e3dafb27ece02cd6addc8bd1041c79d59c2c

  mkdir build          # Create a directory to hold the build output.
  cd build
  cmake ..  # Generate native build scripts.

  make

  cp libfmt.a $SCRIPTPATH/lib/.

  cd ..


  # ln -s source destination
  # the destination must be the actual directory to reference the source

  cd $SCRIPTPATH/include
  ln -s ../fmt/include/fmt .

}

function cppunit {

  git clone https://github.com/freedesktop/libreoffice-cppunit.git cppunit

  cd cppunit
  git checkout 64eaa35c2de99581e522608e841defffb4b2923b

  ./autogen.sh
  ./autogen.sh # need to repeat twice for some dark reason : ltmain.sh is not created for the first time : Why ??
  ./configure
  make


  cp ./src/cppunit/.libs/libcppunit.a $SCRIPTPATH/lib/.

  cd $SCRIPTPATH/include
  ln -s ../cppunit/include/cppunit .

}

cd $SCRIPTPATH
fmt
cd $SCRIPTPATH
cppunit


cd $SCRIPTPATH
ls -l lib
ls -l include
