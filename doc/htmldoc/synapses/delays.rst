Right now, transmission delays can be specified in three ways and can be set heterogeneously per connection (synapse):
```cpp
neuron = nest.Create("iaf_psc_alpha")
```
1.
```cpp
nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom", "delay": 1.0})
```
2.
```cpp
conn = nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom"})
nest.SetStatus(conn, {"delay": 1.0})
```
3.
```cpp
nest.SetDefaults("stdp_pl_synapse_hom", {"delay": 1.0})
```

- Delays are considered fully dendritic by all built-in models, however, the interpretation of the "delay" value is per-se model-specific.

Now it is also possible to specify both heterogeneous axonal and dendritic delays with different parameter names
- It is not allowed anymore to use "delay" as parameter name for these models because of ambiguity
- The parameter names "dendritic_delay" and "axonal_delay" have to be used. If not provided, a dendritic delay of 1ms and 0ms axonal delay are assumed.
- If only axonal delay is provided and no dendritic delay, the dendritic delay is assumed to be 0 and vice-versa.

- Existing models need to be adapted, see developer documentation axonal_delays.rst
- Neuron models must support STDP with axonal delays, if no STDP is used, any neuron model works
- Currently, only iaf_psc_alpha supports STDP with axonal delays and only stdp_pl_synapse_hom_ax_delay implements STDP with axonal delays
