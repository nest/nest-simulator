<!-- TOC -->
-   [About NEST](about.md)
-   [Download](download.md)
-   [Features](features.md)
-   [Documentation](documentation.md)
    -   [Installing NEST](installation.md)
    -   [Introduction to PyNEST](introduction-to-pynest.md)
        -   [Part 1: Neurons and simple neural networks](part-1-neurons-and-simple-neural-networks.md)
        -   [Part 2: Populations of neurons](part-2-populations-of-neurons.md)
        -   [Part 3: Connecting networks with synapses](part-3-connecting-networks-with-synapses.md)
        -   [Part 4: Topologically structured networks](part-4-topologically-structured-networks.md)
    -   [Example Networks](examples/examples.md)
    -   [FAQ](frequently_asked_questions.md)
    -   [Developer Manual](http://nest.github.io/nest-simulator/)
-   [Publications](publications.md)
-   [Community](community.md)

<!-- /TOC -->

Part 3: Connecting networks with synapses
=========================================

Introduction
============

In this handout we look at using synapse models to connect neurons. After you have worked through this material, you will know how to:

-   set synapse model parameters before creation

-   define synapse models with customised parameters

-   use synapse models in connection routines

-   query the synapse values after connection

-   set synapse values during and after connection

For more information on the usage of NEST, please visit:
[nest-initiative.org/index.php/Software:Documentation](http://nest-initiative.org/index.php/Software:Documentation). To carry out the code snippets in this handout, you need to import `nest` and `numpy`.

Parameterising synapse models
=============================

NEST provides a variety of different synapse models. You can see the available models by using the command `Models(synapses)`, which picks only the synapse models out of the list of all available models.

Synapse models can be parameterised analogously to neuron models. You can discover the default parameter settings using `GetDefaults(model)` and set them with `SetDefaults(model,params)`:

``` {.sourceCode .python startfrom="1" language="Python" name="neuronpops" showstringspaces="false"}
nest.SetDefaults("stdp_synapse",{"tau_plus": 15.0})
```

Any synapse generated from this model will then have all the standard parameters except for the `tau_plus`, which will have the value given above.

Moreover, we can also create customised variants of synapse models using `CopyModel()`, exactly as demonstrated for neuron models:

``` {.sourceCode .python startfrom="auto" language="Python" name="neuronpops" showstringspaces="false"}
nest.CopyModel("stdp_synapse","layer1_stdp_synapse",{"Wmax": 90.0})
```

Now `layer1_stdp_synapse` will appear in the list returned by `Models()`, and can be used anywhere that a built-in model name can be used.

### STDP synapses

For the majority of synapses, all of their parameters are accessible via `GetDefaults()` and `SetDefaults()`. Synapse models implementing spike-timing dependent plasticity are an exception to this, as their dynamics are driven by the post-synaptic spike train as well as the pre-synaptic one. As a consequence, the time constant of the depressing window of STDP is a parameter of the post-synaptic neuron. It can be set as follows:

``` {.sourceCode .python startfrom="auto" language="Python" name="neuronpops" showstringspaces="false"}
nest.Create("iaf_neuron", params={"tau_minus": 30.0})
```

or by using any of the other methods of parameterising neurons demonstrated in the first two parts of this introduction.

Connecting with synapse models
==============================

The synapse model as well as parameters associated with the synapse type can be set in the synapse specification dictionary accepted by the connection routine.

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
conn_dict = {"rule": "fixed_indegree", "indegree": K}
syn_dict = {"model": "stdp_synapse", "alpha": 1.0}
nest.Connect(epop1, epop2, conn_dict, syn_dict)
```

If no synapse model is given, connections are made using the model `static_synapse`.

Querying the synapses
=====================

The function `GetConnections(source=None, target=None, synapse_model=None)` returns a list of connection identifiers that match the given specifications. There are no mandatory arguments. If it is called without any arguments, it will return all the connections in the network. If `source` is specified, as a list of one or more nodes, the function will return all outgoing connections from that population:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
nest.GetConnections(epop1)
```

Similarly, we can find the incoming connections of a particular target population by specifying `target` as a list of one or more nodes:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
nest.GetConnections(target=epop2)
```

will return all connections beween all neurons in the network and neurons in `epop2`. Finally, the search can be restricted by specifying a given synapse model:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
nest.GetConnections(synapse_model="stdp_synapse")
```

will return all the connections in the network which are of type `stdp_synapse`. The last two cases are slower than the first case, as a full search of all connections has to be performed.The arguments `source`, `target` and `synapse_model` can be used individually, as above, or in any conjunction:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
nest.GetConnections(epop1, epop2, "stdp_synapse")
```

will return all the connections that the neurons in `epop1` have to neurons in `epop2` of type `stdp_synapse`. Note that all these querying commands will only return the local connections, i.e. those represented on that particular MPI process in a distributed simulation.

Once we have the array of connections, we can extract data from it using `GetStatus()`. In the simplest case, this returns a list of dictionaries, containing the parameters and variables for each connection found by `GetConnections`. However, usually we don’t want all the information from a synapse, but some specific part of it. For example, if we want to check we have connected the network as intended, we might want to examine only the parameter `target` of each connection. We can extract just this information by using the optional `keys` argument of `GetStatus()`:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
conns = nest.GetConnections(epop1, synapse_model="stdp_synapse")
targets = nest.GetStatus(conns, "target")
```

The variable `targets` is now list of all the `target` values of the connections found. If we are interested in more than one parameter, `keys` can be a list of keys as well:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
conns = nest.GetConnections(epop1, synapse_model="stdp_synapse")
conn_vals = nest.GetStatus(conns, ["target","weight"])
```

The variable `conn_vals` is now a list of lists, containing the `target` and `weight` values for each connection found.

To get used to these methods of querying the synapses, it is recommended to try them out on a small network where all connections are known.

Distributing synapse parameters
===============================

The synapse parameters are specified in the synapse dictionary which is passed to the Connect-function. If the parameter is set to a scalar all connections will be drawn using the same parameter. Parameters can be randomly distributed by assigning a dictionary to the parameter. The dictionary has to contain the key `distribution` setting the target distribution of the parameters (for example `normal`). Optionally parameters associated with the distribution can be set (for example `mu`). Here we show an example where the parameters `alpha` and `weight` of the stdp synapse are uniformly distributed.

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
syn_dict = {"model": "stdp_synapse",
            "alpha": {"distribution": "uniform", "low": Min_alpha, "high": Max_alpha},
            "weight": {"distribution": "uniform", "low": Wmin, "high": Wmax},
            "delay": 1.0 }
nest.Connect(epop1, neuron, "all_to_all", syn_dict)
```

Available distributions and associated parameters are described in <http://www.nest-initiative.org/Connection_Management>, the most common ones are:

<table>
<colgroup>
<col width="50%" />
<col width="50%" />
</colgroup>
<thead>
<tr class="header">
<th align="left">Distributions
Keys</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td align="left"><code>normal</code>
<code>mu</code>, <code>sigma</code></td>
<td align="left"><code>lognormal</code>
<code>mu</code>, <code>sigma</code></td>
</tr>
</tbody>
</table>

Coding style
============

As your simulations become more complex, it is very helpful to develop a clean coding style. This reduces the number of errors in the first place, but also assists you to debug your code and makes it easier for others to understand it (or even yourself after two weeks). Here are some pointers, some of which are common to programming in general and some of which are more NEST specific. Another source of useful advice is [PEP-8](http://www.python.org/dev/peps/pep-0008/), which, conveniently, can be automatically checked by many editors and IDEs.

### Numbers and variables

Simulations typically have lots of numbers in them - we use them to set parameters for neuron models, to define the strengths of connections, the length of simulations and so on. Sometimes we want to use the same parameters in different scripts, or calculate some parameters based on the values of other parameters. It is not recommended to hardwire the numbers into your scripts, as this is error-prone: if you later decide to change the value of a given parameter, you have to go through all your code and check that you have changed every instance of it. This is particularly difficult to catch if the value is being used in different contexts, for example to set a weight in one place and to calculate the mean synaptic input in another.

A better approach is to set a variable to your parameter value, and then always use the variable name every time the value is needed. It is also hard to follow the code if the definitions of variables are spread throughout the script. If you have a parameters section in your script, and group the variable names according to function (e.g. neuronal parameters, synaptic parameters, stimulation parameters,...) then it is much easier to find and check them. Similarly, if you need to share parameters between simulation scripts, it is much less error-prone to define all the variable names in a separate parameters file, which the individual scripts can import. Thus a good rule of thumb is that numbers should only be visible in distinct parameter files or parameter sections, otherwise they should be represented by variables.

### Repetitive code, copy-and-paste, functions

Often you need to repeat a section of code with minor modifications. For example, you have two `multimeter`s and you wish to extract the recorded variable from each of them and then calculate its maximum. The temptation is to write the code once, then copy-and-paste it to its new location and make any necessary modifications:

``` {.sourceCode .python startfrom="1" language="Python" name="balancedneuron" showstringspaces="false"}
dma = nest.GetStatus(ma, keys="events")[0]
Vma = dma["Vm"]
amax = max(Vma)
dmb = nest.GetStatus(mb, keys="events")[0]
Vmb = dmb["Vm"]
bmax = max(Vmb)
print(amax-bmax)
```

There are two problems with this. First, it makes the main section of your code longer and harder to follow. Secondly, it is error-prone. A certain percentage of the time you will forget to make all the necessary modifications after the copy-and-paste, and this will introduce errors into your code that are hard to find, not only because they are semantically correct and so don’t cause an obvious error, but also because your eye tends to drift over them:

``` {.sourceCode .python startfrom="1" language="Python" name="balancedneuron" showstringspaces="false"}
dma = nest.GetStatus(multimeter1, keys="events")[0]
Vma = dma["Vm"]
amax = max(Vma)
dmb = nest.GetStatus(multimeter2, keys="events")[0]
Vmb = dmb["Vm"]
bmax = max(Vma)
print(amax-bmax)
```

The best way to avoid this is to define a function:

``` {.sourceCode .python startfrom="1" language="Python" name="balancedneuron" showstringspaces="false"}
def getMaxMemPot(Vdevice):
    dm = nest.GetStatus(Vdevice, keys="events")[0]
    return max(dm["Vm"])
```

Such helper functions can usefully be stored in their own section, analogous to the parameters section. Now we can write down the functionality in a more concise and less error-prone fashion:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
amax = getMaxMemPot(multimeter1)
bmax = getMaxMemPot(multimeter2)
print(amax-bmax)
```

If you find that this clutters your code, as an alternative you can write a `lambda` function as an argument for `map`, and enjoy the feeling of smugness that will pervade the rest of your day. A good policy is that if you find yourself about to copy-and-paste more than one line of code, consider taking the few extra seconds required to define a function. You will easily win this time back by spending less time looking for errors.

### Subsequences and loops

When preparing a simulation or collecting or analysing data, it commonly happens that we need to perform the same operation on each node (or a subset of nodes) in a population. As neurons receive ids at the time of creation, it is possible to use your knowledge of these ids explictly:

``` {.sourceCode .python startfrom="1" language="Python" name="balancedneuron" showstringspaces="false"}
Nrec = 50
neuronpop = nest.Create("iaf_neuron", 200)
sd = nest.Create("spike_detector")
nest.Connect(range(1,N_rec+1),sd,"all_to_all")
```

However, this is *not at all recommended!*. This is because as you develop your simulation, you may well add additional nodes - this means that your initially correct range boundaries are now incorrect, and this is an error that is hard to catch. To get a subsequence of nodes, use a *slice* of the relevant population:

``` {.sourceCode .python startfrom="4" language="Python" name="balancedneuron" showstringspaces="false"}
nest.Connect(neuronpop[:Nrec],spikedetector,"all_to_all")
```

An even worse thing is to use knowledge about neuron ids to set up loops:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
for n in range(1,len(neuronpop)+1):
    nest.SetStatus([n], {"V_m": -67.0})
```

Not only is this error prone as in the previous example, the majority of PyNEST functions are expecting a list anyway. If you give them a list, you are reducing the complexity of your main script (good) and pushing the loop down to the faster C++ kernel, where it will run more quickly (also good). Therefore, instead you should write:

``` {.sourceCode .python startfrom="5" language="Python" name="balancedneuron" showstringspaces="false"}
nest.SetStatus(neuronpop, {"V_m": -67.0})
```

See Part 2 for more examples on operations on multiple neurons, such as setting the status from a random distribution and connecting populations.

If you really really need to loop over neurons, just loop over the population itself (or a slice of it) rather than introducing ranges:

``` {.sourceCode .python startfrom="auto" language="Python" name="balancedneuron" showstringspaces="false"}
for n in neuronpop:
    my_weird_function(n)
```

Thus we can conclude: instead of range operations, use slices of and loops over the neuronal population itself. In the case of loops, check first whether you can avoid it entirely by passing the entire population into the function - you usually can.

Command overview[sec:Command-overview]
======================================

These are the new functions we introduced for the examples in this handout.

### Querying Synapses

`GetConnections(neuron, synapse_model="None"))`:  
Return an array of connection identifiers.

Parameters:
`source` - list of source GIDs
`target` - list of target GIDs
`synapse_model` - string with the synapse model
If GetConnections is called without parameters, all connections in the network are returned. If a list of source neurons is given, only connections from these pre-synaptic neurons are returned. If a list of target neurons is given, only connections to these post-synaptic neurons are returned. If a synapse model is given, only connections with this synapse type are returned. Any combination of source, target and synapse\_model parameters is permitted. Each connection id is a 5-tuple or, if available, a NumPy array with the following five entries: source-gid, target-gid, target-thread, synapse-id, port
Note: Only connections with targets on the MPI process executing the command are returned.
