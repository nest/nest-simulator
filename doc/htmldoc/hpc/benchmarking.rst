Benchmarking NEST
=================

Computational efficiency is essential to simulate complex neuronal networks and study long-term effects such as learning.
The scaling performance of neuronal network simulators on high-performance computing systems can be assessed with benchmark simulations.
However, maintaining comparability of benchmark results across different systems, software environments, network models, and researchers from potentially different labs poses a challenge.  
The software framework `beNNch <https://github.com/INM-6/beNNch>`_ tackles this challenge by implementing a unified, modular workflow for configuring, executing, and analyzing such benchmarks.  
beNNch builds around the `JUBE Benchmarking Environment <https://www.fz-juelich.de/ias/jsc/EN/Expertise/Support/Software/JUBE/_node.html>`_, installs simulation software, provides an interface to benchmark models, automates data and metadata annotation, and accounts for storage and presentation of results.

For more details on the conceptual ideas behind beNNch, refer to `Albers et al. (2022) <https://www.frontiersin.org/articles/10.3389/fninf.2022.837549/full>`_ .

![](multi-area-model_5faa0e9c.png)
***Example ``beNNch`` output (Figure 5C of Albers et al., 2021):
Strong-scaling performance of the [multi-area model](https://github.com/INM-6/multi-area-model) simulated with the neuronal network simulator [NEST](https://www.nest-simulator.org) on JURECA-DC.**
The left graph shows the absolute wall-clock time measured with Python-level timers for both network construction and state propagation.
Error bars indicate variability across three simulation repeats with different random seeds.
The top right graph displays the real-time factor defined as wall-clock time normalized by the model time.
Built-in timers resolve four different phases of the state propagation: update, collocation, communication, and delivery.
Pink error bars show the same variability of state propagation as the left graph.
The lower right graph shows the relative contribution of these phases to the state-propagation time.*

See also the accompanying [GitHub Page](https://inm-6.github.io/beNNch) for further beNNch results in flip-book format.

We recommend taking a look at the instructions of `beNNch <https://github.com/INM-6/beNNch>`_ for details on setting up benchmarking in your project.

Helpful commands
----------------

Here are some commands that can help you understand your system better.
You can run them locally on any Linux machine as well as on HPC systems.

* ``lstopo``
     lstopo and lstopo-no-graphics are capable of displaying a topological map of the system

* ``htop``
      It is similar to top, but allows you to scroll vertically and horizontally, so you can see all the processes
      running on the system, along with their full command lines.


