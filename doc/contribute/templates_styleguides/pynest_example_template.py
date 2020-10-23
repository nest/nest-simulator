# -*- coding: utf-8 -*-
#
# pynest_example_template.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.
"""Template demonstrating how to create examples for PyNEST
----------------------------------------------------------------
[[ Titles should be one line and state what the example does.
  It should begin with a verb in the present tense and include type of model
  and/or method]]

[[ Extended summary - a detailed explanation of your example. Try to answer the
   folowing questions. ]]
[[ What does this script do? What is the purpose? ]]
This template demonstrates how to create an example Python script for
NEST.
Copy this file and replace the sample text with a description of your
example script.

Make sure to remove any text that is irrelevant for your example!

The format is based on `NumPy style docstring
<https://numpydoc.readthedocs.io/en/latest/format.html>`_ and uses
reStructuredText markup. Please review the syntax rules if you are
unfamiliar with either reStructuredText or NumPy style docstrings.
Your example should contain a complete code-block that begins with all
necessary imports and ends with code that displays the output.

Below is a more concrete example of how an extended summary could look like:

This script simulates a neuron by an excitatory and an inhibitory
population of neurons firing a Poisson spike train.
Optimization is performed using the `bisection` method from Scipy,
which simulates the network repeatedly.

The aim of this example script is to find a firing rate for the inhibitory
population that will make the neuron fire at the same rate as the excitatory
population.

[[ What kind of output is expected? ]]

The output shows the target neuron's membrane potential as a function of time.

[[ Does this example have a real world application or use case?
  Are there particular applications or areas of research that would benefit
  from this example? You can reference relevant papers that this example
  may be based on]]

This model used here is applicable for neurorobotics, particularly cases of ...

[[ If applicable, state any prerequisite the reader needs to have installed or
   configured that is not standard ]]

Please ensure that you have configured MUSIC to be ON in your NEST
configuration:
``cmake -Dwith-music=[ON</path/to/music>]``

[[ If applicable, mention the literature reference for this example.
   Note the syntax of the citation. And don't forget to add a "References"
   section! ]]

This model used here corresponds to the formulation presented in
Sander et al. [1]_ and the bisection method developed in
Gewaltig and Diesmann [2]_.

[[ See Also section - Include a couple of related examples, models,
   or functions. ]]

See Also
---------
:ref:`Intrinsic current subthreshold <label_name>`
:doc:`some other doc </path/to/filename>`

Notes
------
[[ Additional information can be included here regarding background theory,
   relevant mathematical equations etc. ]]

The value of :math:`\omega` is X.
Time-averaged from the spiking simulation:
[[ Note the syntax used for displaying equations uses reStructuredText
   directive with LaTeX math formulae ]]

.. math::

    X(e^{j\omega } ) = x(n)e^{ - j\omega n}

 * you can use the asterisk for bullet items
 * bullet points are usually more easily read than paragraphs

References
----------
    [[ Note the format of the reference. No bold nor italics is used. Last name
       of author(s) followed by year, title in sentence case and full name of
       journal followed by volume and page range. Include the doi if
       applicable. ]]

.. [1] Sander M., et al. (2011). Biology of the sauropod dinosaurs: The
       evolution of gigantism. Biological Reviews. 86(1):117-155.
       https://doi.org/10.111/j.1469-185x.2010.00137.x

.. [2] Gewaltig M-O, Diesmann M (2007). NEST (Neural Simulation Tool).
       Scholarpedia 2(4):1430.

[[ Include your name in the author section, so we know who contributed.
  Author(s) should be comma separated with first name as initials followed
  by last name ]]

:Authors: D Adams, N Gaiman
"""
import nest  # [[ begin code section with imports]]
import scipy

###############################################################################
# [[After the initial docstring above, all following comment blocks must begin
# with a line of hashes and each line of a block must begin with a hash.
# This will allow us to generate nice looking examples for the website! ]]
#
# The excitatory `poisson_generator` (`noise[0]`) and the voltmeter are
# configured using `SetStatus`, which expects a list of node handles and
# a list of parameter dictionaries.
# The rate of the inhibitory Poisson generator is set later.
# Note that we do not need to set parameters for the neuron and the
# spike recorder, since they have satisfactory defaults.

nest.SetStatus(noise, [{"rate": n_ex * r_ex}, {"rate": n_in * r_in}])
nest.SetStatus(voltmeter, {"withgid": True, "withtime": True})

complete code ...


##############################################################################
# Finally, we plot the target neuron's membrane potential as a function of time

nest.voltage_trace.from_device(voltmeter)  # [[ end with output ]]
