microcircuit simulation parameters
----------------------------------

.. code-block:: python
   :linenos:

   import os
   sim_dict = {
       # Simulation time (in ms).
       't_sim': 1000.0,
       # Resolution of the simulation (in ms).
       'sim_resolution': 0.1,
       # Path to save the output data.
       'data_path': os.path.join(os.getcwd(), 'data/'),
       # Masterseed for NEST and NumPy.
       'master_seed': 55,
       # Number of threads per MPI process.
       'local_num_threads': 1,
       # Recording interval of the membrane potential (in ms).
       'rec_V_int': 1.0,
       # If True, data will be overwritten,
       # If False, a NESTError is raised if the files already exist.
       'overwrite_files': True,
       # Print the time progress, this should only be used when the simulation
       # is run on a local machine.
       'print_time': True
       }

