NEST-pong
=========
This program simultaneously trains two networks of spiking neurons to play
the classic game of Pong.

Requirements
------------
- NEST 3.3
- NumPy
- Matplotlib

Instructions
------------
To start training between two networks with R-STDP plasticity, run
the ``generate_gif.py`` script. By default, one of the networks will
be stimulated with Gaussian white noise, showing that this is necessary
for learning under this paradigm. In addition to R-STDP, a learning rule
based on the ``stdp_dopamine_synapse`` and temporal difference learning
is implemented, see ``networks.py`` for details.

The learning progress and resulting game can be visualized with the
``generate_gif.py`` script.