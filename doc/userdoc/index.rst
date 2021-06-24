********************************************
Welcome to the NEST simulator documentation!
********************************************

:orange:`Introducing NEST 3.0`
------------------------------

NEST 3.0 provides a more intuitive experience with simplified yet versatile handling and manipulation of nodes and connections.

You can find all the information in our section :doc:`guides/nest2_to_nest3/index`.

.. note::

  Note that some of your simulation scripts may need to be updated to run in NEST 3.0!
  See our :doc:`reference guide <guides/nest2_to_nest3/refguide_nest2_nest3>` comparing NEST 2.x versus NEST 3.0 syntax.



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


**Know which model you need?**
    NEST comes packaged with a large collection of neuron and synaptic plasticity models.
    You can find a list of all available models in our :doc:`model directory <models/index>`,
    or select a model category by clicking one of the images:

.. raw:: html

 <embed>

 <a href="models/index_neuron.html">
    <img src="_static/img/neuron.png" alt="Neuron Models" style="width:150px;height:150px;border:0;">
  </a>
  <a href="models/index_synapse.html">
    <img src="_static/img/synapse1.png" alt="Synapse Models" style="width:150px;height:150px;border:0;">
  </a>
  <a href="models/index_device.html">
    <img src="_static/img/oscilloscope.png" alt="Devices" style="width:150px;height:150px;border:0;">
  </a>
  </embed>

**Create complex networks using the Microcircuit Model:**

.. raw:: html

  <embed>
  <a href="examples/cortical_microcircuit_index.html">
    <img src="_images/microcircuit.png" alt="Microcircuit" style="width:150px;height:150px;border:0;">
  </a>
  </embed>

**Need a different model?**
    To customize or combine features of neuron and synapse models, we recommend
    using the `NESTML modeling language <https://nestml.readthedocs.io/>`_.

**Have a question or issue with NEST?**
    See our :doc:`Getting Help <getting_help>` page.

Where to find what
------------------

* :doc:`Tutorials <tutorials/index>` show you step by step instructions using NEST. If you haven't used NEST before, the PyNEST tutorial is a good place to start.

* :doc:`Example Networks <examples/index>`  demonstrate the use of dozens of the neural network models implemented in NEST.

* :doc:`Topical Guides <guides/index>` provide deeper insight into several topics and concepts from :doc:`Parallel Computing <guides/parallel_computing>` to handling :doc:`Gap Junction Simulations <guides/simulations_with_gap_junctions>` and :doc:`setting up a spatially-structured network <guides/spatial/guide_spatially_structured_networks>`.

* :doc:`Reference Material <ref_material/index>` provides a quick look up of definitions, functions and terms.

Interested in contributing?
---------------------------

* Have you used NEST in an article or presentation? :doc:`Let us know <community>` and we will add it to our list of `publications <https://www.nest-simulator.org/publications/>`_.
  Find out how to :doc:`cite NEST <citing-nest>` in your work.

* If you have any comments or suggestions, please share them on our :doc:`Mailing List <community>`.

* Want to contribute code? Visit out our :doc:`Contributing <contribute/index>` pages to get started!

* Interested in creating or editing documentation? Check out our :doc:`Documentation workflows <documentation_workflow/index>`.

* For more info about our larger community and the history of NEST check out the `NEST Initiative <https://www.nest-initiative.org>`_ website

Related projects
----------------

Many extensions and open-source tools related to the NEST Simulator are available. In particular, the following packages may be of interest:

- `NEST Desktop <https://nest-desktop.readthedocs.io/en/latest/>`_ - a web-based GUI application for NEST Simulator
- `NESTML <https://nestml.readthedocs.io/en/latest/>`_ - a domain specific language to describe neuron models in NEST
- `PyNN <http://neuralensemble.org/PyNN/>`_ - a simulator-independent language for building neuronal network models
- `Elephant <http://neuralensemble.org/elephant/>`_ - a package for the analysis of neurophysiological data, using Neo data structures

You can find more projects by the community, for example by searching GitHub for the topics `"nest-simulator" <https://github.com/topics/nest-simulator>`_ or `"nest-module" <https://github.com/topics/nest-module/>`_.

License
-------

NEST is available under the :doc:`GNU General Public License 2 or later <license>`. This means that you can

-  use NEST for your research,
-  modify and improve NEST according to your needs,
-  distribute NEST to others under the same license.

.. include::  ACKNOWLEDGMENTS.md

.. image:: static/img/HBP.png
  :width: 55 %
  :target: https://www.humanbrainproject.eu/
.. image:: static/img/EBRAINS.svg
  :width: 25 %
  :target: https://ebrains.eu/
