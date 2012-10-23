#!/bin/bash

# This script runs the CUBA simulations once with two threads,
# once with two MPI processes, dumps membrane potentials at
# the end and compares the resulting files.
#
# If all goes well, output should look like (note the four OK lines)

#################################################################
#/virtual_processes 2 def /off_grid false def (cuba_458.sli) run
#-----------------------------------------------------------------
#/virtual_processes 2 def /off_grid false def (cuba_458.sli) run
#-----------------------------------------------------------------
#Plain Offgrid OK
#################################################################
#/virtual_processes 2 def /off_grid true def (cuba_458.sli) run
#-----------------------------------------------------------------
#/virtual_processes 2 def /off_grid true def (cuba_458.sli) run
#-----------------------------------------------------------------
#Plain Ongrid OK
#################################################################
#/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run
#-----------------------------------------------------------------
#/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run
#-----------------------------------------------------------------
#PS Offgrid OK
#################################################################
#/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run
#-----------------------------------------------------------------
#/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run
#-----------------------------------------------------------------
#PS Ongrid OK

# Hans Ekkehard Plesser, 2010-10-18


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

echo "#################################################################"
conf="/virtual_processes 2 def /off_grid false def (cuba_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
conf="/virtual_processes 2 def /off_grid false def (cuba_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
sort iaf_psc_exp_false_2_1_0.dat > ts.dat
sort iaf_psc_exp_false_2_2_0.dat iaf_psc_exp_false_2_2_1.dat > ms.dat
diff ts.dat ms.dat && echo "Plain Offgrid OK" || echo "Plain Offgrid FAILED"
rm ts.dat ms.dat

echo "#################################################################"
conf="/virtual_processes 2 def /off_grid true def (cuba_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
conf="/virtual_processes 2 def /off_grid true def (cuba_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
sort iaf_psc_exp_true_2_1_0.dat > ts.dat
sort iaf_psc_exp_true_2_2_0.dat iaf_psc_exp_true_2_2_1.dat > ms.dat
diff ts.dat ms.dat && echo "Plain Ongrid OK" || echo "Plain Ongrid FAILED"
rm ts.dat ms.dat


echo "#################################################################"
conf="/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
conf="/virtual_processes 2 def /off_grid false def (cuba_ps_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
sort iaf_psc_exp_ps_false_2_1_0.dat > ts.dat
sort iaf_psc_exp_ps_false_2_2_0.dat iaf_psc_exp_ps_false_2_2_1.dat > ms.dat
diff ts.dat ms.dat && echo "PS Offgrid OK" || echo "PS Offgrid FAILED"
rm ts.dat ms.dat

echo "#################################################################"
conf="/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 1 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
conf="/virtual_processes 2 def /off_grid true def (cuba_ps_458.sli) run"
echo $conf
echo $conf > conf.sli
mpirun -np 2 $NEST conf.sli run_benchmark_458_dumpVm.sli > /dev/null 2>&1

echo "-----------------------------------------------------------------"
sort iaf_psc_exp_ps_true_2_1_0.dat > ts.dat
sort iaf_psc_exp_ps_true_2_2_0.dat iaf_psc_exp_ps_true_2_2_1.dat > ms.dat
diff ts.dat ms.dat && echo "PS Ongrid OK" || echo "PS Ongrid FAILED"
rm ts.dat ms.dat

rm conf.sli
rm iaf_psc_exp*_2_*.dat
