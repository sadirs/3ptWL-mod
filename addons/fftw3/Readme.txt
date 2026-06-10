
fftw-3 INSTALLATION to use with wlcf code

Dependencies: OpenMPI if needed

The other way to install the package is to follow the steps:

1. If necessary, define the following environment variables:

For a standard Linux and Mac OS X:
export CC=gcc
export CXX=g++
export F77=gfortran
export FC=gfortran
export F90=gfortran

Portland group:
export CC=pgcc
export CXX=pgcc ?
export F77=pgf90
export FC=pgf90
export F90=pgf90

Intel compilers
export CC=icc
export CXX=icc
export F77=ifort
export FC=ifort
export F90=ifort

2. Unpack file:

tar xvf fftw-3.3.10.tar.gz

3. Change to directory:

cd fftw-3.3.10

4. Configure, make and install:

First double without prefix:
./configure --prefix=$HOME/local/fftw3 --enable-mpi --enable-threads 2>&1 | tee configure_gcc_gfortran_openmpi.log
make 2>&1 | tee make_gcc_gfortran_openmpi.log
make check
make install 2>&1 | tee install_gcc_gfortran_openmpi.log

Not WORKING: --enable-type-prefix ::::

Then in simple precision with prefix 's':
make distclean
./configure --prefix=$HOME/local/fftw3 --enable-float --enable-type-prefix --enable-threads --enable-mpi 2>&1 | tee configure_gcc_gfortran_openmpi.log
make 2>&1 | tee make_gcc_gfortran_openmpi.log
make check
make install 2>&1 | tee install_gcc_gfortran_openmpi.log

We repeat for double precision
make distclean
./configure --prefix=$HOME/local/fftw3 --enable-type-prefix --enable-threads --enable-mpi 2>&1 | tee configure_gcc_gfortran_openmpi.log
make 2>&1 | tee make_gcc_gfortran_openmpi.log
make check
make install 2>&1 | tee install_gcc_gfortran_openmpi.log


5. Clean directory:

make distclean

6. Environment. We assume you are using bash shell. Edit .bash_profile or .profile files that are in $HOME

export PATH=${HOME}/local/fftw3/bin:${PATH}
export MANPATH=${HOME}/local/fftw3/share/man:${MANPATH}
export PKG_CONFIG_PATH=${HOME}/local/fftw3/lib/pkgconfig:${PKG_CONFIG_PATH}
export DYLD_LIBRARY_PATH=${HOME}/local/fftw3/lib:${DYLD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${HOME}/local/fftw3/lib:${LD_LIBRARY_PATH}

Restart shell.

That's it! 

