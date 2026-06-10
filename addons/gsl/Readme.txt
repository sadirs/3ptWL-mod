gsl INSTALLATION to use with wlcf code

1. If necessary, define the following environment variables:

For a standard Linux and Mac OS X:
export CC=gcc
export CXX=g++
export F77=gfortran
export FC=gfortran
export F90=gfortran

Intel compilers:
export CC=icc
export CXX=icc
export F77=ifort
export FC=ifort
export F90=ifort


2. Go to directory: 

cd $HOME/NagBody_pkg/NagBody_sources/additional_libs/gsl

3. Unpack file:

tar xvf gsl-2.7.1.tar.gz

4. Change to directory:

cd gsl-2.7.1

5. Configure, make and install:

./configure --prefix=$HOME/local/gsl 2>&1 | tee configure_gsl.log
make 2>&1 | tee make_gsl.log
make check 2>&1 | tee check_gsl.log
make install 2>&1 | tee install_gsl.log

6. Clean directory:

make distclean

7. Environment. We assume you are using bash shell. Edit .bash_profile or .profile files that are in $HOME

export PATH=${HOME}/local/gsl/bin:${PATH}
export MANPATH=${HOME}/local/gsl/share/man:${MANPATH}
export PKG_CONFIG_PATH=${HOME}/local/gsl/lib/pkgconfig:${PKG_CONFIG_PATH}
export LD_LIBRARY_PATH=${HOME}/local/gsl/lib:${LD_LIBRARY_PATH}

Restart shell.

That's it! 


