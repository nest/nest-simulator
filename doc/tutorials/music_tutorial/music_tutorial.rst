MUSIC Tutorial
===============


.. sidebar:: See Also




In this tutorial we will show you how to use the MUSIC library together with NEST.
We will cover how to use the library from PyNEST and from the SLI language interface.
In addition, we'll introduce the use of MUSIC in a C++ application and how to
connect such an application to a NEST simulation.

Our aim is to show practical examples of how to use MUSIC, and highlight common 
pitfalls that can trip the unwary. Also, we assume only a minimal knowledge of
Python, C++ and (especially) SLI, so the examples will favour clarity and
simplicity over elegance and idiomatic constructs.

A Note On Documentation
-------------------------

The primary MUSIC documentation is the MUSIC manual that you can find under the 
$\texttt{/doc}$ subdirectory in the MUSIC source tree. Another helpful source
is "Run-Time Interoperability Between Neuronal Network Simulators Based on the
MUSIC Framework"\cite{djurfeldt}. The MUSIC source has a couple of examples
under $\texttt{/examples}$, and there are a few more in the NEST source tree
under$\texttt{examples/nest/music}$.


.. code-block:: python

  import nest
  nest.Connect?
  help(nest.Connect)

A Quick Introduction to NEST and MUSIC
---------------------------------------

While the focus here is on MUSIC, we need to know a few things about how NEST
works in order to understand how MUSIC interacts with it.

The Basics of NEST
~~~~~~~~~~~~~~~~~~~~

A NEST network consists of three types of elements: neurons, devices, and
connections between them. Neurons are the basic building blocks, and in NEST
they are generally spiking point neuron models. Devices are supporting units
that, for instance, generate inputs to neurons or record data from them.
The Poisson spike generator, the spike detector recording device and the MUSIC
input and output proxies are all devices. Neurons and devices are collectively
called nodes, and are connected using connections. Connections are unidirectional
and carry events between nodes. Each neuron can get multiple input connections
from any number of other neurons. Neuron connections typically carry spike events,
but other kinds of events, such as voltages and currents, are also available for
recording devices. Synapses are not independent nodes, but are part of the
connection. Synapse models will typically modify the weight or timing of the
spike sent on to the neuron. All connections have a synapse, by default the
$\texttt{static\_synapse}$.


.. figure:: (_static/_img/filename.jpg)
   :width: 200px
   :align: center
   :height: 100px
   :alt: alternate text
   :figclass: align-center

    Figure caption here. Figures are optional but recommended. Images should be stored in
    (nest-simulator/doc/_static/img/)

List of commands and terms
--------------------------

Here you provide a list of the terms learned in the tutorial with a brief definition 

Provide a link here to the next section of your tutorial if there is one.
:doc:`next/part/of/tutorial`

