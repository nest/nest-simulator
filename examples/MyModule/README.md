# NEST Extension Module Example

`MyModule` is an example extension module (i.e a "plugin") for
the [http://nest-simulator.org/](NEST Simulator). Extension modules
allow users to extend the functionality of NEST without messing with
the source code of NEST itself, thus making pulls from upstream NEST
easy, while allowing to extend NEST and sharing the extensions with
other researchers independently.

In order to showcase the possibilites of extension modules and their
use, `MyModule` contains the following (intentionally simple and more
or less silly) custom example components:

* A **neuron model** called `pif_psc_alpha`, which implements a
  *non*-leaky integrate-and-fire model with alpha-function shaped
  post-synaptic potentials.
* A **synapse model** called `drop_odd_spike_connection`, which drops
  all spikes that arrive ao odd-numbered points on the simulation time
  grid and delivers only those arriving at even-numbered grid points.
* A **connection builder** called `step_pattern_builder`, which
  creates step-pattern connectivity between the neurons of a source
  and a target population.
* A **recording backend** called `RecordingBackendSocket`, which
  streams out the data from spike detectors to an external (or local)
  server via UDP.
* A **recording backend** called `RecordingBackendSoundClick`, which creates the illusion
  of a realistic sound from an electrophysiological recording.

In addition to these C++ components, the `sli` directory contains
custom SLI code for the extension like custom SLI functions, example
code, or code to initialize the module (which is what the
`mymodule-init.sli` file is intended for).

## Adapting `MyModule`

If you want to create your own custom extension module using MyModule
as a start, you have to perform the following steps:

1. Replace all occurences of the strings `MyModule`, `mymodule`
   and`my` by something more descriptive and appropriate for your
   module.
2. Remove the example functionality you don't need.
3. Adapt the example functionality you do need to your needs.

More detailed information can be found in the NEST Developer Manual.
