""" The Summary Line - Balanced Neuron Example
----------------------------------------------------------------

Example text for extended summary section.

This model implements X according to the Y model described by [1]_ and [2]_.

This script simulates a neuron driven by an excitatory and an inhibitory
population of neurons firing Poisson spike trains. The aim is to find a firing
rate for the inhibitory population that will make the neuron fire at the same
rate as the excitatory population.

Optimization is performed using the ``bisection`` method from Scipy,
simulating the network repeatedly.


Notes
------

Additional information can be included here regarding background theory etc.

The value of :math:`\omega` is X.
For the population and time-averaged from the spiking simulation:

.. math::

    X(e^{j\omega } ) = x(n)e^{ - j\omega n}

 * use the asterisk for bullet items
 * second item

References
------------

.. [1] Sander M., et al. 2011. Biology of the sauropod dinosaurs: The
       evolution of gigantism. Biological Reviews. 86(1):117-155.
       https://doi.org/10.111/j.1469-185x.2010.00137.x

.. [2] Francillon-Vieillot H, et al. 1990. Microstructure and mineralization of 
       vertebrate skeletal tissues. In: Carter J ed. Skeletal Biomineralization 
       Patterns, Processes and Evolutionary Trends. New York: Van Nostrand
       Reinhold, 471–530.

See Also
---------

intrinisic_current_spiking
intrisic_current_subthreshold

:Authors:
    Adams, D

KEYWORDS: scipy, poisson spike train, integrate and fire
"""

import nest # begin with imports

################################################################################
# The excitatory ``poisson_generator`` (``noise[0]``) and the voltmeter are
# configured using ``SetStatus``, which expects a list of node handles and
# a list of parameter dictionaries.
# The rate of the inhibitory Poisson generator is set later.
# Note that we do not need to set parameters for the neuron and the
# spike detector, since they have satisfactory defaults.

nest.SetStatus(noise, [{"rate": n_ex * r_ex}, {"rate": n_in * r_in}])
nest.SetStatus(voltmeter, {"withgid": True, "withtime": True})

################################################################################
# Finally, we plot the target neuron's membrane potential as a function of time

nest.voltage_trace.from_device(voltmeter) # end with output
