********************************************
Welcome to the NEST simulator documentation!
********************************************

:orange:`Introducing NEST 3.0`
------------------------------

NEST 3.0 provides a more intuitive experience with simplified yet versatile handling and manipulation of nodes and connections.


- Visit our :doc:`What's new? <guides/nest2_to_nest3/nest2_to_nest3_overview>` guide to get an overview of NEST 3.0 and the new functionality.


- Read the :doc:`Detailed transition guide <guides/nest2_to_nest3/nest2_to_nest3_detailed_transition_guide>` for an in-depth comparison between old and new syntax.

----

If you use NEST for your project, don't forget to :doc:`cite NEST <citing-nest>`!

+------------------------------------+---------------------------------------+
|                                    |                                       |
|    :doc:`Download <download>`      |  :doc:`Install <installation/index>`  |
|                                    |                                       |
+------------------------------------+---------------------------------------+

NEST is a simulator for **spiking neural network models**, ideal for networks of any size, for example:

1.  Models of information processing e.g., in the visual or auditory cortex of
    mammals,

2.  Models of network activity dynamics, e.g., laminar cortical networks or
    balanced random networks,

3.  Models of learning and plasticity.

**New to NEST?**
    Start here at our :doc:`Getting Started <getting_started>` page


**Have an idea of the type of model you need?**
    Click on one of the images to access our :doc:`model directory <models/index>`:

.. raw:: html

 <embed>

 <a href="models/neurons.html">
    <img src="_static/img/neuron.png" alt="Neuron Models" style="width:150px;height:150px;border:0;">
  </a>
  <a href="models/synapses.html">
    <img src="_static/img/synapse1.png" alt="Synapse Models" style="width:150px;height:150px;border:0;">
  </a>
  <a href="models/devices.html">
    <img src="_static/img/oscilloscope.png" alt="Devices" style="width:150px;height:150px;border:0;">
  </a>
  </embed>

**Create complex networks using the Microcircuit Model:**

.. raw:: html

  <embed>
  <a href="microcircuit/index.html">
    <img src="_images/microcircuit.png" alt="Microcircuit" style="width:150px;height:150px;border:0;">
  </a>


  </embed>


**Need a different model?**
    Check out how you can :doc:`create you own model <models/create_model>` here.

**Have a question or issue with NEST?**
    See our :doc:`Getting Help <getting_help>` page.

Where to find what
##################

* :doc:`Tutorials <tutorials/index>` show you step by step instructions using NEST. If you haven't used NEST before, the PyNEST tutorial is a good place to start.

* :doc:`Example Networks <examples/index>`  demonstrate the use of dozens of the neural network models implemented in NEST.

* :doc:`Topical Guides <guides/index>` provide deeper insight into several topics and concepts from :doc:`Parallel Computing <guides/parallel_computing>` to handling :doc:`Gap Junction Simulations <guides/simulations_with_gap_junctions>` and :doc:`setting up a spatially-structured network <guides/spatial/guide_spatially_structured_networks>`.

* :doc:`Reference Material <ref_material/index>` provides a quick look up of definitions, functions and terms.

Interested in contributing?
###########################

* Have you used NEST in an article or presentation? :doc:`Let us know <community>` and we will add it to our list of `publications <https://www.nest-simulator.org/publications/>`_.
  Find out how to :doc:`cite NEST <citing-nest>` in your work.

* If you have any comments or suggestions, please share them on our :doc:`Mailing List <community>`.

* Want to contribute code? Visit out our `Developer Space <https://nest.github.io/nest-simulator/>`_ to get started!

* Interested in creating or editing documentation? Check out our :doc:`Documentation workflows <documentation_workflow/index>`.

* For more info about our larger community and the history of NEST check out the `NEST Initiative <https://www.nest-initiative.org>`_ website

Related projects
################

Many extensions and open-source tools related to the NEST Simulator are available. In particular, the following packages may be of interest:

- `NEST Desktop <https://nest-desktop.readthedocs.io/en/latest/>`_ - a web-based GUI application for NEST Simulator
- `NESTML <https://nestml.readthedocs.io/en/latest/>`_ - a domain specific language to describe neuron models in NEST
- `PyNN <http://neuralensemble.org/PyNN/>`_ - a simulator-independent language for building neuronal network models
- `Elephant <http://neuralensemble.org/elephant/>`_ - a package for the analysis of neurophysiological data, using Neo data structures

You can find more projects by the community, for example by searching GitHub for the topics `"nest-simulator" <https://github.com/topics/nest-simulator>`_ or `"nest-module" <https://github.com/topics/nest-module/>`_.

License
#######

NEST is available under the :doc:`GNU General Public License 2 or later <license>`. This means that you can

-  use NEST for your research,
-  modify and improve NEST according to your needs,
-  distribute NEST to others under the same license.

Acknowledgements
################

This project has received funding from the European Union’s Horizon 2020 Framework Programme for Research and
Innovation under Specific Grant Agreement No. 945539 (Human Brain Project SGA3), No. 720270 (Human Brain Project
SGA1), No. 785907 (Human Brain Project SGA2), No. 754304 (DEEP-EST) and No. 800858 (ICEI).

The authors gratefully acknowledge the received support and funding from the European Union 6th and 7th Framework
Program under grant agreement no. 15879 (FACETS), the European Union 7th Framework Program under grant agreement no.
269921 (BrainScaleS), the European Union 7th Framework Programme ([FP7/2007-2013]) under grant agreement no. 604102
(Human Brain Project, HBP), the computing time granted by the JARA-HPC Vergabegremium and provided on the JARA-HPC
Partition part of the supercomputers JUQUEEN and JURECA at Forschungszentrum Jülich (VSR computation time grant
JINB33), the Jülich Aachen Research Alliance (JARA), the Next-Generation Supercomputer Project of MEXT, Japan, the
eScience program of the Research Council of Norway under grant 178892/V30 (eNeuro), the Helmholtz Association through
the Helmholtz Portfolio Theme "Supercomputing and Modeling for the Human Brain", the Excellence Initiative of the
German federal and state governments, the Priority Program (SPP 2041 "Computational Connectomics") of the Deutsche
Forschungsgemeinschaft [S.J. van Albada: AL 2041/1-1], the Helmholtz young investigator's group VH-NG-1028 "Theory of
multi-scale neuronal networks", and compute time provided by UNINETT Sigma2 - the National Infrastructure for High
Performance Computing and Data Storage in Norway and its predecessors.

.. image:: _static/img/HBP.png
  :width: 55 %
  :target: https://www.humanbrainproject.eu/
.. image:: _static/img/EBRAINS.svg
  :width: 25 %
  :target: https://ebrains.eu/
