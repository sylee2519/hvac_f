#module load cmake libfabric
#module load Core/24.07 json-glib/1.6.6-hipo3my
#module unload darshan-runtime

module load rocm/6.0.0
#module load cray-pmi/6.1.13
#module load cray-mpich/8.1.28

#source /lustre/orion/proj-shared/stf008/hvac/hsoon_venv/bin/activate 

export HVAC_SERVER_COUNT=1
export HVAC_LOG_LEVEL=800
export RDMAV_FORK_SAFE=1
export VERBS_LOG_LEVEL=4
export BBPATH=/mnt/bb/$USER

export http_proxy=http://proxy.ccs.ornl.gov:3128/
export https_proxy=http://proxy.ccs.ornl.gov:3128/

#export CC=/lustre/orion/proj-shared/stf008/hvac/hsoon/GCC-9.1.0/bin/gcc
#export CXX=/lustre/orion/proj-shared/stf008/hvac/hsoon/GCC-9.1.0/bin/c++

#export CC=/lustre/orion/gen008/proj-shared/GCC-9.1.0/bin/gcc
#export CXX=/lustre/orion/gen008/proj-shared/GCC-9.1.0/bin/g++

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/lustre/orion/gen008/proj-shared/log4c-1.2.4/install/lib/pkgconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/orion/gen008/proj-shared/log4c-1.2.4/install/lib
export PATH=/lustre/orion/gen008/proj-shared/mercury-2.0.1/build/bin:$PATH
export LD_LIBRARY_PATH=/lustre/orion/gen008/proj-shared/rlibrary/mercury2.0.1/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=/lustre/orion/gen008/proj-shared/rlibrary/mercury2.0.1/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/lustre/orion/gen008/proj-shared/rlibrary/mercury2.0.1/include:$CPLUS_INCLUDE_PATH
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/lustre/orion/gen008/proj-shared/rlibrary/mercury2.0.1/lib/pkgconfig


#export PATH=$PATH:/lustre/orion/proj-shared/stf008/hvac/HVAC/build/mercury/install/bin
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/orion/proj-shared/stf008/hvac/HVAC/build/mercury/install/lib
#export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/lustre/orion/proj-shared/stf008/hvac/HVAC/build/mercury/install/lib/pkgconfig
#export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/lustre/orion/proj-shared/stf008/hvac/log4c-1.2.4/install/lib/pkgconfig
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/orion/proj-shared/stf008/hvac/log4c-1.2.4/install/lib
#export C_INCLUDE_PATH=$C_INCLUDE_PATH:/lustre/orion/proj-shared/stf008/hvac/HVAC/build/mercury/install/include
#export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/lustre/orion/proj-shared/stf008/hvac/HVAC/build/mercury/install/include


