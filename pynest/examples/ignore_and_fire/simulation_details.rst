Simulation details
==================

By default, this implementation is based on the LIF neuron,
:doc:`iaf_psc_alpha </models/iaf_psc_alpha>` and the
:doc:`stdp_pl_synapse_hom  </models/stdp_pl_synapse_hom>`
synapse models provided in NEST. With
``pars['neuron_model']='ignore_and_fire'``, the model is configured into
a truly scalable mode where the ``integrate-and-fire`` dynamics are replaced
by an ``ignore_and_fire`` dynamics, while the plasticity dynamics
remains intact.

The network is connected according to the
``fixed_indegree`` :ref:`connection rule <connection_management>` in NEST.

The neuron dynamics is propagated in time using exact integration
based on Rotter and Diesmann [1]_ with a simulation step
size :math:`\Delta{}t`. The synapse dynamics is updated in an
event-based manner as described by Morrison et
al. [2]_.


 .. The model implementation runs with `NEST 3.6 <https://github.com/nest/nest-simulator.git>`__ and `NESTML 6.0.0 <https://github.com/nest/nestml>`__.

Simulation parameters
---------------------

+-----------------------+---------------------------------+-----------------------------+
| Name                  | Value                           | Description                 |
+=======================+=================================+=============================+
| :math:`T`             | :math:`1000\,\text{ms}`         | total simulation time       |
+-----------------------+---------------------------------+-----------------------------+
| :math:`\Delta{}t`     | :math:`2^{-3}=0.125\,\text{ms}` | duration pof simulation step|
+-----------------------+---------------------------------+-----------------------------+
| ``tics_per_step``     | :math:`2^7=128`                 | number of tics per          |
|                       |                                 | simulation step             |
|                       |                                 | :math:`\Delta{t}`           |
|                       |                                 | (time resolution)           |
+-----------------------+---------------------------------+-----------------------------+

References
----------

.. [1] Rotter & Diesmann (1999). Exact digital simulation of time-invariant
       linear systems with applications to neuronal modeling. Biological
       Cybernetics 81(5-6):381-402.
       doi:10.1007/s004220050570 https://doi.org/10.1007/s004220050570

.. [2] Morrison et al. (2007). Spike-timing dependent plasticity in
       balanced random networks. Neural Computation
       19:1437-1467 10.1162/neco.2007.19.6.1437
