---
layout: index
---

> NEST is a simulator for spiking neural networks. Development is
  coordinated by the [NEST Initiative](http://nest-initiative.org). For
  details about how to use NEST for your own modeling studies, see
  [http://nest-simulator.org](http://nest-simulator.org).

<hr>

# This is the developer manual for NEST.

We use a private repository for internal and unpublished work, while
all general NEST development takes place in the public repository.

This means that you can follow NEST's development as it happens.

For ways to contribute your own code, see below.

## Development infrastructure and workflow

NEST development is backed by multiple interlinked systems, most
notably,

* a central [source code repository](https://github.com/nest/nest-simulator) hosted on [GitHub](https://github.com/)
* the associated GitHub [issue tracker](https://github.com/nest/nest-simulator/issues)
* [continuous integration](continuous_integration) for pull requests provided by [TravisCI](https://travis-ci.org/)
* self-hosted [mailing lists](http://www.nest-simulator.org/community/) based on [Mailman](http://www.gnu.org/software/mailman/).

The development workflow is based purely on pull requests. This means
that no direct commits to the repository are allowed, but all has to
go through the code review process unconditionally.

The details of the development workflow and the code review are layed
out in the following pages:

* [Development workflow for NEST](development_workflow)  
* [Code review guidelines](code_review_guidelines)  

## Extending NEST

The NEST simulator is a scientific tool and as such it is never ready
and constantly changing to meet the needs of novel neuroscientific
endeavors.

When adding your own neuron or synapse models, the easiest way to do
so without messing with NEST's source code, is to write a plugin in
the form of an extension module:

* [Writing an Extension Module](extension_modules)

A neuron model in NEST is a C++ class that contains the neuron
dynamics and implements the API for setting and retrieving parameters,
updating the dynamics, sending and receiving events, and recording
analog quantities from the model.

Devices are similar to neurons, but used to stimulate the network or
record from the neurons without necessarily having internal dynamics.

* [Developing neuron and device models](neuron_and_device_models)
* [Multimeter support for models](multimeter_support)

Synapses mediate the signal flow between neuron or device models. They
can either be static or implement synaptic plasticity rules such as
STDP.

* [Synapses in NEST: An overview](synapses_overview)
* [Developing synapse models](synapse_models)

If you find your models written for NEST version 2.4 and prior not
working anymore in newer versions, the most likely reason is that we
have updated the API for neuron and synapse models. To make the
transision of models easier for you, there is a conversion guide:

* [Updating models for NEST 2.4 or prior to 2.6 or later](model_conversion_3g_4g)

## Dig deeper!

NEST is a complex piece of software with a [long
history](http://dx.doi.org/10.3389/conf.fninf.2013.09.00106). To get
you started at learning about it, there's a collection of documents
describing the NEST simulation kernel in historical order:

* Diesmann et al. (2002) [An environment for neural systems simulation](http://cns-classes.bu.edu/cn510/Papers/diesmann-gewaltig-02.pdf) explains the first version of the NEST simulation kernel.
* J.M. Eppler (2006) [A Multithreaded and Distributed System for the  Simulation of Large Biological Neural Networks](http://mindzoo.de/files/Diploma-JME.pdf) explains the general architecture of the first hybrid-parallel version of NEST, which is the 2nd generation of the simulation kernel.
* Helias et al. (2012) [Supercomputers ready for use as discovery machines for neuroscience.](http://dx.doi.org/10.3389/fninf.2012.00026) explains updates to the simulation kernel to enable the routinely use of supercomputers.
* Kunkel et al. (2014) [Spiking network simulation code for petascale computers](http://dx.doi.org/10.3389/fninf.2014.00078) introduced the 4th generation connection infrastructure of NEST, which reduces the memory consumption considerably and thus enables the use of the world's largest supercomputers.
* [Overview of scheduling and update strategies](simulation_loop_mindelay)

For even more information, see the [numerous
publications](http://www.nest-initiative.org/publications/) about the
technology behind NEST.

## Contributions welcome!

We are happily accepting contributions in the form of new models and
functions or general improvements to NEST. To have your code included,
it must adhere to some minimal coding and naming conventions:

* [Coding Guidelines for C++](coding_guidelines_c++)
* [Coding Guidelines for SLI](coding_guidelines_sli)
* [Naming convention for neuron models](neuron_model_naming)
* [Naming convention for synapse models](synapse_model_naming)
* [Naming convention for variables and parameters](variables_parameters_naming)

Once your code is in shape, head over to the [workflow
manual](development_workflow) to find out how to initiate the inclusion
by issuing a pull request.

You might also want to consider [becoming a
member](http://www.nest-initiative.org/membership/) in the NEST
Initiative.

## Further reading (aka needs-a-better-home)

* [Creating and handling Tokens and Datums in SLI](tokens_and_datums)
* [Xcode workflow](xcode_workflow)
* [Eclipse workflow](eclipse_workflow)

