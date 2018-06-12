---
layout: index
---

> NEST is a simulator for spiking neural networks. Development is
  coordinated by the [NEST Initiative](http://nest-initiative.org). For
  details about how to use NEST for your own modeling studies, see
  [http://nest-simulator.org](http://nest-simulator.org).

<hr>

This is the developer manual for NEST. For assistance with using NEST, please
use the NEST users' [mailing list](http://www.nest-simulator.org/community/).

In order to keep the community up to
date with NEST development, we use a public source code repository. Any
unpublished, internal work is done in private repositories and made public when
ready.

For ways to add your own code to NEST, please read further.

# Development infrastructure and workflow

NEST development is backed by multiple interlinked systems, most
notably:

* a central [source code repository](https://github.com/nest/nest-simulator)
  hosted on [GitHub](https://github.com/)
* the associated GitHub [issue
  tracker](https://github.com/nest/nest-simulator/issues)
* [continuous integration](continuous_integration) for pull requests provided
  by [TravisCI](https://travis-ci.org/)
* self-hosted [mailing lists](http://www.nest-simulator.org/community/) based
  on [Mailman](http://www.gnu.org/software/mailman/).

The development workflow is based purely on pull requests. This means
that no direct commits to the repository are allowed. All new material must 
go through the code review process.

The details of the development workflow and the code review process can be
found in the following pages:

* [Development workflow for NEST](development_workflow)  
* [Code review guidelines](code_review_guidelines)  

# Extending NEST

The NEST simulator is a scientific tool that is is constantly changing
to meet the needs of novel neuroscientific endeavors. If functionality you need
is not included in NEST already, you can extend NEST. The easiest way to add
neuron or synapse models to NEST is in the form of a plugin in the form of an
extension module:

* [Writing an Extension Module](extension_modules)

A neuron model in NEST is a C++ class that contains the neuron
dynamics and implements the API for setting and retrieving parameters,
updating the dynamics, sending and receiving events, and recording
analog quantities from the model.

Devices are similar to neurons, but used to stimulate the network or
record from the neurons without necessarily having internal dynamics:

* [Developing neuron and device models](neuron_and_device_models)
* [Multimeter support for models](multimeter_support)

Synapses mediate the signal flow between neuron or device models. They
can either be static or implement synaptic plasticity rules such as
[spike time dependent plasticity
(STDP)](http://www.scholarpedia.org/article/Spike-timing_dependent_plasticity):

* [Synapses in NEST: An overview](synapses_overview)
* [Developing synapse models](synapse_models)

## Updating models to 2.16

With the introduction of the new connection infrastructure of the [5g kernel](https://www.frontiersin.org/articles/10.3389/fninf.2018.00002/full), 
rate neuron, synapse and device models need to be slightly adapted from prior 
versions to be compatible with the latest release (2.16). In the following we describe 
all necessary changes:

* [How to update models from NEST 2.14 or prior to 2.16](model_conversion_5g)

## Updating models to 2.6 or later

If you find models written for NEST version 2.4 and prior not
working anymore in newer versions, it is most likely due to recent
updates to the API for neuron and synapse models. We've put together
a conversion guide to make the transition of models easier for you:

* [How to update models from NEST 2.4 or prior to 2.6 or later](model_conversion_3g_4g)

# Dig deeper!

NEST is a complex piece of software with a [long
history](http://dx.doi.org/10.3389/conf.fninf.2013.09.00106). 
Here's a collection of documents describing the NEST simulation kernel in
chronological order to help you get started:

* Diesmann et al. (2002): [An environment for neural systems
  simulation](http://cns-classes.bu.edu/cn510/Papers/diesmann-gewaltig-02.pdf)
  explains the first version of the NEST simulation kernel.
* J.M. Eppler (2006): [A Multithreaded and Distributed System for the
  Simulation of Large Biological Neural
  Networks](http://mindzoo.de/files/Diploma-JME.pdf) explains the general
  architecture of the first hybrid-parallel version of NEST, which is the 2nd
  generation of the simulation kernel.
* Helias et al. (2012): [Supercomputers ready for use as discovery machines for
  neuroscience.](http://dx.doi.org/10.3389/fninf.2012.00026) explains updates
  to the simulation kernel to enable the routinely use of supercomputers.
* Kunkel et al. (2014): [Spiking network simulation code for petascale
  computers](http://dx.doi.org/10.3389/fninf.2014.00078) introduced the 4th
  generation connection infrastructure of NEST, which reduces the memory
  consumption considerably and thus enables the use of the world's largest
  supercomputers.

For even more information, please see the [numerous
publications](http://www.nest-initiative.org/publications/) about the
technology behind NEST.

# Contributions welcome!

We encourage contributions to NEST! New models, new functions, and any general
improvements to NEST are most welcome!

To prevent [bit rot](https://en.wikipedia.org/wiki/Software_rot) all code in
NEST must adhere to some minimal coding and naming conventions:

* [Coding Guidelines for C++](coding_guidelines_c++)
* [Coding Guidelines for SLI](coding_guidelines_sli)
* [Naming convention for neuron models](neuron_model_naming)
* [Naming convention for synapse models](synapse_model_naming)
* [Naming convention for variables and parameters](variables_parameters_naming)

Please ensure that you follow these guidelines while adding new features to
NEST. The [workflow manual](development_workflow) documents the development
process in detail. 

## Contributor License Agreement

In order to make sure that the NEST Initiative can manage the NEST
code base in the long term, you need to send us a completed and signed
[NEST Contributor Agreement](NEST_Contributor_Agreement.pdf) to
transfer your copyright to the NEST Initiative before we can merge
your pull request.

You might also want to consider [becoming a
member](http://www.nest-initiative.org/membership/) in the NEST
Initiative.

# Further reading (aka needs-a-better-home)

* [Creating and handling Tokens and Datums in SLI](tokens_and_datums)
* [Xcode workflow](xcode_workflow)
* [Eclipse workflow](eclipse_workflow)
