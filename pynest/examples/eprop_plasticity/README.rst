E-prop plasticity examples
==========================

.. image:: eprop_supervised_regression_sine-waves.png

Eligibility propagation (e-prop) :footcite:p:`Bellec2020` is a three-factor learning rule for spiking neural networks that approximates the
performance of backpropagation through time (BPTT). A complete e-prop model comprises a recurrent neuron model, a
readout neuron model, a synapse model, and a learning signal connection. Two such models are provided: the original
formulation by Bellec et al. (2020) :footcite:p:`Bellec2020` and an extended variant with additional biological features :footcite:p:`KorcsakGorzo2025`. The e-prop models are related as follows:

.. image:: eprop_model_relationships.png

We provide tutorials to reproduce the supervised regression task for generating temporal patterns and the supervised
classification task from the original `TensorFlow implementation <https://github.com/IGITUGraz/eligibility_propagation/blob/master/Figure_3_and_S7_e_prop_tutorials/>`__. In addition, we provide two tutorials on
supervised regression for generating two-dimensional temporal patterns and on supervised classification of neuromorphic
MNIST :footcite:p:`Orchard2015`.

References
----------

.. footbibliography::
