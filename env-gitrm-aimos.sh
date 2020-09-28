module use /gpfs/u/software/dcs-spack-install/v0133gccSpectrum/lmod/linux-rhel7-ppc64le/gcc/7.4.0-1/
module load spectrum-mpi/10.3-doq6u5y
module load gcc/7.4.0/1
module load cmake/3.15.4-mnqjvz6
#module load netcdf-cxx4/4.3.1-7lwefeg
module load netcdf-cxx4/4.3.1-4es6a7y

build=build-dcs-gcc74
export pp=/gpfs/u/barn/MPFS/shared/gopan/gitrmwork/build-dcs-gcc74-pumipic-gitrm/install
cuda=/usr/local/cuda-10.1
export PATH=$cuda/bin:$PATH
export LD_LIBRARY_PATH=$cuda/lib64:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$pp:$CMAKE_PREFIX_PATH
export root=/gpfs/u/barn/MPFS/shared/gopan/libraries
export OMPI_CXX=$root/kokkos/bin/nvcc_wrapper
export OIMPI_CC=gcc
