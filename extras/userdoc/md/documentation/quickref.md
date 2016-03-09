NEST/SLI Quick Reference
========================

Table of Contents
-----------------

-   [Introduction](#introduction)
-   [The online-help system](#the-online-help-system)
-   [Simulation kernel](#simulation-kernel)
-   [Models and nodes](#models-and-nodes)
-   [Connections](#connection)
-   [Network access](#network-access)
-   [Stack Commands](#stack-commands)
-   [Arithmetic functions](#arithmetic-functions)
-   [Boolean functions](#boolean-functions)
-   [Special mathematical functions](#special-mathematical-functions)
-   [Random number generation](#random-number-generation)
-   [Statistics](#statistics)
-   [Arrays, vectors, and matrices](#arrays-vectors-and-matrices)

<!-- -->

-   [Strings](#strings)
-   [Dictionaries and Namespaces](#dictionaries-and-Namespaces)
-   [Names, functions, and variables](#names-functions-and-variables)
-   [Loop structures](#loop-structures)
-   [Control structures](#control-structures)
-   [Functions and procedures](#functions-and-procedures)
-   [Error handling and debugging](#error-handling-and-debugging])
-   [Files and streams](#files-and-streams)
-   [Executing SLI programs](#executing-sli-programs)
-   [Interfacing the host's file-system](#interfacing-the-host-s-file-system)
-   [Process control](#process-control)
-   [SLI Interpreter Control](#sli-interpreter-control)
-   [Parallel computing](#parallel-computing)



-   [Alphabetical command index](../helpindex/cmd_index.html)

Introduction
------------

SLI is the simulation language of the NEST simulation system. SLI is a stack oriented language, i.e. each command expects to find its arguments on the stack.
 When a SLI command is executed, it usually removes all arguments from the stack and pushes one or more results back on the stack.

This document presents a directory of the most important SLI operators, grouped into sections. Since this document is not automatically generated from the help pages, it might not contain the latest additions to SLI.

[↑ Table of contents ↑](#toc)

The on-line help system
-----------------------

### Layout of help pages

For most SLI commands, there exists help information which can be viewed either as plain ASCII text or in HTML-format. The structure of a help entry resembles that of a UNIX mamual page. It consists of several sections which are explained below:

Name:

*[namespace::]Command*

 

 

Synopsis:

*arg1 ... argn Command -\> res1 ... resn*

*arg1*

*argn*

*argn*

*-\>*

*resn*

Parameters:

 

*[result]*

Description:

Options:

*SetOptions*

 

*namespace::*

Examples:

Variants:

Bugs:

Diagnostics:

Author:

FirstVersion:

Remarks:

Availability:

References:

SeeAlso:

Source:

### Accessing the on-line help

The following commands are available to access NEST's on-line help facilities.

[help](../helpindex/cmds/help/index.html) Display help for a given symbol. [helpdesk](../helpindex/cmds/helpdesk/index.html) Open the on-line help main page in an HTML browser. [helpindex](../helpindex/cmds/helpindex-cmd/index.html) Display a list of all documented commands. [page](../helpindex/cmds/page/index.html) The pager program to use for help output. [apropos](../helpindex/cmds/apropos/index.html) Search the command index for a regular expression.

[↑ Table of contents ↑](#toc)

Simulation kernel
-----------------

### Controlling the simulation

[Simulate](../helpindex/cmds/Simulate/index.html) Simulate n milliseconds. [ResumeSimulation](../helpindex/cmds/ResumeSimulation/index.html) Resume an interrupted simulation. [ResetKernel](../helpindex/cmds/ResetKernel/index.html) Put the simulation kernel back to its initial state. [ResetNetwork](../helpindex/cmds/ResetNetwork/index.html) Reset the dynamic state of the network. [reset](../helpindex/cmds/reset/index.html) Reset dictionary stack and clear the userdict.

### Setting and retrieving kernel parameters

Most kernel parameters are accessed via the root node's status dictionary. The id of the root node is 0. It can be inspected and modified like all other network nodes.

[kernel](../helpindex/cmds/kernel/index.html) Global properties of the simulation kernel. [SetStatus](../helpindex/cmds/SetStatus/index.html) Modify status of an element. [GetStatus](../helpindex/cmds/GetStatus/index.html) Return the status dictionary of an element. [ShowStatus](../helpindex/cmds/ShowStatus/index.html) Show the status dictionary of a network node.

### Memory consumption

[MemoryInfo](../helpindex/cmds/MemoryInfo/index.html) Report the current memory usage. [memory\_thisjob](../helpindex/cmds/memory_thisjob/index.html) Report virtual memory size for current NEST process.

[↑ Table of contents ↑](#toc)

Models and network nodes
------------------------

All nodes in NEST (i.e. neurons, devices, and subnetworks/index.html) are derived from a common base class. Therefore they share some general properties.

[Node](../helpindex/cmds/Node/index.html) General properties of all nodes. [modeldict](../helpindex/cmds/modeldict/index.html) Dictionary with NEST model objects.

### Constructing nodes

[Create](../helpindex/cmds/Create/index.html) Create network elements in the current subnet. [LayoutNetwork](../helpindex/cmds/LayoutNetwork/index.html) Create a nested multi-dimensional network structure in the current subnet.

### Neuron Models

[iaf\_neuron](../helpindex/cmds/iaf_neuron), [iaf\_psc\_alpha](../helpindex/cmds/iaf_psc_alpha/index.html) Leaky integrate-and-fire neuron model. [iaf\_psc\_delta](../helpindex/cmds/iaf_psc_delta/index.html) Leaky integrate-and-fire neuron model. [iaf\_psc\_exp](../helpindex/cmds/iaf_psc_exp/index.html) Leaky integrate-and-fire neuron model with exponential PSCs [iaf\_cond\_alpha](../helpindex/cmds/iaf_cond_alpha/index.html) Simple conductance based leaky integrate-and-fire neuron model. [iaf\_cond\_exp](../helpindex/cmds/iaf_cond_exp/index.html) Simple conductance based leaky integrate-and-fire neuron model. [iaf\_cond\_alpha\_mc](../helpindex/cmds/iaf_cond_alpha_mc/index.html) Multi-compartment conductance-based leaky integrate-and-fire neuron model. [iaf\_tum\_2000](../helpindex/cmds/iaf_tum_2000/index.html) Leaky integrate-and-fire neuron model with exponential PSCs. [hh\_psc\_alpha](../helpindex/cmds/hh_psc_alpha/index.html) Hodgkin Huxley neuron model. [hh\_cond\_exp\_traub](../helpindex/cmds/hh_cond_exp_traub/index.html) Hodgin Huxley based model, Traub modified.

### Device Models

[Device](../helpindex/cmds/Device/index.html) General properties of devices. [RecordingDevice](../helpindex/cmds/RecordingDevice/index.html) Common properties of all recording devices. [StimulatingDevice](../helpindex/cmds/StimulatingDevice/index.html) General properties of stimulating devices.

#### Spike input devices

The following devices generate sequences of spikes which can be send to a neuron. These devices act like populations of neurons and connected to their targets like a neuron.

[pulsepacket\_generator](../helpindex/cmds/pulsepacket_generator/index.html) Device to produce Gaussian spike-trains. [poisson\_generator](../helpindex/cmds/poisson_generator/index.html) Generate Poisson spike trains. [spike\_generator](../helpindex/cmds/spike_generator/index.html) Generates spike-events from an array.

#### Analog input devices

[dc\_generator](../helpindex/cmds/dc_generator/index.html) Generate a DC current. [ac\_generator](../helpindex/cmds/ac_generator/index.html) Generate a sinusoidal alternating current. [noise\_generator](../helpindex/cmds/noise_generator/index.html) Generate a Gaussian noise current. [layerdc\_generator](../helpindex/cmds/layerdc_generator/index.html) Generate a DC current for a neuron layer. [step\_current\_generator](../helpindex/cmds/step_current_generator/index.html) Provides a piecewise constant DC input current.

#### Recording devices

[spike\_detector](../helpindex/cmds/spike_detector/index.html) Device to record spiking activity from one or more nodes. [correlation\_detector](../helpindex/cmds/correlation_detector/index.html) Device for evaluating cross correlation between two spike sources. [multimeter](../helpindex/cmds/multimeter/index.html) Device to record analog data from neurons. [voltmeter](../helpindex/cmds/voltmeter/index.html) Device to observe membrane potentials. [conductancemeter](../helpindex/cmds/conductancemeter/index.html) Device to observe synaptic conductances.

[↑ Table of contents ↑](#toc)

Connecting nodes
----------------

[synapsedict](../helpindex/cmds/synapsedict/index.html) Dictionary containing all synapse models. [Connect](../helpindex/cmds/Connect/index.html) Connect two network nodes. [ConvergentConnect](../helpindex/cmds/ConvergentConnect/index.html) Connect many source nodes to one target node. [DivergentConnect](../helpindex/cmds/DivergentConnect/index.html) Connect one source node to many target nodes. [RandomConvergentConnect](../helpindex/cmds/RandomConvergentConnect/index.html) Randomly connect one source node to many target nodes. [RandomDivergentConnect](../helpindex/cmds/RandomDivergentConnect/index.html) Randomly connect one source node to many target nodes. [BinomialConvergentConnect](../helpindex/cmds/BinomialConvergentConnect/index.html) Connect a target to a binomial number of sources.

### Topological connections

[topology::CreateLayer](../helpindex/cmds/topology::CreateLayer/index.html) Creates topological node layer. [topology::ConnectLayers](../helpindex/cmds/topology::ConnectLayers/index.html) Connect two layers. [topology::GetElement](../helpindex/cmds/topology::GetElement/index.html) Return node GID at specified layer position. [topology::GetPosition](../helpindex/cmds/topology::GetPosition/index.html) Retrieve position of input node. [topology::GetRelativeDistance](../helpindex/cmds/topology::GetRelativeDistance/index.html) Retrieve a vector of distance between two nodes. [topology::LayerGidPositionMap](../helpindex/cmds/topology::LayerGidPositionMap/index.html) Prints layer node positions to file. [topology::PrintLayerConnections](../helpindex/cmds/topology::PrintLayerConnections/index.html) Prints a list of the connections of the nodes in the layer to file.

[↑ Table of contents ↑](#toc)

Looking at node, networks, and connectivity
-------------------------------------------

### Navigating the network

[CurrentSubnet](../helpindex/cmds/CurrentSubnet/index.html) Return the GID of the current network node. [ChangeSubnet](../helpindex/cmds/ChangeSubnet/index.html) Change the curent working subnet to a specified position. [PrintNetwork](../helpindex/cmds/PrintNetwork/index.html) Print network tree in readable form. [NetworkDimensions](../helpindex/cmds/NetworkDimensions/index.html) Returns an array with the dimensions of a structured subnet. [GetGlobalNodes](../helpindex/cmds/GetGlobalNodes/index.html) Return IDs of all nodes of a subnet. [GetGlobalLeaves](../helpindex/cmds/GetGlobalLeaves/index.html) Return IDs of all leaves of a subnet. [GetGlobalChildren](../helpindex/cmds/GetGlobalChildren/index.html) Return IDs of all immediate child nodes of a subnet. [GetLocalNodes](../helpindex/cmds/GetLocalNodes/index.html) Return IDs of all nodes of a subnet local to the MPI process executing the command. [GetLocalLeaves](../helpindex/cmds/GetLocalLeaves/index.html) Return IDs of all leaves of a subnet local to the MPI process executing the command. [GetLocalChildren](../helpindex/cmds/GetLocalChildren/index.html) Return IDs of all immediate child nodes of a subnet local to the MPI process executing the command.

### Investigating connectivity

[FindConnections](../helpindex/cmds/FindConnections/index.html) Find connections that fulfil the given criteria. [SetStatus](../helpindex/cmds/SetStatus/index.html) Modify status of an element. [GetStatus](../helpindex/cmds/GetStatus/index.html) Return the status dictionary of an element.

### Setting and retrieving parameters of network elements

[SetStatus](../helpindex/cmds/SetStatus/index.html) Modify status of an element. [GetStatus](../helpindex/cmds/GetStatus/index.html) Return the status dictionary of an element. [ShowStatus](../helpindex/cmds/ShowStatus/index.html) Show the status dictionary of a network node. [type](../helpindex/cmds/type/index.html) Return the type of an element. [ResetKernel](../helpindex/cmds/ResetKernel/index.html) Put the simulation kernel back to its initial state. [ResetNetwork](../helpindex/cmds/ResetNetwork/index.html) Reset the dynamic state of the network. [reset](../helpindex/cmds/reset/index.html) Reset dictionary stack and clear the userdict.

[↑ Table of contents ↑](#toc)

Stacks
------

### Operand stack

#### Stack contents

[stack](../helpindex/cmds/stack/index.html) Display stack contents. [pstack](../helpindex/cmds/pstack/index.html) Display stack contents in syntax form. [inspect](../helpindex/cmds/inspect/index.html) Inspect an object. [typestack](../helpindex/cmds/typestack/index.html) Display type information of the stack. [operandstack](../helpindex/cmds/operandstack/index.html) Store the contents of the stack in an array. [restoreostack](../helpindex/cmds/restoreostack/index.html) Restore the stack from an array.

#### Counting stack levels

[count](../helpindex/cmds/count/index.html) Count the number of objects on the stack. [counttomark](../helpindex/cmds/counttomark/index.html) Count number of objects on the stack from top to marker. [mark](../helpindex/cmds/mark/index.html) Push a mark-object on the stack.

#### Copying stack elements

[dup](../helpindex/cmds/dup/index.html) Duplicate the object which is on top of the stack. [over](../helpindex/cmds/over/index.html) Copy stack object at level 1. [copy](../helpindex/cmds/copy/index.html) Copy the first n stack levels. [pick](../helpindex/cmds/pick/index.html) Copy object from stack level n. [index](../helpindex/cmds/index/index.html) Copy object from stack level n.

#### Removing stack elements

[pop](../helpindex/cmds/pop/index.html) Pop the top object off the stack. [npop](../helpindex/cmds/npop/index.html) Pop n object off the stack. [clear](../helpindex/cmds/clear/index.html) Clear the entire stack.

#### Rearranging stack elements

[exch](../helpindex/cmds/exch/index.html) Exchange the order of the first two stack objects. [roll](../helpindex/cmds/roll/index.html) Roll a portion n stack levels k times. [rolld](../helpindex/cmds/rolld/index.html) Roll the three top stack elements downwards. [rollu](../helpindex/cmds/rollu/index.html) Roll the three top stack elements upwards. [rot](../helpindex/cmds/rot/index.html) Rotate entire stack contents.

### Execution-stack access

[execstack](../helpindex/cmds/execstack/index.html) Return the contents of the execution stack as array. [restoreestack](../helpindex/cmds/restoreestack/index.html) Restore the execution stack from an array.

[↑ Table of contents ↑](#toc)

Arithmetic functions
--------------------

[abs](../helpindex/cmds/abs/index.html) Absolute value of a number. [add](../helpindex/cmds/add/index.html) Add two numbers or vectors. [sub](../helpindex/cmds/sub/index.html) Subtract two numbers or vectors. [mul](../helpindex/cmds/mul/index.html) Multiply two numbers or vectors (point-wise). [div](../helpindex/cmds/div/index.html) Divide two numbers or vectors (point-wise). [inv](../helpindex/cmds/inv/index.html) Compute 1/x. [mod](../helpindex/cmds/mod/index.html) Compute the modulo of two integer numbers. [neg](../helpindex/cmds/neg/index.html) Reverse sign of a number. [ceil](../helpindex/cmds/ceil/index.html) Return nearest integer larger than the argument. [round](../helpindex/cmds/round/index.html) Round double to the nearest integer. [trunc](../helpindex/cmds/trunc/index.html) Truncate decimals of a double. [floor](../helpindex/cmds/floor/index.html) Return nearest integer smaller than the argument. [pow](../helpindex/cmds/pow/index.html) Raise a number to a power. [sqr](../helpindex/cmds/sqr/index.html) Compute the square of a number. [sqrt](../helpindex/cmds/sqrt/index.html) Compute the square root of a non-negative number. [exp](../helpindex/cmds/exp/index.html) Calculate the exponential of double number. [ln](../helpindex/cmds/ln/index.html) Calculate natural logarithm of double number. [log](../helpindex/cmds/log/index.html) Calculate decadic logarithm of double number. [frexp](../helpindex/cmds/frexp/index.html) Decomposes its argument into an exponent of 2 and a factor. [modf](../helpindex/cmds/modf/index.html) Decomposes its argument into fractional and integral part. [cos](../helpindex/cmds/cos/index.html) Calculate the cosine of double number. [sin](../helpindex/cmds/sin/index.html) Calculate the sine of double number. [max](../helpindex/cmds/max/index.html) Return the greater of two values. [min](../helpindex/cmds/min/index.html) Return the smaller of two values.

[↑ Table of contents ↑](#toc)

Boolean functions
-----------------

### Comparison functions

[eq](../helpindex/cmds/eq/index.html) Test two objects for equality. [gt](../helpindex/cmds/gt/index.html) Test if one object is greater than another object. [geq](../helpindex/cmds/geq/index.html) Test if one object is greater or equal than another object. [lt](../helpindex/cmds/lt/index.html) Test if one object is less than another object. [leq](../helpindex/cmds/leq/index.html) Test if one object is less or equal than another object. [neq](../helpindex/cmds/neq/index.html) Test two objects for inequality. [Min](../helpindex/cmds/Min/index.html) Returns the smallest element of an array. [Max](../helpindex/cmds/Max/index.html) Returns the largest element of an array.

### Boolean operators

[and](../helpindex/cmds/and/index.html) Logical and operator. [or](../helpindex/cmds/or/index.html) Logical or operator. [not](../helpindex/cmds/not/index.html) Logical not operator. [xor](../helpindex/cmds/xor/index.html) Logical xor operator.

[↑ Table of contents ↑](#toc)

Special mathematical functions
------------------------------

[Erf](../helpindex/cmds/Erf/index.html) Error function. [Erfc](../helpindex/cmds/Erfc/index.html) Complementary error function. [Gammainc](../helpindex/cmds/Gammainc/index.html) Incomplete Gamma function. [GaussDiskConv](../helpindex/cmds/GaussDiskConv/index.html) Convolution of an excentric Gaussian with a disk. [CyclicValue](../helpindex/cmds/CyclicValue/index.html) Project a cyclic value onto it's norm interval (e.g. angle on [0,360)). [FractionalPart](../helpindex/cmds/FractionalPart/index.html) Return fractional part of the argument. [IntegerPart](../helpindex/cmds/IntegerPart/index.html) Return integer part of the argument. [UnitStep](../helpindex/cmds/UnitStep/index.html) The unit step function (aka Heavyside function). [LambertW](../helpindex/cmds/LambertW/index.html) Simple iteration implementing the Lambert-W function.

### Mathematical constants

[E](../helpindex/cmds/E/index.html) Euler constant. [Pi](../helpindex/cmds/Pi/index.html) Pi constant.

[↑ Table of contents ↑](#toc)

Random number generation
------------------------

[seed](../helpindex/cmds/seed/index.html) Set the seed of a random number generator. [irand](../helpindex/cmds/irand/index.html) Generate a random integer number. [drand](../helpindex/cmds/drand/index.html) Generate a random double number. [rngdict](../helpindex/cmds/rngdict/index.html) Dictionary of random generator types. [rdevdict](../helpindex/cmds/rdevdict/index.html) Dictionary of random deviate types. [CreateRNG](../helpindex/cmds/CreateRNG/index.html) Create a random number generator. [CreateRDV](../helpindex/cmds/CreateRDV/index.html) Create a random deviate. [Random](../helpindex/cmds/Random/index.html) Returns a random number. [RandomArray](../helpindex/cmds/RandomArray/index.html) Returns array with random numbers. [RandomSubset](../helpindex/cmds/RandomSubset/index.html) Random subset of an arry without repetitions. [GetStatus](../helpindex/cmds/GetStatus/index.html) Return the property dictionary of a random deviate generator. [SetStatus](../helpindex/cmds/SetStatus/index.html) Modify the properties of a random deviate generator.

### Random deviate generator types

[binomial](../helpindex/cmds/rdevdict::binomial/index.html) Binomial random deviate generator. [exponential](../helpindex/cmds/rdevdict::exponential/index.html) Exponential random deviate generator. [gamma](../helpindex/cmds/rdevdict::gamma/index.html) Gamma random deviate generator. [normal](../helpindex/cmds/rdevdict::normal/index.html) Normal random deviate generator. [normal\_clipped](../helpindex/cmds/rdevdict::normal_clipped/index.html) Clipped normal random deviate generator. [normal\_clipped\_left](../helpindex/cmds/rdevdict::normal_clipped_left/index.html) Left clipped normal random deviate generator. [normal\_clipped\_right](../helpindex/cmds/rdevdict::normal_clipped_right/index.html) Right clipped normal random deviate generator. [poisson](../helpindex/cmds/rdevdict::poisson/index.html) Poisson random deviate generator. [uniform\_int](../helpindex/cmds/rdevdict::uniformint/index.html) Uniform integer random deviate generator.

[↑ Table of contents ↑](#toc)

Statistics
----------

[Min](../helpindex/cmds/Min/index.html) Returns the smallest element of an array. [Max](../helpindex/cmds/Max/index.html) Returns the largest element of an array. [Total](../helpindex/cmds/Total/index.html) Returns the sum of the elements of an array. [Mean](../helpindex/cmds/Mean/index.html) Returns the mean of the elements of an array. [Variance](../helpindex/cmds/Variance/index.html) Returns the unbiased variance of the elements of an array. [StandardDeviation](../helpindex/cmds/StandardDeviation/index.html) Returns the standard deviation of the element of an array.

[↑ Table of contents ↑](#toc)

Arrays, vectors, and matrices
-----------------------------

### Construction

[array](../helpindex/cmds/array/index.html) Construct array with n zeros (PS). [arraystore](../helpindex/cmds/arraystore/index.html) Pops the first n elements of the stack into an array. [LayoutArray](../helpindex/cmds/LayoutArray/index.html) Create a multi-dimensional array. [Range](../helpindex/cmds/Range/index.html) Generate array with range of numbers. [Table](../helpindex/cmds/Table/index.html) Generate an array according to a given function. [ReadList](../helpindex/cmds/ReadList/index.html) Read a list of specified format from a stream. [GaborPatch](../helpindex/cmds/arr::GaborPatch/index.html) Create a two-dimensional array filled with values from the Gabor function. [GaussPatch](../helpindex/cmds/arr::GaussPatch/index.html) Create a two-dimensional array filled with values from the Gauss function.

### Conversions

[cva](../helpindex/cmds/cva/index.html) Convert dictionary/trie to array. [cst](../helpindex/cmds/cst/index.html) Convert string to array of tokens. [cvlit](../helpindex/cmds/cvlit/index.html) Convert name/string/procedure to literal/array. [cvx](../helpindex/cmds/cvx/index.html) Convert array/string to procedure. [cv1d](../helpindex/cmds/cv1d/index.html) Convert 2-dimensional coordinates to 1-dim index. [cv2d](../helpindex/cmds/cv2d/index.html) Convert 1-dimensional index to 2-dim coordinate. [Export](../helpindex/cmds/Export/index.html) Save in a foreign file format. [MathematicaToSliIndex](../helpindex/cmds/MathematicaToSliIndex/index.html) Convert Mathematica-like indices to SLI indices. [SliToMathematicaIndex](../helpindex/cmds/SliToMathematicaIndex/index.html) Convert SLI indices to Mathematica-like indices.

### Insertion

[put](../helpindex/cmds/put/index.html) Put indexed object into container. [insert](../helpindex/cmds/insert/index.html) Insert an object in a container at a specific position. [insertelement](../helpindex/cmds/insertelement/index.html) Insert an element to a container at a specific position. [join](../helpindex/cmds/join/index.html) Join two containers of the same type. [JoinTo](../helpindex/cmds/JoinTo/index.html) Join with container referenced by l-value. [append](../helpindex/cmds/append/index.html) Append object to container. [AppendTo](../helpindex/cmds/AppendTo/index.html) Append to container referenced by l-value. [prepend](../helpindex/cmds/prepend/index.html) Attach an object to the front of a container.

### Inspecting an array or matrix

[empty](../helpindex/cmds/empty/index.html) Tests if a string or array is empty. [length](../helpindex/cmds/length/index.html) Counts elements of an object. [size](../helpindex/cmds/size/index.html) Returns the size of an array/string. [GetMin](../helpindex/cmds/GetMin/index.html) Get minimal element. [GetMax](../helpindex/cmds/GetMax/index.html) Get maximal element. [ArrayQ](../helpindex/cmds/ArrayQ/index.html) Returns true if top object is an array. [MatrixQ](../helpindex/cmds/MatrixQ/index.html) Test if a SLI array is a (hyper-rectengular/index.html) matrix. [Dimensions](../helpindex/cmds/Dimensions/index.html) Determine dimenstions of a (hyper-rectangular/index.html) SLI array. [TensorRank](../helpindex/cmds/TensorRank/index.html) Estimate the rank of a tensor.

### Retrieving elements from an array

[get](../helpindex/cmds/get/index.html) Retrieve indexed Object from a container. [Part](../helpindex/cmds/Part/index.html) Returns parts of an array. [Take](../helpindex/cmds/Take/index.html) Extract sequences from an array. [getinterval](../helpindex/cmds/getinterval/index.html) Return a subsequence of a container. [erase](../helpindex/cmds/erase/index.html) Deletes a subsequece of a container. [First](../helpindex/cmds/First/index.html) Return the first element of an array. [Last](../helpindex/cmds/Last/index.html) Return the last element of an array. [Rest](../helpindex/cmds/Rest/index.html) Remove the first element of an array and return the rest. [Select](../helpindex/cmds/Select/index.html) Reduces an array to elements which fulfill a criterion. [Sort](../helpindex/cmds/Sort/index.html) Sorts a homogeneous array of doubles. [arrayload](../helpindex/cmds/arrayload/index.html) Pushes array elements followed by number of elements. [area](../helpindex/cmds/area/index.html) Return array of indices defining a 2d subarea of a 2d array. [area2](../helpindex/cmds/area/index.html) Return array of indices defining a 2d subarea of a 2d array.

#### Removing and replacing elements

[putinterval](../helpindex/cmds/putinterval/index.html) Replace sections of an array/string (PS). [erase](../helpindex/cmds/erase/index.html) Deletes a subsequece of a container. [replace](../helpindex/cmds/replace/index.html) Replace a section of a container by a new sequence. [ReplaceOccurrences](../helpindex/cmds/ReplaceOccurrences/index.html) Replace the occurences of a key in a container. [breakup](../helpindex/cmds/breakup/index.html) Break a string or an array at given Substrings or SubArrays. [trim](../helpindex/cmds/trim/index.html) Delete leading/trailing elements in a container.

#### Operations on lists and arrays

[Partition](../helpindex/cmds/Partition/index.html) Partition list into n element pieces. [Flatten](../helpindex/cmds/Flatten/index.html) Flatten out a nested list. [Reform](../helpindex/cmds/arr::Reform/index.html) Reform the dimensions of a hyperrectengular array. [Select](../helpindex/cmds/Select/index.html) Reduces an array to elements which fulfill a criterion. [Split](../helpindex/cmds/Split/index.html) Splits array into subarrays of sequences of identical elements. [MergeLists](../helpindex/cmds/MergeLists/index.html) Merges sorted lists. [MemberQ](../helpindex/cmds/MemberQ/index.html) Checks if array contains a specific element. [HasDifferentMemberQ](../helpindex/cmds/HasDifferentMemberQ/index.html) Checks if array contains an element different from given value.

### Functional operations on lists and arrays

[Map](../helpindex/cmds/Map/index.html) Apply a procedure to each element of a list or string. [MapAt](../helpindex/cmds/MapAt/index.html) Apply a procedure to some elements of a list or string. [MapIndexed](../helpindex/cmds/MapIndexed/index.html) Apply a function to each element of a list/string. [MapThread](../helpindex/cmds/MapThread/index.html) Apply a procedure to to corresponding elements of n arrays. [FixedPoint](../helpindex/cmds/FixedPoint/index.html) Apply a procedure repeatedly until the result is an invariant. [NestList](../helpindex/cmds/NestList/index.html) Gives a list of the results of applying f to x 0 through n times. [Nest](../helpindex/cmds/Nest/index.html) Apply a function n times. [FoldList](../helpindex/cmds/FoldList/index.html) Gives a list of the results of repeatedly applying a function with two parameters. [Fold](../helpindex/cmds/Fold/index.html) Result of repeatedly applying a function with two arguments. [ScanThread](../helpindex/cmds/ScanThread/index.html) Apply a procedure to corresponding elements of n arrays, not returing results. [forall](../helpindex/cmds/forall/index.html) Call a procedure for each element of a list/string. [forallindexed](../helpindex/cmds/forallindexed/index.html) Call a procedure for each element of a list/string. [EdgeClip](../helpindex/cmds/arr::EdgeClip/index.html) Clip 2-d array indices at array edges. [EdgeTruncate](../helpindex/cmds/arr::EdgeTruncate/index.html) Truncate 2-d array indices at array edges. [EdgeWrap](../helpindex/cmds/arr::EdgeWrap/index.html) Wrap 2-d array indices around edges (toriodal). [IndexWrap](../helpindex/cmds/arr::IndexWrap/index.html) Project a cyclic index value onto interval [0,N).

### Arrays as vectors and matrices

#### Dimensions and rank

[MatrixQ](../helpindex/cmds/MatrixQ/index.html) Test whether a nested array is a matrix. [Dimensions](../helpindex/cmds/Dimensions/index.html) Determine dimensions of an array. [TensorRank](../helpindex/cmds/TensorRank/index.html) Determine the level to which an array is a full vector.

#### Operations on vectors and matrices

[add](../helpindex/cmds/add/index.html) Add two numbers or vectors. [sub](../helpindex/cmds/sub/index.html) Subtract two numbers or vectors. [mul](../helpindex/cmds/mul/index.html) Multiply two numbers or vectors (point-wise). [div](../helpindex/cmds/div/index.html) Divide two numbers or vectors (point-wise). [Dot](../helpindex/cmds/Dot/index.html) Product of vectors, matrices, and tensors. [Plus](../helpindex/cmds/Plus/index.html) Sum of all vector components. [Times](../helpindex/cmds/Times/index.html) Product of all vector components. [Transpose](../helpindex/cmds/Transpose/index.html) Transposes the first two levels of its argument. [reverse](../helpindex/cmds/reverse/index.html) Reverse a string or array. [rotate](../helpindex/cmds/rotate/index.html) Rotate an array. [OuterProduct](../helpindex/cmds/OuterProduct/index.html) Outer product.

### Memory management

[capacity](../helpindex/cmds/capacity/index.html) Returns the capacity of an array. [reserve](../helpindex/cmds/reserve/index.html) Bring an array to a certain capacity. [resize](../helpindex/cmds/resize/index.html) Change the internal size of an array. [shrink](../helpindex/cmds/shrink/index.html) Reduce the capacity of an array to its minimum.

[↑ Table of contents ↑](#toc)

Strings
-------

### Construction

[=](../helpindex/cmds/=/index.html) Display top operand stack object. [==](../helpindex/cmds/==/index.html) Display top operand stack object. [==only](../helpindex/cmds/==only/index.html) Display top operand stack object without linefeed. [=only](../helpindex/cmds/=only/index.html) Display top operand stack object without linefeed. [getline](../helpindex/cmds/getline/index.html) Read an entire line from an input stream. [gets](../helpindex/cmds/gets/index.html) Read white space terminated string from stream. [Read](../helpindex/cmds/Read/index.html) Read an object of a certain type from a stream. [ReadWord](../helpindex/cmds/ReadWord/index.html) Read white space terminated string from stream.

### Conversion

[cvs](../helpindex/cmds/cvs/index.html) Convert object to string. [cst](../helpindex/cmds/cst/index.html) Convert string to array of tokens. [cvd\_s](../helpindex/cmds/cvd_s/index.html) Convert string to double. [token\_s](../helpindex/cmds/token_s/index.html) Read a token from a string.

### Insertion

[length](../helpindex/cmds/length/index.html) Counts elements of an object. [put](../helpindex/cmds/put/index.html) Put indexed object into container. [putinterval](../helpindex/cmds/putinterval/index.html) Replace sections of an array/string (PS). [insert](../helpindex/cmds/insert/index.html) Insert an object in a container at a specific position. [insertelement](../helpindex/cmds/insertelement/index.html) Insert an element to a container at a specific position. [prepend](../helpindex/cmds/prepend/index.html) Attach an object to the front of a container. [append.](../helpindex/cmds/append/index.html) Append object to container. [join](../helpindex/cmds/join/index.html) Join two containers of the same type.

### Retrieving characters and substrings

[empty](../helpindex/cmds/empty/index.html) Tests if a string or array is empty. [get](../helpindex/cmds/get/index.html) Lookup indexed Object of a container. [getinterval](../helpindex/cmds/getinterval/index.html) Return a subsequence of a container. [erase](../helpindex/cmds/erase/index.html) Deletes a subsequece of a container. [First](../helpindex/cmds/First/index.html) Return the first element of an array/string. [Last](../helpindex/cmds/Last/index.html) Return the last element of an array/string. [Rest](../helpindex/cmds/Rest/index.html) Remove the first element of an array/string and return the rest. [search](../helpindex/cmds/search/index.html) Search for a sequence in an array or string. [searchif](../helpindex/cmds/searchif/index.html) Check wether a substring is contained within a string.

### Removing and replacing

[erase](../helpindex/cmds/erase/index.html) Deletes a subsequece of a container. [replace](../helpindex/cmds/replace/index.html) Replace a section of a container by a new sequence. [ReplaceOccurrences](../helpindex/cmds/ReplaceOccurrences/index.html) replace the occurences of a key in a container. [breakup](../helpindex/cmds/breakup/index.html) Break a string or an array at given sub-strings or sub-arrays. [trim](../helpindex/cmds/trim/index.html) Delete leading/trailing elements in a container.

### Operations on strings

[empty](../helpindex/cmds/empty/index.html) Tests if a string or array is empty. [length](../helpindex/cmds/length/index.html) Counts elements of an object. [size](../helpindex/cmds/size/index.html) Returns the size of an array/string. [forall](../helpindex/cmds/forall/index.html) Call a procedure for each element of a list/string. [forallindexed](../helpindex/cmds/forallindexed/index.html) Call a procedure for each element of a list/string. [Map](../helpindex/cmds/Map/index.html) Apply a procedure to each element of a list or string. [MapIndexed](../helpindex/cmds/MapIndexed/index.html) Apply a function to each element of a list/string. [reverse](../helpindex/cmds/reverse/index.html) Reverse a string or array. [ToUppercase](../helpindex/cmds/ToUppercase/index.html) Convert a string to upper case. [ToLowercase](../helpindex/cmds/ToLowercase/index.html) Convert a string to lower case.

### Regular expressions

[regcomp](../helpindex/cmds/regcomp/index.html) Create a regular expression. [regex\_find](../helpindex/cmds/regex_find/index.html) Check if a regex is included in a string or stream. [regex\_find\_r](../helpindex/cmds/regex_find_r/index.html) Check if a regex is included in a string. [regex\_find\_rf](../helpindex/cmds/regex_find_rf/index.html) Check if a regex is included in a stream. [regex\_find\_s](../helpindex/cmds/regex_find_s/index.html) Check if a regex is included in a string. [regex\_find\_sf](../helpindex/cmds/regex_find_sf/index.html) Check if a regex is included in a stream. [regex\_replace](../helpindex/cmds/regex_replace/index.html) Replace all occurences of a regex. [regexec](../helpindex/cmds/regexec/index.html) Compare string and regular expression.

[↑ Table of contents ↑](#toc)

Dictionaries
------------

### Construction

[\<\<\>\>](../helpindex/cmds/%3C%3C%3E%3E/index.html) construct a dictionary. [dict](../helpindex/cmds/dict/index.html) Create new, empty dictionary. [clonedict](../helpindex/cmds/clonedict/index.html) Create a copy of a dictionary. [SaveDictionary](../helpindex/cmds/SaveDictionary/index.html) Save contents of a dictionary to a file. [RestoreDictionary](../helpindex/cmds/RestoreDictionary/index.html) Read a dictionary definition from a file. [MergeDictionary](../helpindex/cmds/MergeDictionary/index.html) Merge all definitions of a dictionary with the current dicitonary.

### Conversion

[cva](../helpindex/cmds/cva/index.html) Convert dictionary/trie to array.

### Insertion and lookup

[using](../helpindex/cmds/using/index.html) Add a namespace (or dictionary/index.html) to the local scope, keeping the current dictionary. [endusing](../helpindex/cmds/endusing/index.html) Close the scope of a 'using' context. [get](../helpindex/cmds/get/index.html) Lookup indexed Object of a container. [call](../helpindex/cmds/call/index.html) Execute object from a dictionary (or namespace, see below). [put](../helpindex/cmds/put/index.html) Put indexed object into container. [put\_d](../helpindex/cmds/put_d/index.html) Add an entry to a dictionary. [known](../helpindex/cmds/known/index.html) Check whether a name is defined in a dictionary. [info](../helpindex/cmds/info/index.html) Display the contents of a dictionary. [info\_ds](../helpindex/cmds/info_ds/index.html) Print contents of all dictionaries on the dicitonary stack to stream. [topinfo\_d](../helpindex/cmds/topinfo_d/index.html) Print contents of top dictionary to stream. [length](../helpindex/cmds/length/index.html) Counts elements of a container object. [SubsetQ](../helpindex/cmds/SubsetQ/index.html) Test if one dictionary is a subset of another.

### Key removal

[undef](../helpindex/cmds/undef/index.html) Remove a key from a dictionary. [cleardict](../helpindex/cmds/cleardict/index.html) Clears the contents of a dictionary.

### Special dictionaries

[errordict](../helpindex/cmds/errordict/index.html) Pushes error dictionary on operand stack. [modeldict](../helpindex/cmds/modeldict/index.html) Dictionary with neural model objects. [synapsedict](../helpindex/cmds/synapsedict/index.html) Dictionary containing all synapse models. [statusdict](../helpindex/cmds/statusdict/index.html) Dictionary with platform dependent status. [signaldict](../helpindex/cmds/signaldict/index.html) Dictionary containing the machine-dependent signal codes. [libdict](../helpindex/cmds/libdict/index.html) Dictionary of provided libraries and their components. [elementstates](../helpindex/cmds/elementstates/index.html) Dictionary with symbolic element state tag. [ReadModes](../helpindex/cmds/ReadModes/index.html) Dictionary with type specifiers for read functions. [OptionsDictionary](../helpindex/cmds/OptionsDictionary/index.html) Dictionary for global options.

### Dictionary Stack

[dictstack](../helpindex/cmds/dictstack/index.html) Return current dictionary stack as array. [begin](../helpindex/cmds/begin/index.html) Open a dictionary. [end](../helpindex/cmds/end/index.html) Closes the current dictionary. [currentdict](../helpindex/cmds/currentdict/index.html) Return topmost dictionary of the dictionary stack. [lookup](../helpindex/cmds/lookup/index.html) Search for a key in each dictionay on the dictionary stack. [who](../helpindex/cmds/who/index.html) List contents of the top-level dicitonary. [whos](../helpindex/cmds/whos/index.html) List contents of all dictionaries on the dicitonary stack. [countdictstack](../helpindex/cmds/countdictstack/index.html) Return number of dictionaries on the dictionary stack. [cleardictstack](../helpindex/cmds/cleardictstack/index.html) Pop all non-standard dictionaries of the stack.

### Namespaces

Dictionaries can be used to limit names to a certain scope. This is useful for logical grouping, and to prevent name conflicts, very much like the use of namespaces in C++. Furthermore, it keeps the system dictionary from being littered.

 

When refering to a variable or routine of limited scope, its scope shall be indicated by the notation namespace::name (e.g. arr::Reform). If no namespace is specified, systemdict:: is implicitely assumed, meaning unlimited scope.

*Note:* Please note that the notation namespace::name is not yet supported in program code. Use the command call instead.

[namespace](../helpindex/cmds/namespace/index.html) Open or create a namespace dictionary.
[call](../helpindex/cmds/call/index.html) Execute object from a namespace (or dictionary, see above).
[::](../helpindex/cmds/::/index.html/index.html) Execute a symbol from a nested namespace.

[↑ Table of contents ↑](#toc)

Names, functions, and variabes
------------------------------

### Defining variables and functions

[def](../helpindex/cmds/def/index.html) Define a variable or function. [Set](../helpindex/cmds/Set/index.html) Same as def with reversed arguments. [undef](../helpindex/cmds/undef/index.html) Remove a key from a dictionary. [SLIFunctionWrapper](../helpindex/cmds/SLIFunctionWrapper/index.html) Define a SLI function with lots of comfort.

### Show defined variables and functions

[who](../helpindex/cmds/who/index.html) List contents of the top-level dicitonary. [whos](../helpindex/cmds/whos/index.html) List contents of all dictionaries on the dicitonary stack. [lookup](../helpindex/cmds/lookup/index.html) Search for a key in each dictionay on the dictionary stack. [known](../helpindex/cmds/known/index.html) Check whether a name is defined in a dictionary.

### Names and scoping

[begin](../helpindex/cmds/begin/index.html) Open a dictionary. [end](../helpindex/cmds/end/index.html) Closes the current dictionary. [lookup](../helpindex/cmds/lookup/index.html) Search for a key in each dictionay on the dictionary stack. [load](../helpindex/cmds/load/index.html) Search for a key in each dictionay on the dictionary stack.

[↑ Table of contents ↑](#toc)

Loops
-----

### Infinite loops

[loop](../helpindex/cmds/loop/index.html) Repeatedly execute a procedure. [exit](../helpindex/cmds/exit/index.html) Exit a loop construct.

### Loops with counters

[for](../helpindex/cmds/for/index.html) Execute a procedure for a sequence of numbers. [repeat](../helpindex/cmds/repeat/index.html) Execute a procedure n times. [FindRoot](../helpindex/cmds/FindRoot/index.html) Numerically find a root in an interval. [exit](../helpindex/cmds/exit/index.html) Exit a loop construct.

### Loops over containers

[forall](../helpindex/cmds/forall/index.html) Call a procedure for each element of a list/string. [forallindexed](../helpindex/cmds/forallindexed/index.html) Call a procedure for each element of a list/string. [Map](../helpindex/cmds/Map/index.html) Apply a procedure to each element of a list or string. [MapIndexed](../helpindex/cmds/MapIndexed/index.html) Apply a function to each element of a list/string. [MapThread](../helpindex/cmds/MapThread/index.html) Apply a procedure to corresponding elements of n arrays, returning the result. [ScanThread](../helpindex/cmds/ScanThread/index.html) Apply a procedure to corresponding elements of n arrays, not returing results. [NestList](../helpindex/cmds/NestList/index.html) Gives a list of the results of applying f to x 0 through n times. [FoldList](../helpindex/cmds/FoldList/index.html) Repeatedly apply a function with two parameters. [exit](../helpindex/cmds/exit/index.html) Exit a loop construct.

[↑ Table of contents ↑](#toc)

Control structures
------------------

[if](../helpindex/cmds/if/index.html) Conditionaly execute a procedure. [ifelse](../helpindex/cmds/ifelse/index.html) Conditionaly execute a procedure. [case](../helpindex/cmds/case/index.html) Like if, but test a series of conditions. [switch](../helpindex/cmds/switch/index.html) Finish a case ... switch structure. [switchdefault](../helpindex/cmds/switchdefault/index.html) Finish a case ... switchdefault structure. [stop](../helpindex/cmds/stop/index.html) Raise a stop signal. [stopped](../helpindex/cmds/stopped/index.html) Returns true if execution was stopped by stop.

[↑ Table of contents ↑](#toc)

Functions and procedures
------------------------

### Object oriented techniques

[call](../helpindex/cmds/call/index.html) Execute object from a dictionary.

### Type checking

#### Accessing type information

[type](../helpindex/cmds/type/index.html) Return the type of an object. [typeinfo](../helpindex/cmds/typeinfo/index.html) Return the type of an object. [LiteralQ](../helpindex/cmds/LiteralQ/index.html) Returns true if top object is a literal. [NumberQ](../helpindex/cmds/NumberQ/index.html) Returns true if top object is a number (int or double). [StringQ](../helpindex/cmds/StringQ/index.html) Returns true if top object is a string.

#### Type tries

A type-trie is a lookup structure which can be used to implement automatic type checking as well as operator overloading.

[def](../helpindex/cmds/def/index.html) Define a variable or function. [SLIFunctionWrapper](../helpindex/cmds/SLIFunctionWrapper/index.html) Define a SLI function with lots of comfort. [trie](../helpindex/cmds/trie/index.html) Create a new type-trie object. [addtotrie](../helpindex/cmds/addtotrie/index.html) Add a function variant to a trie-object. [cva](../helpindex/cmds/cva/index.html) Convert dictionary/trie to array. [cva\_t](../helpindex/cmds/cva_t/index.html) Converts a type trie to an equivalent array. [cvt\_a](../helpindex/cmds/cvt_a/index.html) Converts an array to the equivalent type trie.

### Type Conversions

[cva](../helpindex/cmds/cva/index.html) Convert dictionary/trie to array. [cst](../helpindex/cmds/cst/index.html) Convert string to array of tokens. [cvi](../helpindex/cmds/cvi/index.html) Convert double/string to integer. [cvd](../helpindex/cmds/cvd/index.html) Convert integer/string to double. [cvlit](../helpindex/cmds/cvlit/index.html) Convert name/string/procedure to literal/array. [cvn](../helpindex/cmds/cvn/index.html) Convert literal/string to name. [cvx](../helpindex/cmds/cvx/index.html) Convert array/string to procedure.

### Options

[Options](../helpindex/cmds/Options/index.html) Define a new set of options for a given name. [GetOption](../helpindex/cmds/GetOption/index.html) Get the value of a procedure option. [GetOptions](../helpindex/cmds/GetOptions/index.html) Get all options for a given name. [SetOptions](../helpindex/cmds/SetOptions/index.html) Set options for a given name. [SaveOptions](../helpindex/cmds/SaveOptions/index.html) Temporarily save options of a command [ResetOptions](../helpindex/cmds/ResetOptions/index.html) Reset all options of a command to their default values. [RestoreOptions](../helpindex/cmds/RestoreOptions/index.html) Restore the temporarily saved options of a command. [ShowOptions](../helpindex/cmds/ShowOptions/index.html) Display all options for a given name. [OptionsDictionary](../helpindex/cmds/OptionsDictionary/index.html) Dictionary for global options.

### Optimization

[bind](../helpindex/cmds/bind/index.html) Recursively replaces executable operator names by their values.

[↑ Table of contents ↑](#toc)

Error handling and debugging
----------------------------

If an error occurs, execution is usually interrupted and a diagnostic message is printed. In most cases, the stack is restored to the state where it was immediately before the command that raised the error was called.

 

If the error was raised from within a procedure, the stack might not be restored to the state before the procedure was called.

[handleerror](../helpindex/cmds/handleerror/index.html) Default error handler. [stop](../helpindex/cmds/stop/index.html) Raise a stop signal. [stopped](../helpindex/cmds/stopped/index.html) Returns true if execution was stopped by stop. [raiseerror](../helpindex/cmds/raiseerror/index.html) Raise an error to the system. [raiseagain](../helpindex/cmds/raiseagain/index.html) Re-raise the last error. [resume](../helpindex/cmds/resume/index.html) Resume interrupted SLI program after a system signal. [setguard](../helpindex/cmds/setguard/index.html) Limit the number of interpreter cycles. [break](../helpindex/cmds/break/index.html) Interrupt the execution of a procedure for inspection. [continue](../helpindex/cmds/continue/index.html) Continue an interrupted procedure. [assert](../helpindex/cmds/assert/index.html) Assert that procedure returns true. [currentname](../helpindex/cmds/currentname/index.html) Returns the most recently resolved name. [debug.sli](../helpindex/cmds/debug.sli/index.html) Debugging support for SLI.

[↑ Table of contents ↑](#toc)

Files and Streams
-----------------

### Standard input and output

[cin](../helpindex/cmds/cin/index.html) Standard input stream. [cout](../helpindex/cmds/cout/index.html) Standard output stream. [cerr](../helpindex/cmds/cerr/index.html) Standard error output stream. [setprecision](../helpindex/cmds/setprecision/index.html) Set precision for decimal place of a stream. [inspect](../helpindex/cmds/inspect/index.html) Inspect an object. [=](../helpindex/cmds/=/index.html) Display top operand stack object. [==](../helpindex/cmds/==/index.html) Display top operand stack object. [==only](../helpindex/cmds/==only/index.html) Display top operand stack object without linefeed. [=only](../helpindex/cmds/=only/index.html) Display top operand stack object without linefeed. [print](../helpindex/cmds/print/index.html) Print object to a stream. [pprint](../helpindex/cmds/pprint/index.html) Pretty print object to a stream.

### Opening and Closing

[file](../helpindex/cmds/file/index.html) Opens file for reading or writing. [ifstream](../helpindex/cmds/ifstream/index.html) Open file stream for reading. [xifstream](../helpindex/cmds/xifstream/index.html) Create an executable input-stream. [searchfile](../helpindex/cmds/searchfile/index.html) Tries to open a file for reading using the search-path. [searchifstream](../helpindex/cmds/searchifstream/index.html) Searches SLI's search path for a file. [ofstream](../helpindex/cmds/ofstream/index.html) Open a file stream for writing. [ofsopen](../helpindex/cmds/ofsopen/index.html) Open an existing file for appending or writing. [osstream](../helpindex/cmds/osstream/index.html) Create a string-stream object. [pipe](../helpindex/cmds/pipe/index.html) Open up a pipe. [mkfifo](../helpindex/cmds/mkfifo/index.html) Create a FIFO special file (named pipe). [close](../helpindex/cmds/close/index.html) Close a stream. [tmpnam](../helpindex/cmds/tmpnam/index.html) Return valid nonexisting file name. [dup2](../helpindex/cmds/dup2/index.html) Duplicate a filestream's file descriptor onto another's. [isatty](../helpindex/cmds/isatty/index.html) Determine if a stream is connected to a terminal.

### Reading from streams

#### Checking for data

[setNONBLOCK](../helpindex/cmds/setNONBLOCK/index.html) Switch between blocking and non-blocking I/O. [in\_avail](../helpindex/cmds/in_avail/index.html) Check for available data in an input stream's buffer. [available](../helpindex/cmds/available/index.html) Check if data is available from an istream. [ignore](../helpindex/cmds/ignore/index.html) Ignore any waiting data on an istream.

#### Unformatted string input

[getc](../helpindex/cmds/getc/index.html) Read single character from input stream. [gets](../helpindex/cmds/gets/index.html) Read white space terminated string from stream. [getline](../helpindex/cmds/getline/index.html) Read a newline terminated string from an input stream. [readline](../helpindex/cmds/readline/index.html) Read and edit a line from standard input. [GNUreadline](../helpindex/cmds/GNUreadline/index.html) Read and edit a line from standard input. [GNUaddhistory](../helpindex/cmds/GNUaddhistory/index.html) Add a string to the readline-history. [oldgetline](../helpindex/cmds/oldgetline/index.html) "Old", ignorant version of getline.

#### Formatted input

[str](../helpindex/cmds/str/index.html) Retrieve a string from a string-stream. [token](../helpindex/cmds/token/index.html) Read a token from a stream or string. [token\_is](../helpindex/cmds/token_is/index.html) Read a token from an input stream. [token\_s](../helpindex/cmds/token_s/index.html) Read a token from a string. [Read](../helpindex/cmds/Read/index.html) Read an object of a certain type from a stream. [ReadDouble](../helpindex/cmds/ReadDouble/index.html) Read a double from a stream. [ReadInt](../helpindex/cmds/ReadInt/index.html) Read an integer from a stream. [ReadList](../helpindex/cmds/ReadList/index.html) Read a list of specified format from a stream. [ReadWord](../helpindex/cmds/ReadWord/index.html) Read white space terminated string from stream. [ReadModes](../helpindex/cmds/ReadModes/index.html) Dictionary with type specifiers for read functions.

#### Formatted Output

[Export](../helpindex/cmds/Export/index.html) Save in a foreign file format.

### Writing to streams

[print](../helpindex/cmds/print/index.html) Print object to a stream. [pprint](../helpindex/cmds/pprint/index.html) Pretty print object to a stream. [endl](../helpindex/cmds/endl/index.html) Line break. [flush](../helpindex/cmds/flush/index.html) Force the buffer of a stream to be flushed. [setprecision](../helpindex/cmds/setprecision/index.html) Set precision for decimal place of a stream.

### Handling flags of streams

[iclear](../helpindex/cmds/iclear/index.html) Clear the state-flags of input stream. [oclear](../helpindex/cmds/oclear/index.html) Clear the state-flags of output stream. [ifail](../helpindex/cmds/ifail/index.html) Check the "fail"-flag of an input stream. [ieof](../helpindex/cmds/ieof/index.html) Check the "eof"-flag of an input stream. [oeof](../helpindex/cmds/oeof/index.html) Check the "eof"-flag of an output stream. [eof](../helpindex/cmds/eof/index.html) Check eof status of a stream. [igood](../helpindex/cmds/igood/index.html) Check the "good"-flag of a stream. [ogood](../helpindex/cmds/ogood/index.html) Check the "good"-flag of an output stream. [good](../helpindex/cmds/good/index.html) Check good status of a stream.

### Handling Image Files

[readPGM](../helpindex/cmds/readPGM/index.html) Read in grey-level image in PGM Format. [writePGM](../helpindex/cmds/writePGM/index.html) Erite out a grey-level image in PGM format.

### Editing Files

[edit](../helpindex/cmds/edit/index.html) .Edit a file.

[↑ Table of contents ↑](#toc)

Executing SLI programs
----------------------

[run](../helpindex/cmds/run/index.html) Execute a .sli file. [spoon](../helpindex/cmds/spoon/index.html) Execute a parallel SLI-process. [setpath](../helpindex/cmds/setpath/index.html) Set the SLI search path. [addpath](../helpindex/cmds/addpath/index.html) Append a path to SLISearchPath. [path](../helpindex/cmds/path/index.html) Return current search path as array.

[↑ Table of contents ↑](#toc)

Interfacing the hosts file-system
---------------------------------

[Directory](../helpindex/cmds/Directory/index.html) Return current working directory. [cd](../helpindex/cmds/cd/index.html) Change working directory. [SetDirectory](../helpindex/cmds/SetDirectory/index.html) Change working directory. [ls](../helpindex/cmds/ls/index.html) Print contents of current working directory. [FileNames](../helpindex/cmds/FileNames/index.html) Return contents of current working directory. [DeleteFile](../helpindex/cmds/DeleteFile/index.html) Delete a file. [mkfifo](../helpindex/cmds/mkfifo/index.html) Create a FIFO special file (named pipe). [MoveFile](../helpindex/cmds/MoveFile/index.html) Rename a file. [LocateFileNames](../helpindex/cmds/LocateFileNames/index.html) Look up complete pathnames of given file in given. [tmpnam](../helpindex/cmds/tmpnam/index.html) Return valid nonexisting file name. [MakeDirectory](../helpindex/cmds/MakeDirectory/index.html) Create a new directory. [MoveDirectory](../helpindex/cmds/MoveDirectory/index.html) Rename a directory. [RemoveDirectory](../helpindex/cmds/RemoveDirectory/index.html) Delete a directory. [CompareFiles](../helpindex/cmds/CompareFiles/index.html) Compare two files for equality.

[↑ Table of contents ↑](#toc)

Process control
---------------

### Executing UNIX commands and external programs

[system](../helpindex/cmds/system/index.html) Execute a UNIX command in a parallel process. [spawn](../helpindex/cmds/spawn/index.html) Spawn a UNIX process and redirect stdin and stdout. [shpawn](../helpindex/cmds/shpawn/index.html) Spawn a UNIX process using a shell and redirect stdin and stdout. [sysexec](../helpindex/cmds/sysexec/index.html) Transfer control to a UNIX-command. [kill](../helpindex/cmds/kill/index.html) Send a signal to another process.

### Low-level process control

[exec](../helpindex/cmds/exec/index.html) Execute an object. [ctermid](../helpindex/cmds/ctermid/index.html) Return the path to the controlling terminal of the process. [fork](../helpindex/cmds/fork/index.html) Create a child process of SLI. [spawn](../helpindex/cmds/spawn/index.html) Spawn a UNIX process and redirect stdin and stdout. [shpawn](../helpindex/cmds/shpawn/index.html) Spawn a UNIX process using a shell and redirect stdin and stdout. [spoon](../helpindex/cmds/spoon/index.html) Execute a parallel SLI-process. [kill](../helpindex/cmds/kill/index.html) Send a signal to another process. [wait](../helpindex/cmds/wait/index.html) Wait for any child process to terminate. [waitPID](../helpindex/cmds/waitPID/index.html) Wait or check for a child process to terminate. [getPGRP](../helpindex/cmds/getPGRP/index.html) Get process group ID of the current process. [getPID](../helpindex/cmds/getPID/index.html) Get ID of the current process.

### Accessing the process environment

[environment](../helpindex/cmds/environment/index.html) Return the environment of the current SLI process. [setenvironment](../helpindex/cmds/setenvironment/index.html) Set the environment of the current SLI process. [getenv](../helpindex/cmds/getenv/index.html) Evaluates if a string is an evironment variable.

### Unit Conversion

[unit\_conversion](../helpindex/cmds/unit_conversion/index.html) Conversion factors for SI units. [Hz](../helpindex/cmds/Hz/index.html) Specification in Hz (for readability). [ms](../helpindex/cmds/ms/index.html) Specification in ms (for readability). [pA](../helpindex/cmds/pA/index.html) Specification in pA (for readability). [mV](../helpindex/cmds/mV/index.html) Specification in mV (for readability). [spikes](../helpindex/cmds/spikes/index.html) Specification in spikes (for readability).

[↑ Table of contents ↑](#toc)

SLI Interpreter control
-----------------------

### Startup and initialization

[nestrc](../helpindex/cmds/nestrc/index.html) Personal interpreter initialization file. [start](../helpindex/cmds/start/index.html) Interpreter start symbol. [executive](../helpindex/cmds/executive/index.html) Start interactive interpreter session. [quit](../helpindex/cmds/quit/index.html) Leave the SLI interpreter. [welcome](../helpindex/cmds/welcome/index.html) Print SLI welcome message.

### Controlling the running interpreter

[cycles](../helpindex/cmds/cycles/index.html) Return the number of elapsed interpreter cycles. [clic](../helpindex/cmds/clic/index.html) Start measuring interpreter cycles. [cloc](../helpindex/cmds/cloc/index.html) Return elapsed interpreter cycles since clic was called. [setguard](../helpindex/cmds/setguard/index.html) Limit the number of interpreter cycles. [removeguard](../helpindex/cmds/removeguard/index.html) Removes the limit on the number of interpreter cycles. [allocations](../helpindex/cmds/allocations/index.html) Return the number of array reallocations. [setcallback](../helpindex/cmds/setcallback/index.html) Install a callback object in the interpreter cycle. [clearcallback](../helpindex/cmds/clearcallback/index.html) Clear the installed callback. [parsestdin](../helpindex/cmds/parsestdin/index.html) Read and execute tokens from standard input. [pgetrusage](../helpindex/cmds/pgetrusage/index.html) Get resource consumption information. [exithook](../helpindex/cmds/exithook/index.html) Procedure executed if the executive mode is left. [noop](../helpindex/cmds/noop/index.html) No operation function.

### Interpreter messages

[message](../helpindex/cmds/message/index.html) Display an information message. [verbosity](../helpindex/cmds/verbosity/index.html) Return the current verbosity level for interpreter messages. [setverbosity](../helpindex/cmds/setverbosity/index.html) Set verbosity level for message. [endl](../helpindex/cmds/endl/index.html) Line break.

### Measuring time

[clock](../helpindex/cmds/clock/index.html) Returns realtime. [time](../helpindex/cmds/time/index.html) Return wall clock time in s since 1.1.1970 00:00. [tic](../helpindex/cmds/tic/index.html) Start measuring usertime of a command. [toc](../helpindex/cmds/toc/index.html) Return usertime since tic was called. [usertime](../helpindex/cmds/usertime/index.html) Return clock time in ms [realtime](../helpindex/cmds/realtime/index.html) Returns realtime. [sleep](../helpindex/cmds/sleep/index.html) Pauses current process. [systemtime](../helpindex/cmds/systemtime/index.html) Returns system time for current process. [ms2hms](../helpindex/cmds/ms2hms/index.html) Convert milliseconds to an array [h min sec]. [pclocks](../helpindex/cmds/pclocks/index.html) Returns POSIX clocks for real, user, system time. [pclockspersec](../helpindex/cmds/pclockspersec/index.html) POSIX clock ticks per second. [ptimes](../helpindex/cmds/ptimes/index.html) Returns real, user, and system time.

[↑ Table of contents ↑](#toc)

Parallel computing
------------------

[Rank](../helpindex/cmds/Rank/index.html) Return the MPI rank (MPI\_Comm\_rank/index.html) of the process. [SyncProcesses](../helpindex/cmds/SyncProcesses/index.html) Synchronize all MPI processes. [MPIProcessorName](../helpindex/cmds/MPIProcessorName/index.html) Return an unique specifier for the compute node (MPI\_Get\_processor\_name). [NumProcesses](../helpindex/cmds/NumProcesses/index.html) Return the number of MPI processes (MPI\_Comm\_size).

[↑ Table of contents ↑](#toc)
