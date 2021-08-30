Stimulation backends
====================

In previous versions of NEST, each stimulation device stored an
internal representation of the data it needed for generating the
stimuli for connected nodes.

In NEST 3.0, the interface for stimulation devices has been extended
by a means to get this data from an external source instead of setting
it directly from your PyNEST script. Such an external source could be
another simulator, or a generic signal generator toolkit.

Technically, this new feature is implemented through stimulation backends that can be based on NEST has been extended by an architecture to select of Nest has been modified to include a backend for
stimulation devices. This modification is inspired by the backend for
recording devices. (:doc:`recording from simulations <recording_simulations>`)

With NEST 3.0, we change the terminology of input device to stimulation device.
NEST 3.0 supports one stimulation backend, `MPI communication`. This backend is
available only when the NEST is compiled with MPI support. This is useful in
case of co-simulation and allows a closed loop simulation if it's coupled with the
MPI recording backend.

Changes
^^^^^^^

We added the new parameter ``stimulus_source``, which can be used to select
a backend for the stimulation device. By default, all stimulation
devices generate their own stimulation signals based on their
parametrization.  If a backend, such as ``mpi`` is selected, the
values for producing the stimuli are received from the backend's input
channel.

All details about the new infrastructure can be found in the guide on
:doc:`stimulating the network <../../stimulate_the_network.rst>`.
