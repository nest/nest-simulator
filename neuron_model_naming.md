---
layout: index
---

[« Back to the index](index)

<hr>

# Conventions for `model names`

For model names we want to use the formula:

    dynamics + [element type] + [psc | cond] + synaptic dynamics + [numerics]

And therefore we get:

    iaf_psc_delta        IaF neuron with delta-current input pulses
    iaf_psc_exp          IaF with exponentially decaying input current
    iaf_psc_alpha        IaF with alpha-shaped input current
    iaf_psc_alpha_presc  same, with prescient numerics
    iaf_psc_alpha_canon  same, with canonical numerics
    iaf_cond_alpha       IaF with conductance-based alpha input

A further label could specify the number and interpretation of the `rports`. For example: `exin` for neurons with different dynamics for excitatory and inhibitory synapses, e.g. `iaf_psc_exin_alpha` to denote "2 dendrites".

More specific and complicated neuron models are probably better described by a project name than by a full list of all their components: e.g. use `facets_neuron` instead of `iaf_psc_exp_sfa_exin_fancy_something`.
