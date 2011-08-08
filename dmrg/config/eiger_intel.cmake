
set(CMAKE_C_COMPILER icc)
set(CMAKE_CXX_COMPILER icpc)

set(MPI_FOUND TRUE)
set(MPI_COMPILE_FLAGS "")
set(MPI_INCLUDE_PATH "/apps/eiger/Intel-MPI-4.0/intel64/include ")
set(MPI_LINK_FLAGS "-L/apps/eiger/Intel-MPI-4.0/intel64/lib -Xlinker --enable-new-dtags -Xlinker -rpath -Xlinker /apps/eiger/Intel-MPI-4.0/intel64/lib -Xlinker -rpath -Xlinker /opt/intel/mpi-rt/4.0.0")
set(MPI_LIBRARY "-lmpigc4 -lmpi -lmpigf -lmpigi")
set(MPI_EXTRA_LIBRARY "-ldl -lpthread -lrt")
set(MPI_LIBRARIES "-ldl -lmpigc4 -lmpi -lmpigf -lmpigi -lpthread -lrt")
set(MPIEXEC "/apps/eiger/Intel-MPI-4.0/intel64/bin/mpirun")
set(MPIEXEC_NUMPROC_FLAG -np)
set(MPIEXEC_PREFLAGS )
set(MPIEXEC_POSTFLAGS )

set(DEFAULT_BLAS_LAPACK manual)
set(BLAS_LAPACK_MANUAL_LIBS -lmkl_scalapack_ilp64 -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -lmkl_blacs_intelmpi_ilp64 -lpthread)
#set(BLAS_LAPACK_MANUAL_LIBS -Wl,--start-group -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -Wl,--end-group -lpthread)
set(BLAS_LAPACK_MANUAL_LIBS_DIR /apps/eiger/Intel-Composer-XE-2011/mkl/lib/intel64)
set(BLAS_LAPACK_MANUAL_INCLUDES /apps/eiger/Intel-Composer-XE-2011/mkl/include)
