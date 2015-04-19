---
layout: index
---

> NEST is a simulator for spiking neural networks. Development is
  coordinated by the [NEST Initiative](http://nest-initiative.org). For
  details about how to use NEST for your own modeling studies, see
  [http://nest-simulator.org](http://nest-simulator.org).

<hr>

# Here, you find the developer manual for NEST.

We use a private repository for internal development, while all
general NEST development takes place in the public repository. This
means that you can follow the development as it happens.

## Development infrastructure and workflow

NEST development is backed by multiple interlinked systems, most
notably,

* a central [source code repository](https://github.com/nest/nest-simulator) hosted on GitHub
* the associated GitHub [issue tracker](https://github.com/nest/nest-simulator/issues)
* continuous integration for pull requests provided by TravisCI
* self-hosted [mailing lists](http://www.nest-simulator.org/community/) based on Mailman.

The development workflow is based purely on pull requests. This means
that no direct commits to the repository are allowed, but all has to
go through the code review process unconditionally.

* [Development workflow for NEST](development_workflow)  
* [Code review guidelines](code_review_guidelines)  

## Extending NEST

If you intend to develop your own neuron or synapse models, the
easiest way to do so without messing with NEST's source code is to
write a plugin in the form of an extension module:

* [Writing an Extension Module](extension_modules)

* Developing neuron and device models
* Synapses in NEST: An overview
* Developing synapse models

If you find your models written for NEST version 2.4 and prior not
working in newer versions, this is because we have updated the API. To
make the transision easier for you, there is a conversion guide:

* [Updating models for NEST <= 2.4 to >= 2.6](model_conversion_3g_4g) 

## Contributions welcome!

We are happily accepting contributions in the form of new functions,
models or general improvements. To have your code included in NEST, it
must adhere to some minimal coding and naming conventions:

* [Coding Guidelines for C++](coding_guidelines_c++)  
* [Coding Guidelines for SLI](coding_guidelines_sli)  

* [Naming convention for neuron models](neuron_model_naming)  
* [Naming convention for synapse models](synapse_model_naming)  
* [Naming convention for variables and parameters](variables_parameters_naming)  

## Further reading

* [Multimeter support for models](multimeter_support)  
* [Creating and handling Tokens and Datums in SLI](tokens_and_datums)  
* [Continuous integration](continuous_integration)  
