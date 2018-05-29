Basics of NEST
#################

Here we will give an overview of the basic syntax and constructs of NEST.

.. _FAnchor:

Basic Functions
----------------
Fundamentally, we can build a basic network with the following functions::

    #Create the models we want to simulate
    #This function returns a list of handles called the GIDs, which we assign to a variable
    node_id = nest.Create("model_name")

    # Modify properties of interest of given node
    nest.SetStatus(node_id, {"key" : value})

    # Tell NEST how nodes are connected to each other
    nest.Connect(pre_synaptic_node, post_synaptic_node)

    # Simulate network providing a specific timeframe.
    nest.Simulate(time_in_ms)

.. seealso:: :doc:`PyNEST Tutorial Part 1: Neurons and simple neural networks <tutorials/part-1-neurons-and-simple-neural-networks>` or the :doc:`One Neuron Example </examples/one_neuron>` to try it out yourself.

.. note:: Properties of objects in NEST are generally in the form of dictionaries {key : value}. Function calls typically return dictionary or lists of dictionaries.



Basic terminology
-----------------
Models
*********
 Models consist of :ref:`NAnchor` and :ref:`SAnchor`. In Python, you can type :code:`nest.Models()` to get the complete list of models in NEST.
.. note::  The function takes two optional parameters; you can specify  node or synapse :code:`nest.Models("nodes")` or :code:`nest.Models("synapses")` and do further filtering of results with the option :code:`nest.Models("nodes", "some_filter")`.

To see the default properties and values of any model::

    nest.GetDefaults("model_name")

This command above will provide a dictionary of properties for the model, but not all properties are relevant for the dynamics of the neuron.

To view the parameters for a model that are useful to modify type::

    nest.help("model_name")

This will output the documentation for that model including the parameters of interest.

.. note::  NEST is type sensitive, so when modifying a parameter check the type indicated in the parameter list. For example, if a property is of type double then the decimal point is required!

.. _NAnchor:

Nodes
~~~~~
Nodes consist of neuron models, devices and subnets. Each node can connect to another node.

    * **Neuron models** serve as the heart of NEST. There are many neuron models from simple integrate-and-fire neurons to Hodgkin-Huxley neuron models.

    * **Devices** represent instruments to either measure or stimulate the network.

      Examples include measuring devices like a multimeter or spike detector or generators of neural activity, like the poisson generator.



    * **Subnets** are network nodes. The default subnetwork is called the :code:`root node`. Subnets can be arranged and connected to build hierarchal networks.

    .. seealso:: :doc:`PyNEST Tutorial Part 4: Topologically structured networks <tutorials/part-4-topologically-structured-networks>` or the :download:`Topology Manual <guides/Topology_UserManual.pdf>`  for more details.

Once you have :ref:`created your node <FAnchor>`, you can view its current parameters and corresponding values::

    nest.GetStatus(node_id)


.. _SAnchor:

Synapses
~~~~~~~~

The synapse model and its properties can be passed as an optional parameter, :code:`syn_spec`, either as a string or dictionary
in the :code:`nest.Connect()` function.

You can also modify multiple dictionary properties and pass them into :code:`nest.Connect()`::

    syn_dict_ex =[{"model": "tsodyks_synapse"}, {"weight": 1.2}]
    nest.Connect(pre_syn_node, post_syn_node, syn_spec=syn_dict_ex)

.. seealso:: :doc:`PyNEST Tutorial Part 3: Connecting networks with synapses <tutorials/part-3-connecting-networks-with-synapses>` for details on using synapses in your network.
.. note:: Available keys in the synapse dictionary include "model", "weight", "delay", "receptor_type" and parameters specific to the chosen synapse model. You can view and alter the default parameters with :code:`nest.GetDefaults("synapse_model")` and :code:`nest.SetDefaults("synapse_model")`, respectively. 



