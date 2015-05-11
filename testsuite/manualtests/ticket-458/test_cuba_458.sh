#!/bin/bash

# Some stuff for convenient testing on Stallo
#PBS -lnodes=1:ppn=2
#PBS -lwalltime=00:10:00
#PBS -lpmem=1000MB
#PBS -j oe
#PBS -m abe

# PBS needs to change, normal script just does cd .
cd ${PBS_O_WORKDIR:-.}               

# replace with proper path to NEST exectutable
NEST=/home/plesser/NEST/trunk/ins/bin/nest

conf="/virtual_processes 1 def /off_grid false def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid false def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid false def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

echo "********************************************************************"
echo "********************************************************************"
echo "********************************************************************"

conf="/virtual_processes 1 def /off_grid true def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid true def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid true def (cuba_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

echo "===================================================================="
echo "===================================================================="
echo "===================================================================="

conf="/virtual_processes 1 def /off_grid false def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

echo "********************************************************************"
echo "********************************************************************"
echo "********************************************************************"

conf="/virtual_processes 1 def /off_grid true def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

conf="/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run"
echo "#################################################################"
echo $conf
echo "-----------------------------------------------------------------"
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458.sli

echo "#################################################################"

rm conf.sli
