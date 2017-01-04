---
layout: index
---

[« Back to the index](index)

<hr>

# Conventions for neuron model names

For model names we want to use the formula:

    dynamics + [element type] + [psc | cond] + synaptic dynamics + [numerics]

And therefore we get:

    iaf_psc_delta        IaF neuron with delta-current input pulses
    iaf_psc_exp          IaF with exponentially decaying input current
    iaf_psc_alpha        IaF with alpha-shaped input current
    iaf_psc_alpha_presc  same, with prescient numerics
    iaf_psc_alpha_canon  same, with canonical numerics
    iaf_cond_alpha       IaF with conductance-based alpha input

A further label could specify the number and interpretation of the
`rports`, for example `iaf_psc_alpha_multisynapse`.

For models based on specific publications the general naming rule is

    <model category>_<initials>_year

Examples are
- `iaf_chs_2007` (Carandini M, Horton JC, Sincich LC (2007) Thalamic filtering of retinal
   spike trains by postsynaptic summation. J Vis 7(14):20,1-11)
- `iaf_chxk_2008` (Casti A, Hayot F, Xiao Y, and Kaplan E (2008) A simple model of retina-LGN
transmission. J Comput Neurosci 24:235-252)
- `iaf_tum_2000` (Tsodyks M, Uziel A, Markram H (2000) Synchrony Generation
   in Recurrent Networks with Frequency-Dependent Synapses. J Neurosci
   20:RC50, 1-5)

We haven't applied these rules completely consistently, but for neuron
models that are mainly known from a single paper and that have not
been given a more general name in the publications that introduced
them (e.g. `aeif` and `amat` models), one should stick to this rule.



