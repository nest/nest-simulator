Stimulation from simulations
============================

The architecture of Nest has been modified to include a backend for
stimulating devices. This modification is inspired by the backend for
recording devices. (:doc:`recording from simulations <recording_simulations>`)

With Nest 3.0, we change the terminology of input device to stimulating device.
Nest 3.0 supports one stimulating backend, `MPI communication`. This backend is
available only when the Nest is compiled with `MPI` support. This is useful in
case of co-simulation and allows a closed loop simulation if it's coupled with the
MPI recording backend.

Changes
^^^^^^^

Added the new parameter `stimulus_source`, which can be used to select the
right backend for the stimulating device. By default, all stimulating devices
generate their own stimulating signals. If a backend, such as the `MPI` one
is selected, then the values for producing the stimulating signals are 
collected using the backend data input channel.
Added the parameter `label` for the MPI stimulating backend. This parameter
allows the user to select the file used for storing configuration details of 
the MPI connection.

All details about the new infrastructure can be found in the guide on
:doc:`stimulating a network <../../stimulating_the_network.rst>`.

