E-prop plasticity examples
==========================


.. image:: ../../../../pynest/examples/eprop_plasticity/eprop_supervised_regression_schematic_sine-waves.png

Eligibility propagation (e-prop) [1]_ is a three-factor learning rule for spiking neural networks
that approximates backpropagation through time. The original TensorFlow implementation of e-prop
was demonstrated, among others, on a supervised regression task to generate temporal patterns and a
supervised classification task to accumulate evidence [2]_. Here, you find tutorials on how to
reproduce these two tasks as well as two more advanced regression tasks using the NEST implementation
of e-prop [3]_ and how to visualize the simulation recordings.

The tutorials with names ending in `_bsshslm_2020` use this original e-prop model. The suffix _bsshslm_2020
follows the NEST convention to indicate in the model name the paper that introduced it by the first letter of
the authors' last names and the publication year. In contrast, the tutorials
without this suffix use an e-prop version with additional biological features described in [3]_.

Users interested in endowing an existing model with e-prop plasticity, may compare the .cpp and .h files of the
:doc:`iaf_psc_delta</models/iaf_psc_delta>` and :doc:`eprop_iaf_psc_delta</models/eprop_iaf_psc_delta>` model.
Parameters to run the `eprop_iaf_psc_delta` model are provided in
:doc:`eprop_supervised_regression_sine-waves.py <eprop_supervised_regression_sine-waves>`.

References
----------

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] https://github.com/IGITUGraz/eligibility_propagation/blob/master/Figure_3_and_S7_e_prop_tutorials/

.. [3] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
       van Albada SJ, Plesser HE, Bolten M, Diesmann M. Event-based implementation of
       eligibility propagation (in preparation)
