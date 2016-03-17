# MUSIC_CONT_OUT_PROXY EXAMPLE

## Requirements
+ MUSIC 1.1.15 or higher
+ NEST 2.10.0 or higher compiled with MPI and MUSIC
+ Numpy

## Instructions
This example runs 2 NEST instances and one receiver instance. 
Neurons on the NEST instances are observed by the music_cont_out_proxy 
and their values are forwarded through MUSIC to the receiver.

```sh
$  mpiexec -np 3 music test.music
```

