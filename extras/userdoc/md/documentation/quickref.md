# NEST/SLI Quick Reference

## Table of Contents

-   [Introduction](#introduction)
-   [The online-help system](#the-on-line-help-system)
-   [Simulation kernel](#simulation-kernel)
-   [Models and nodes](#models-and-network-nodes)
-   [Connections](#connection-nodes)
-   [Network access](#looking-at-node-networks-and-connectivity)
-   [Stack Commands](#stacks)
-   [Arithmetic functions](#arithmetic-functions)
-   [Boolean functions](#boolean-functions)
-   [Special mathematical functions](#special-mathematical-functions)
-   [Random number generation](#random-number-generation)
-   [Statistics](#statistics)
-   [Arrays, vectors, and matrices](#arrays-vectors-and-matrices)

<!-- -->

-   [Strings](#strings)
-   [Dictionaries and Namespaces](#dictionaries)
-   [Names, functions, and variables](#names-functions-and-variables)
-   [Loop structures](#loops)
-   [Control structures](#control-structures)
-   [Functions and procedures](#functions-and-procedures)
-   [Error handling and debugging](#error-handling-and-debugging])
-   [Files and streams](#files-and-streams)
-   [Executing SLI programs](#executing-sli-programs)
-   [Interfacing the host's file-system](#interfacing-the-hosts-file-system)
-   [Process control](#process-control)
-   [SLI Interpreter Control](#sli-interpreter-control)
-   [Parallel computing](#parallel-computing)


-   [Alphabetical command index](../helpindex/cmd_index.md)

## Introduction

SLI is the simulation language of the NEST simulation system. SLI is a stack
oriented language, i.e. each command expects to find its arguments on the stack.
When a SLI command is executed, it usually removes all arguments from the stack
and pushes one or more results back on the stack.

This document presents a directory of the most important SLI operators, grouped
into sections. Since this document is not automatically generated from the help
pages, it might not contain the latest additions to SLI.

## The on-line help system

### Layout of help pages

For most SLI commands, there exists help information which can be viewed either
as plain ASCII text or in HTML-format. The structure of a help entry resembles
that of a UNIX mamual page. It consists of several sections which are explained
below:

Name:

*\[namespace::\]Command*

Synopsis:

*arg1 ... argn Command -\> res1 ... resn*

*arg1*

*argn*

*argn*

*-\>*

*resn*

Parameters:

*\[result\]*

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

[help](../helpindex/cmds/help.md) Display help for a given symbol.
[helpdesk](../helpindex/cmds/helpdesk.md) Open the on-line help main page in an
HTML browser.
[helpindex](../helpindex/cmds/helpindex-cmd.md) Display a list of all documented
commands.
[page](../helpindex/cmds/page.md) The pager program to use for help output.
[apropos](../helpindex/cmds/apropos.md) Search the command index for a regular
expression.

## Simulation kernel

### Controlling the simulation

[Simulate](../helpindex/cmds/Simulate.md) Simulate n milliseconds.
[ResumeSimulation](../helpindex/cmds/ResumeSimulation.md) Resume an interrupted
simulation.
[ResetKernel](../helpindex/cmds/ResetKernel.md) Put the simulation kernel back
to its initial state.
[ResetNetwork](../helpindex/cmds/ResetNetwork.md) Reset the dynamic state of the
network.
[reset](../helpindex/cmds/reset.md) Reset dictionary stack and clear the
userdict.

### Setting and retrieving kernel parameters

Most kernel parameters are accessed via the root node's status dictionary. The
id of the root node is 0. It can be inspected and modified like all other
network nodes.

[kernel](../helpindex/cmds/kernel.md) Global properties of the
simulation kernel.
[SetStatus](../helpindex/cmds/SetStatus.md)
Modify status of an element.
[GetStatus](../helpindex/cmds/GetStatus.md)
Return the status dictionary of an element.
[ShowStatus](../helpindex/cmds/ShowStatus.md)
Show the status dictionary of a network node.

### Memory consumption

[MemoryInfo](../helpindex/cmds/MemoryInfo.md) Report the current memory
usage.
[memory\_thisjob](../helpindex/cmds/memory_thisjob.md) Report
virtual memory size for current NEST process.

## Models and network nodes

All nodes in NEST (i.e. neurons, devices, and subnetworks.md) are
derived from a common base class. Therefore they share some general properties.

[Node](../helpindex/cmds/Node.md) General properties of all nodes.
[modeldict](../helpindex/cmds/modeldict.md)
Dictionary with NEST model objects.

### Constructing nodes

[Create](../helpindex/cmds/Create.md) Create network elements in the
current subnet.
[LayoutNetwork](../helpindex/cmds/LayoutNetwork.md)
Create a nested multi-dimensional network structure in the current subnet.

### Neuron Models

[iaf\_psc\_alpha](../helpindex/cmds/iaf_psc_alpha.md)
Leaky integrate-and-fire neuron model.
[iaf\_psc\_delta](../helpindex/cmds/iaf_psc_delta.md)
Leaky integrate-and-fire neuron model.
[iaf\_psc\_exp](../helpindex/cmds/iaf_psc_exp.md)
Leaky integrate-and-fire neuron model with exponential PSCs
[iaf\_cond\_alpha](../helpindex/cmds/iaf_cond_alpha.md)
Simple conductance based leaky integrate-and-fire neuron model.
[iaf\_cond\_exp](../helpindex/cmds/iaf_cond_exp.md)
Simple conductance based leaky integrate-and-fire neuron model.
[iaf\_cond\_alpha\_mc](../helpindex/cmds/iaf_cond_alpha_mc.md)
Multi-compartment conductance-based leaky integrate-and-fire neuron model.
[iaf\_tum\_2000](../helpindex/cmds/iaf_tum_2000.md)
Leaky integrate-and-fire neuron model with exponential PSCs.
[hh\_psc\_alpha](../helpindex/cmds/hh_psc_alpha.md)
Hodgkin Huxley neuron model.
[hh\_cond\_exp\_traub](../helpindex/cmds/hh_cond_exp_traub.md)
Hodgin Huxley based model, Traub modified.

### Device Models

[Device](../helpindex/cmds/Device.md) General properties of devices.
[RecordingDevice](../helpindex/cmds/RecordingDevice.md)
Common properties of all recording devices.
[StimulatingDevice](../helpindex/cmds/StimulatingDevice.md)
General properties of stimulating devices.

#### Spike input devices

The following devices generate sequences of spikes which can be send to a
neuron. These devices act like populations of neurons and connected to their
targets like a neuron.

[pulsepacket\_generator](../helpindex/cmds/pulsepacket_generator.md)
Device to produce Gaussian spike-trains.
[poisson\_generator](../helpindex/cmds/poisson_generator.md)
Generate Poisson spike trains.
 [spike\_generator](../helpindex/cmds/spike_generator.md)
Generates spike-events from an array.

#### Analog input devices

[dc\_generator](../helpindex/cmds/dc_generator.md) Generate a DC
current.
[ac\_generator](../helpindex/cmds/ac_generator.md) Generate a
sinusoidal alternating current.
[noise\_generator](../helpindex/cmds/noise_generator.md)
Generate a Gaussian noise current.
[layerdc\_generator](../helpindex/cmds/layerdc_generator.md)
Generate a DC current for a neuron layer.
[step\_current\_generator](../helpindex/cmds/step_current_generator.md)
Provides a piecewise constant DC input current.

#### Recording devices

[spike\_detector](../helpindex/cmds/spike_detector.md) Device to record
spiking activity from one or more nodes.
[correlation\_detector](../helpindex/cmds/correlation_detector.md)
Device for evaluating cross correlation between two spike sources.
[multimeter](../helpindex/cmds/multimeter.md)
Device to record analog data from neurons.
[voltmeter](../helpindex/cmds/voltmeter.md)
Device to observe membrane potentials.
[conductancemeter](../helpindex/cmds/conductancemeter.md)
Device to observe synaptic conductances.

## Connecting nodes

[synapsedict](../helpindex/cmds/synapsedict.md) Dictionary containing
all synapse models.
[Connect](../helpindex/cmds/Connect.md) Connect two
network nodes.
[DataConnect](../helpindex/cmds/DataConnect.md)
Build connections from explicit specifications.

### Topological connections

[topology::CreateLayer](../helpindex/cmds/topology::CreateLayer.md)
Creates topological node layer.
[topology::ConnectLayers](../helpindex/cmds/topology::ConnectLayers.md)
Connect two layers.
[topology::GetElement](../helpindex/cmds/topology::GetElement.md)
Return node GID at specified layer position.
[topology::GetPosition](../helpindex/cmds/topology::GetPosition.md)
Retrieve position of input node.
[topology::GetRelativeDistance](../helpindex/cmds/topology::GetRelativeDistance.md)
Retrieve a vector of distance between two nodes.
[topology::LayerGidPositionMap](../helpindex/cmds/topology::LayerGidPositionMap.md)
Prints layer node positions to file.
[topology::PrintLayerConnections](../helpindex/cmds/topology::PrintLayerConnections.md)
Prints a list of the connections of the nodes in the layer to file.

## Looking at node, networks, and connectivity

### Navigating the network

[CurrentSubnet](../helpindex/cmds/CurrentSubnet.md) Return the GID of
the current network node.
[ChangeSubnet](../helpindex/cmds/ChangeSubnet.md)
Change the curent working subnet to a specified position.
[PrintNetwork](../helpindex/cmds/PrintNetwork.md)
Print network tree in readable form.
[NetworkDimensions](../helpindex/cmds/NetworkDimensions.md)
Returns an array with the dimensions of a structured subnet.
[GetGlobalNodes](../helpindex/cmds/GetGlobalNodes.md)
Return IDs of all nodes of a subnet.
[GetGlobalLeaves](../helpindex/cmds/GetGlobalLeaves.md)
Return IDs of all leaves of a subnet.
[GetGlobalChildren](../helpindex/cmds/GetGlobalChildren.md)
Return IDs of all immediate child nodes of a subnet.
[GetLocalNodes](../helpindex/cmds/GetLocalNodes.md)
Return IDs of all nodes of a subnet local to the MPI process executing the
command.
[GetLocalLeaves](../helpindex/cmds/GetLocalLeaves.md) Return
IDs of all leaves of a subnet local to the MPI process executing the command.
[GetLocalChildren](../helpindex/cmds/GetLocalChildren.md)
Return IDs of all immediate child nodes of a subnet local to the MPI process
executing the command.

### Investigating connectivity

[GetConnections](../helpindex/cmds/GetConnections.md) Return connections
that fulfil the given criteria.
[SetStatus](../helpindex/cmds/SetStatus.md)
Modify status of an element.
[GetStatus](../helpindex/cmds/GetStatus.md)
Return the status dictionary of an element.

### Setting and retrieving parameters of network elements

[SetStatus](../helpindex/cmds/SetStatus.md) Modify status of an element.
[GetStatus](../helpindex/cmds/GetStatus.md) Return the status dictionary
of an element.
[ShowStatus](../helpindex/cmds/ShowStatus.md) Show the
status dictionary of a network node.
[type](../helpindex/cmds/type.md)
Return the type of an element.
[ResetKernel](../helpindex/cmds/ResetKernel.md)
Put the simulation kernel back to its initial state.
[ResetNetwork](../helpindex/cmds/ResetNetwork.md)
Reset the dynamic state of the network.
[reset](../helpindex/cmds/reset.md)
Reset dictionary stack and clear the userdict.

## Stacks

### Operand stack

#### Stack contents

[stack](../helpindex/cmds/stack.md) Display stack contents.
[pstack](../helpindex/cmds/pstack.md)
Display stack contents in syntax form.
[inspect](../helpindex/cmds/inspect.md)
Inspect an object.
[typestack](../helpindex/cmds/typestack.md) Display
type information of the stack.
[operandstack](../helpindex/cmds/operandstack.md)
Store the contents of the stack in an array.
[restoreostack](../helpindex/cmds/restoreostack.md)
Restore the stack from an array.

#### Counting stack levels

[count](../helpindex/cmds/count.md) Count the number of objects on the
stack.
[counttomark](../helpindex/cmds/counttomark.md) Count number of
objects on the stack from top to marker.
[mark](../helpindex/cmds/mark.md)
Push a mark-object on the stack.

#### Copying stack elements

[dup](../helpindex/cmds/dup.md) Duplicate the object which is on top of
the stack.
[over](../helpindex/cmds/over.md) Copy stack object at
level 1.
[copy](../helpindex/cmds/copy.md) Copy the first n stack
levels.
[pick](../helpindex/cmds/pick.md) Copy object from stack level
n.
[index](../helpindex/cmds/index.md) Copy object from stack level n.

#### Removing stack elements

[pop](../helpindex/cmds/pop.md) Pop the top object off the stack.
[npop](../helpindex/cmds/npop.md)
Pop n object off the stack.
[clear](../helpindex/cmds/clear.md)
Clear the entire stack.

#### Rearranging stack elements

[exch](../helpindex/cmds/exch.md) Exchange the order of the first two
stack objects.
[roll](../helpindex/cmds/roll.md) Roll a portion n stack
levels k times.
[rolld](../helpindex/cmds/rolld.md) Roll the three top
stack elements downwards.
[rollu](../helpindex/cmds/rollu.md) Roll the
three top stack elements upwards.
[rot](../helpindex/cmds/rot.md) Rotate
entire stack contents.

### Execution-stack access

[execstack](../helpindex/cmds/execstack.md) Return the contents of the
execution stack as array.
[restoreestack](../helpindex/cmds/restoreestack.md)
Restore the execution stack from an array.

## Arithmetic functions

[abs](../helpindex/cmds/abs.md) Absolute value of a number.
[add](../helpindex/cmds/add.md)
Add two numbers or vectors.
[sub](../helpindex/cmds/sub.md) Subtract two
numbers or vectors.
[mul](../helpindex/cmds/mul.md) Multiply two numbers
or vectors (point-wise).
[div](../helpindex/cmds/div.md) Divide two
numbers or vectors (point-wise).
[inv](../helpindex/cmds/inv.md) Compute
1/x.
[mod](../helpindex/cmds/mod.md) Compute the modulo of two integer
numbers.
[neg](../helpindex/cmds/neg.md) Reverse sign of a number.
[ceil](../helpindex/cmds/ceil.md) Return nearest integer larger than the
argument.
[round](../helpindex/cmds/round.md) Round double to the
nearest integer.
[trunc](../helpindex/cmds/trunc.md) Truncate decimals
of a double.
[floor](../helpindex/cmds/floor.md) Return nearest integer
smaller than the argument.
[pow](../helpindex/cmds/pow.md) Raise a
number to a power.
[sqr](../helpindex/cmds/sqr.md) Compute the square of
a number.
[sqrt](../helpindex/cmds/sqrt.md) Compute the square root of a
non-negative number.
[exp](../helpindex/cmds/exp.md) Calculate the
exponential of double number.
[ln](../helpindex/cmds/ln.md) Calculate
natural logarithm of double number.
[log](../helpindex/cmds/log.md)
Calculate decadic logarithm of double number.
[frexp](../helpindex/cmds/frexp.md)
Decomposes its argument into an exponent of 2 and a factor.
[modf](../helpindex/cmds/modf.md)
Decomposes its argument into fractional and integral part.
[cos](../helpindex/cmds/cos.md)
Calculate the cosine of double number.
[sin](../helpindex/cmds/sin.md)
Calculate the sine of double number.
[max](../helpindex/cmds/max.md)
Return the greater of two values.
[min](../helpindex/cmds/min.md) Return
the smaller of two values.

## Boolean functions

### Comparison functions

[eq](../helpindex/cmds/eq.md) Test two objects for equality.
[gt](../helpindex/cmds/gt.md)
Test if one object is greater than another object.
[geq](../helpindex/cmds/geq.md)
Test if one object is greater or equal than another object.
[lt](../helpindex/cmds/lt.md)
Test if one object is less than another object.
[leq](../helpindex/cmds/leq.md)
Test if one object is less or equal than another object.
[neq](../helpindex/cmds/neq.md)
Test two objects for inequality.
[Min](../helpindex/cmds/Min.md) Returns
the smallest element of an array.
[Max](../helpindex/cmds/Max.md)
Returns the largest element of an array.

### Boolean operators

[and](../helpindex/cmds/and.md) Logical and operator.
[or](../helpindex/cmds/or.md)
Logical or operator.
[not](../helpindex/cmds/not.md) Logical not
operator.
[xor](../helpindex/cmds/xor.md) Logical xor operator.

## Special mathematical functions

[Erf](../helpindex/cmds/Erf.md) Error function.
[Erfc](../helpindex/cmds/Erfc.md)
Complementary error function.
[Gammainc](../helpindex/cmds/Gammainc.md)
Incomplete Gamma function.
[GaussDiskConv](../helpindex/cmds/GaussDiskConv.md)
Convolution of an excentric Gaussian with a disk.
[CyclicValue](../helpindex/cmds/CyclicValue.md)
Project a cyclic value onto it's norm interval (e.g. angle on [0,360)).
[FractionalPart](../helpindex/cmds/FractionalPart.md)
Return fractional part of the argument.
[IntegerPart](../helpindex/cmds/IntegerPart.md)
Return integer part of the argument.
[UnitStep](../helpindex/cmds/UnitStep.md)
The unit step function (aka Heavyside function).
[LambertW](../helpindex/cmds/LambertW.md)
Simple iteration implementing the Lambert-W function.

### Mathematical constants

[E](../helpindex/cmds/E.md) Euler constant.
[Pi](../helpindex/cmds/Pi.md)
Pi constant.

## Random number generation

[seed](../helpindex/cmds/seed.md) Set the seed of a random number
generator.
[irand](../helpindex/cmds/irand.md) Generate a random integer
number.
[drand](../helpindex/cmds/drand.md) Generate a random double
number.
[rngdict](../helpindex/cmds/rngdict.md) Dictionary of random
generator types.
[rdevdict](../helpindex/cmds/rdevdict.md) Dictionary of
random deviate types.
[CreateRNG](../helpindex/cmds/CreateRNG.md) Create
a random number generator.
[CreateRDV](../helpindex/cmds/CreateRDV.md)
Create a random deviate.
[Random](../helpindex/cmds/Random.md) Returns
a random number.
[RandomArray](../helpindex/cmds/RandomArray.md) Returns
array with random numbers.
[RandomSubset](../helpindex/cmds/RandomSubset.md)
Random subset of an arry without repetitions.
[GetStatus](../helpindex/cmds/GetStatus.md)
Return the property dictionary of a random deviate generator.
[SetStatus](../helpindex/cmds/SetStatus.md)
Modify the properties of a random deviate generator.

### Random deviate generator types

[binomial](../helpindex/cmds/rdevdict::binomial.md) Binomial random
deviate generator.
[exponential](../helpindex/cmds/rdevdict::exponential.md)
Exponential random deviate generator.
[gamma](../helpindex/cmds/rdevdict::gamma.md)
Gamma random deviate generator.
[normal](../helpindex/cmds/rdevdict::normal.md)
Normal random deviate generator.
[normal\_clipped](../helpindex/cmds/rdevdict::normal_clipped.md)
Clipped normal random deviate generator.
[normal\_clipped\_left](../helpindex/cmds/rdevdict::normal_clipped_left.md)
Left clipped normal random deviate generator.
[normal\_clipped\_right](../helpindex/cmds/rdevdict::normal_clipped_right.md)
Right clipped normal random deviate generator.
[poisson](../helpindex/cmds/rdevdict::poisson.md)
Poisson random deviate generator.
[uniform\_int](../helpindex/cmds/rdevdict::uniformint.md)
Uniform integer random deviate generator.

## Statistics

[Min](../helpindex/cmds/Min.md) Returns the smallest element of an
array.
[Max](../helpindex/cmds/Max.md) Returns the largest element of an
array.
[Total](../helpindex/cmds/Total.md) Returns the sum of the
elements of an array.
[Mean](../helpindex/cmds/Mean.md) Returns the mean
of the elements of an array.
[Variance](../helpindex/cmds/Variance.md)
Returns the unbiased variance of the elements of an array.
[StandardDeviation](../helpindex/cmds/StandardDeviation.md)
Returns the standard deviation of the element of an array.

## Arrays, vectors, and matrices

### Construction

[array](../helpindex/cmds/array.md) Construct array with n zeros (PS).
[arraystore](../helpindex/cmds/arraystore.md)
Pops the first n elements of the stack into an array.
[LayoutArray](../helpindex/cmds/LayoutArray.md)
Create a multi-dimensional array.
[Range](../helpindex/cmds/Range.md)
Generate array with range of numbers.
[Table](../helpindex/cmds/Table.md)
Generate an array according to a given function.
[ReadList](../helpindex/cmds/ReadList.md)
Read a list of specified format from a stream.
[GaborPatch](../helpindex/cmds/arr::GaborPatch.md)
Create a two-dimensional array filled with values from the Gabor function.
[GaussPatch](../helpindex/cmds/arr::GaussPatch.md)
Create a two-dimensional array filled with values from the Gauss function.

### Conversions

[cva](../helpindex/cmds/cva.md) Convert dictionary/trie to array.
[cst](../helpindex/cmds/cst.md)
Convert string to array of tokens.
[cvlit](../helpindex/cmds/cvlit.md)
Convert name/string/procedure to literal/array.
[cvx](../helpindex/cmds/cvx.md)
Convert array/string to procedure.
[cv1d](../helpindex/cmds/cv1d.md)
Convert 2-dimensional coordinates to 1-dim index.
[cv2d](../helpindex/cmds/cv2d.md)
Convert 1-dimensional index to 2-dim coordinate.
[Export](../helpindex/cmds/Export.md)
Save in a foreign file format.
[MathematicaToSliIndex](../helpindex/cmds/MathematicaToSliIndex.md)
Convert Mathematica-like indices to SLI indices.
[SliToMathematicaIndex](../helpindex/cmds/SliToMathematicaIndex.md)
Convert SLI indices to Mathematica-like indices.

### Insertion

[put](../helpindex/cmds/put.md) Put indexed object into container.
[insert](../helpindex/cmds/insert.md)
Insert an object in a container at a specific position.
[insertelement](../helpindex/cmds/insertelement.md)
Insert an element to a container at a specific position.
[join](../helpindex/cmds/join.md)
Join two containers of the same type.
[JoinTo](../helpindex/cmds/JoinTo.md)
Join with container referenced by l-value.
[append](../helpindex/cmds/append.md)
Append object to container.
[AppendTo](../helpindex/cmds/AppendTo.md) Append to
container referenced by l-value.
[prepend](../helpindex/cmds/prepend.md) Attach
an object to the front of a container.

### Inspecting an array or matrix

[empty](../helpindex/cmds/empty.md) Tests if a string or array is empty.
[length](../helpindex/cmds/length.md)
Counts elements of an object.
[size](../helpindex/cmds/size.md) Returns the size
of an array/string.
[GetMin](../helpindex/cmds/GetMin.md) Get minimal element.
[GetMax](../helpindex/cmds/GetMax.md) Get maximal element.
[ArrayQ](../helpindex/cmds/ArrayQ.md)
Returns true if top object is an array.
[MatrixQ](../helpindex/cmds/MatrixQ.md)
Test if a SLI array is a (hyper-rectengular.md) matrix.
[Dimensions](../helpindex/cmds/Dimensions.md)
Determine dimenstions of a (hyper-rectangular.md) SLI array.
[TensorRank](../helpindex/cmds/TensorRank.md)
Estimate the rank of a tensor.

### Retrieving elements from an array

[get](../helpindex/cmds/get.md) Retrieve indexed Object from a container.
[Part](../helpindex/cmds/Part.md)
Returns parts of an array.
[Take](../helpindex/cmds/Take.md) Extract sequences
from an array.
[getinterval](../helpindex/cmds/getinterval.md) Return a
subsequence of a container.
[erase](../helpindex/cmds/erase.md) Deletes a
subsequece of a container.
[First](../helpindex/cmds/First.md) Return the first
element of an array.
[Last](../helpindex/cmds/Last.md) Return the last element
of an array.
[Rest](../helpindex/cmds/Rest.md) Remove the first element of an
array and return the rest.
[Select](../helpindex/cmds/Select.md) Reduces an
array to elements which fulfill a criterion.
[Sort](../helpindex/cmds/Sort.md)
Sorts a homogeneous array of doubles.
[arrayload](../helpindex/cmds/arrayload.md)
Pushes array elements followed by number of elements.
[area](../helpindex/cmds/area.md)
Return array of indices defining a 2d subarea of a 2d array.
[area2](../helpindex/cmds/area.md)
Return array of indices defining a 2d subarea of a 2d array.

#### Removing and replacing elements

[putinterval](../helpindex/cmds/putinterval.md) Replace sections of an
array/string (PS).
[erase](../helpindex/cmds/erase.md) Deletes a subsequece of a
container.
[replace](../helpindex/cmds/replace.md) Replace a section of a
container by a new sequence.
[ReplaceOccurrences](../helpindex/cmds/ReplaceOccurrences.md)
Replace the occurences of a key in a container.
[breakup](../helpindex/cmds/breakup.md)
Break a string or an array at given Substrings or SubArrays.
[trim](../helpindex/cmds/trim.md)
Delete leading/trailing elements in a container.

#### Operations on lists and arrays

[Partition](../helpindex/cmds/Partition.md) Partition list into n element
pieces.
[Flatten](../helpindex/cmds/Flatten.md) Flatten out a nested list.
[Reform](../helpindex/cmds/arr::Reform.md) Reform the dimensions of a
hyperrectengular array.
[Select](../helpindex/cmds/Select.md) Reduces an array
to elements which fulfill a criterion.
[Split](../helpindex/cmds/Split.md)
Splits array into subarrays of sequences of identical elements.
[MergeLists](../helpindex/cmds/MergeLists.md)
Merges sorted lists.
[MemberQ](../helpindex/cmds/MemberQ.md) Checks if array
contains a specific element.
[HasDifferentMemberQ](../helpindex/cmds/HasDifferentMemberQ.md)
Checks if array contains an element different from given value.

### Functional operations on lists and arrays

[Map](../helpindex/cmds/Map.md) Apply a procedure to each element of a list or
string.
[MapAt](../helpindex/cmds/MapAt.md) Apply a procedure to some elements
of a list or string.
[MapIndexed](../helpindex/cmds/MapIndexed.md) Apply a
function to each element of a list/string.
[MapThread](../helpindex/cmds/MapThread.md)
Apply a procedure to to corresponding elements of n arrays.
[FixedPoint](../helpindex/cmds/FixedPoint.md)
Apply a procedure repeatedly until the result is an invariant.
[NestList](../helpindex/cmds/NestList.md)
Gives a list of the results of applying f to x 0 through n times.
[Nest](../helpindex/cmds/Nest.md)
Apply a function n times.
[FoldList](../helpindex/cmds/FoldList.md) Gives a list
of the results of repeatedly applying a function with two parameters.
[Fold](../helpindex/cmds/Fold.md)
Result of repeatedly applying a function with two arguments.
[ScanThread](../helpindex/cmds/ScanThread.md)
Apply a procedure to corresponding elements of n arrays, not returing results.
[forall](../helpindex/cmds/forall.md)
Call a procedure for each element of a list/string.
[forallindexed](../helpindex/cmds/forallindexed.md)
Call a procedure for each element of a list/string.
[EdgeClip](../helpindex/cmds/arr::EdgeClip.md)
Clip 2-d array indices at array edges.
[EdgeTruncate](../helpindex/cmds/arr::EdgeTruncate.md)
Truncate 2-d array indices at array edges.
[EdgeWrap](../helpindex/cmds/arr::EdgeWrap.md)
Wrap 2-d array indices around edges (toriodal).
[IndexWrap](../helpindex/cmds/arr::IndexWrap.md)
Project a cyclic index value onto interval [0,N).

### Arrays as vectors and matrices

#### Dimensions and rank

[MatrixQ](../helpindex/cmds/MatrixQ.md) Test whether a nested array is a matrix.
[Dimensions](../helpindex/cmds/Dimensions.md) Determine dimensions of an array.
[TensorRank](../helpindex/cmds/TensorRank.md) Determine the level to which an
array is a full vector.

#### Operations on vectors and matrices

[add](../helpindex/cmds/add.md) Add two numbers or vectors.
[sub](../helpindex/cmds/sub.md)
Subtract two numbers or vectors.
[mul](../helpindex/cmds/mul.md) Multiply two
numbers or vectors (point-wise).
[div](../helpindex/cmds/div.md) Divide two
numbers or vectors (point-wise).
[Dot](../helpindex/cmds/Dot.md) Product of
vectors, matrices, and tensors.
[Plus](../helpindex/cmds/Plus.md) Sum of all
vector components.
[Times](../helpindex/cmds/Times.md) Product of all vector
components.
[Transpose](../helpindex/cmds/Transpose.md) Transposes the first two
levels of its argument.
[reverse](../helpindex/cmds/reverse.md) Reverse a string
or array.
[rotate](../helpindex/cmds/rotate.md) Rotate an array.
[OuterProduct](../helpindex/cmds/OuterProduct.md)
Outer product.

### Memory management

[capacity](../helpindex/cmds/capacity.md) Returns the capacity of an array.
[reserve](../helpindex/cmds/reserve.md)
Bring an array to a certain capacity.
[resize](../helpindex/cmds/resize.md)
Change the internal size of an array.
[shrink](../helpindex/cmds/shrink.md)
Reduce the capacity of an array to its minimum.

## Strings

### Construction

[=](../helpindex/cmds/=.md) Display top operand stack object.
[==](../helpindex/cmds/==.md)
Display top operand stack object.
[==only](../helpindex/cmds/==only.md)
Display top operand stack object without linefeed.
[=only](../helpindex/cmds/=only.md)
Display top operand stack object without linefeed.
[getline](../helpindex/cmds/getline.md)
Read an entire line from an input stream.
[gets](../helpindex/cmds/gets.md)
Read white space terminated string from stream.
[Read](../helpindex/cmds/Read.md)
Read an object of a certain type from a stream.
[ReadWord](../helpindex/cmds/ReadWord.md)
Read white space terminated string from stream.

### Conversion

[cvs](../helpindex/cmds/cvs.md) Convert object to string.
[cst](../helpindex/cmds/cst.md)
Convert string to array of tokens.
[cvd\_s](../helpindex/cmds/cvd_s.md) Convert
string to double.
[token\_s](../helpindex/cmds/token_s.md) Read a token from a
string.

### Insertion

[length](../helpindex/cmds/length.md) Counts elements of an object.
[put](../helpindex/cmds/put.md)
Put indexed object into container.
[putinterval](../helpindex/cmds/putinterval.md)
Replace sections of an array/string (PS).
[insert](../helpindex/cmds/insert.md)
Insert an object in a container at a specific position.
[insertelement](../helpindex/cmds/insertelement.md)
Insert an element to a container at a specific position.
[prepend](../helpindex/cmds/prepend.md)
Attach an object to the front of a container.
[append.](../helpindex/cmds/append.md)
Append object to container.
[join](../helpindex/cmds/join.md) Join two
containers of the same type.

### Retrieving characters and substrings

[empty](../helpindex/cmds/empty.md) Tests if a string or array is empty.
[get](../helpindex/cmds/get.md)
Lookup indexed Object of a container.
[getinterval](../helpindex/cmds/getinterval.md)
Return a subsequence of a container.
[erase](../helpindex/cmds/erase.md) Deletes
a subsequece of a container.
[First](../helpindex/cmds/First.md) Return the
first element of an array/string.
[Last](../helpindex/cmds/Last.md) Return the
last element of an array/string.
[Rest](../helpindex/cmds/Rest.md) Remove the
first element of an array/string and return the rest.
[search](../helpindex/cmds/search.md)
Search for a sequence in an array or string.
[searchif](../helpindex/cmds/searchif.md)
Check wether a substring is contained within a string.

### Removing and replacing

[erase](../helpindex/cmds/erase.md) Deletes a subsequece of a container.
[replace](../helpindex/cmds/replace.md)
Replace a section of a container by a new sequence.
[ReplaceOccurrences](../helpindex/cmds/ReplaceOccurrences.md)
replace the occurences of a key in a container.
[breakup](../helpindex/cmds/breakup.md)
Break a string or an array at given sub-strings or sub-arrays.
[trim](../helpindex/cmds/trim.md)
Delete leading/trailing elements in a container.

### Operations on strings

[empty](../helpindex/cmds/empty.md) Tests if a string or array is empty.
[length](../helpindex/cmds/length.md)
Counts elements of an object.
[size](../helpindex/cmds/size.md) Returns the size
of an array/string.
[forall](../helpindex/cmds/forall.md) Call a procedure for
each element of a list/string.
[forallindexed](../helpindex/cmds/forallindexed.md)
Call a procedure for each element of a list/string.
[Map](../helpindex/cmds/Map.md)
Apply a procedure to each element of a list or string.
[MapIndexed](../helpindex/cmds/MapIndexed.md)
Apply a function to each element of a list/string.
[reverse](../helpindex/cmds/reverse.md)
Reverse a string or array.
[ToUppercase](../helpindex/cmds/ToUppercase.md)
Convert a string to upper case.
[ToLowercase](../helpindex/cmds/ToLowercase.md)
Convert a string to lower case.

### Regular expressions

[regcomp](../helpindex/cmds/regcomp.md) Create a regular expression.
[regex\_find](../helpindex/cmds/regex_find.md)
Check if a regex is included in a string or stream.
[regex\_find\_r](../helpindex/cmds/regex_find_r.md)
Check if a regex is included in a string.
[regex\_find\_rf](../helpindex/cmds/regex_find_rf.md)
Check if a regex is included in a stream.
[regex\_find\_s](../helpindex/cmds/regex_find_s.md)
Check if a regex is included in a string.
[regex\_find\_sf](../helpindex/cmds/regex_find_sf.md)
Check if a regex is included in a stream.
[regex\_replace](../helpindex/cmds/regex_replace.md)
Replace all occurences of a regex.
[regexec](../helpindex/cmds/regexec.md)
Compare string and regular expression.

## Dictionaries

### Construction

[\<\<\>\>](../helpindex/cmds/%3C%3C%3E%3E.md) construct a dictionary.
[dict](../helpindex/cmds/dict.md)
Create new, empty dictionary.
[clonedict](../helpindex/cmds/clonedict.md) Create
a copy of a dictionary.
[SaveDictionary](../helpindex/cmds/SaveDictionary.md)
Save contents of a dictionary to a file.
[RestoreDictionary](../helpindex/cmds/RestoreDictionary.md)
Read a dictionary definition from a file.
[MergeDictionary](../helpindex/cmds/MergeDictionary.md)
Merge all definitions of a dictionary with the current dicitonary.

### Conversion

[cva](../helpindex/cmds/cva.md) Convert dictionary/trie to array.

### Insertion and lookup

[using](../helpindex/cmds/using.md) Add a namespace (or dictionary.md) to the
local scope, keeping the current dictionary.
[endusing](../helpindex/cmds/endusing.md)
Close the scope of a 'using' context.
[get](../helpindex/cmds/get.md) Lookup
indexed Object of a container.
[call](../helpindex/cmds/call.md) Execute object
from a dictionary (or namespace, see below).
[put](../helpindex/cmds/put.md) Put
indexed object into container.
[put\_d](../helpindex/cmds/put_d.md) Add an entry
to a dictionary.
[known](../helpindex/cmds/known.md) Check whether a name is
defined in a dictionary.
[info](../helpindex/cmds/info.md) Display the contents
of a dictionary.
[info\_ds](../helpindex/cmds/info_ds.md) Print contents of all
dictionaries on the dicitonary stack to stream.
[topinfo\_d](../helpindex/cmds/topinfo_d.md)
Print contents of top dictionary to stream.
[length](../helpindex/cmds/length.md)
Counts elements of a container object.
[SubsetQ](../helpindex/cmds/SubsetQ.md)
Test if one dictionary is a subset of another.

### Key removal

[undef](../helpindex/cmds/undef.md) Remove a key from a dictionary.
[cleardict](../helpindex/cmds/cleardict.md)
Clears the contents of a dictionary.

### Special dictionaries

[errordict](../helpindex/cmds/errordict.md) Pushes error dictionary on operand
stack.
[modeldict](../helpindex/cmds/modeldict.md) Dictionary with neural model
objects.
[synapsedict](../helpindex/cmds/synapsedict.md) Dictionary containing
all synapse models.
[statusdict](../helpindex/cmds/statusdict.md) Dictionary
with platform dependent status.
[signaldict](../helpindex/cmds/signaldict.md)
Dictionary containing the machine-dependent signal codes.
[libdict](../helpindex/cmds/libdict.md)
Dictionary of provided libraries and their components.
[elementstates](../helpindex/cmds/elementstates.md)
Dictionary with symbolic element state tag.
[ReadModes](../helpindex/cmds/ReadModes.md)
Dictionary with type specifiers for read functions.
[OptionsDictionary](../helpindex/cmds/OptionsDictionary.md)
Dictionary for global options.

### Dictionary Stack

[dictstack](../helpindex/cmds/dictstack.md) Return current dictionary stack as
array.
[begin](../helpindex/cmds/begin.md) Open a dictionary.
[end](../helpindex/cmds/end.md)
Closes the current dictionary.
[currentdict](../helpindex/cmds/currentdict.md)
Return topmost dictionary of the dictionary stack.
[lookup](../helpindex/cmds/lookup.md)
Search for a key in each dictionay on the dictionary stack.
[who](../helpindex/cmds/who.md)
List contents of the top-level dicitonary.
[whos](../helpindex/cmds/whos.md)
List contents of all dictionaries on the dicitonary stack.
[countdictstack](../helpindex/cmds/countdictstack.md)
Return number of dictionaries on the dictionary stack.
[cleardictstack](../helpindex/cmds/cleardictstack.md)
Pop all non-standard dictionaries of the stack.

### Namespaces

Dictionaries can be used to limit names to a certain scope. This is useful for
logical grouping, and to prevent name conflicts, very much like the use of
namespaces in C++. Furthermore, it keeps the system dictionary from being
littered.

When refering to a variable or routine of limited scope, its scope shall be
indicated by the notation namespace::name (e.g. arr::Reform). If no namespace
is specified, systemdict:: is implicitely assumed, meaning unlimited scope.

*Note:* Please note that the notation namespace::name is not yet supported in
program code. Use the command call instead.

[namespace](../helpindex/cmds/namespace.md) Open or create a namespace
dictionary.
[call](../helpindex/cmds/call.md) Execute object from a namespace (or
dictionary, see above).
[::](../helpindex/cmds/::.md.md) Execute a symbol from a nested namespace.

## Names, functions, and variabes

### Defining variables and functions

[def](../helpindex/cmds/def.md) Define a variable or function.
[Set](../helpindex/cmds/Set.md)
Same as def with reversed arguments.
[undef](../helpindex/cmds/undef.md)
Remove a key from a dictionary.
[SLIFunctionWrapper](../helpindex/cmds/SLIFunctionWrapper.md)
Define a SLI function with lots of comfort.

### Show defined variables and functions

[who](../helpindex/cmds/who.md) List contents of the top-level dicitonary.
[whos](../helpindex/cmds/whos.md)
List contents of all dictionaries on the dicitonary stack.
[lookup](../helpindex/cmds/lookup.md)
Search for a key in each dictionay on the dictionary stack.
[known](../helpindex/cmds/known.md)
Check whether a name is defined in a dictionary.

### Names and scoping

[begin](../helpindex/cmds/begin.md) Open a dictionary.
[end](../helpindex/cmds/end.md)
Closes the current dictionary.
[lookup](../helpindex/cmds/lookup.md) Search for
a key in each dictionay on the dictionary stack.
[load](../helpindex/cmds/load.md)
Search for a key in each dictionay on the dictionary stack.

## Loops

### Infinite loops

[loop](../helpindex/cmds/loop.md) Repeatedly execute a procedure.
[exit](../helpindex/cmds/exit.md)
Exit a loop construct.

### Loops with counters

[for](../helpindex/cmds/for.md) Execute a procedure for a sequence of numbers.
[repeat](../helpindex/cmds/repeat.md) Execute a procedure n times.
[FindRoot](../helpindex/cmds/FindRoot.md)
Numerically find a root in an interval.
[exit](../helpindex/cmds/exit.md) Exit a
loop construct.

### Loops over containers

[forall](../helpindex/cmds/forall.md) Call a procedure for each element of a
list/string.
[forallindexed](../helpindex/cmds/forallindexed.md) Call a
procedure for each element of a list/string.
[Map](../helpindex/cmds/Map.md)
Apply a procedure to each element of a list or string.
[MapIndexed](../helpindex/cmds/MapIndexed.md)
Apply a function to each element of a list/string.
[MapThread](../helpindex/cmds/MapThread.md)
Apply a procedure to corresponding elements of n arrays, returning the result.
[ScanThread](../helpindex/cmds/ScanThread.md) Apply a procedure to corresponding
elements of n arrays, not returing results.
[NestList](../helpindex/cmds/NestList.md)
Gives a list of the results of applying f to x 0 through n times.
[FoldList](../helpindex/cmds/FoldList.md)
Repeatedly apply a function with two parameters.
[exit](../helpindex/cmds/exit.md)
Exit a loop construct.

## Control structures

[if](../helpindex/cmds/if.md) Conditionaly execute a procedure.
[ifelse](../helpindex/cmds/ifelse.md)
Conditionaly execute a procedure.
[case](../helpindex/cmds/case.md) Like if, but
test a series of conditions.
[switch](../helpindex/cmds/switch.md) Finish a
case ... switch structure.
[switchdefault](../helpindex/cmds/switchdefault.md)
Finish a case ... switchdefault structure.
[stop](../helpindex/cmds/stop.md)
Raise a stop signal.
[stopped](../helpindex/cmds/stopped.md) Returns true if
execution was stopped by stop.

## Functions and procedures

### Object oriented techniques

[call](../helpindex/cmds/call.md) Execute object from a dictionary.

### Type checking

#### Accessing type information

[type](../helpindex/cmds/type.md) Return the type of an object.
[typeinfo](../helpindex/cmds/typeinfo.md)
Return the type of an object.
[LiteralQ](../helpindex/cmds/LiteralQ.md) Returns
true if top object is a literal.
[NumberQ](../helpindex/cmds/NumberQ.md)
Returns true if top object is a number (int or double).
[StringQ](../helpindex/cmds/StringQ.md)
Returns true if top object is a string.

#### Type tries

A type-trie is a lookup structure which can be used to implement automatic type
checking as well as operator overloading.

[def](../helpindex/cmds/def.md) Define a variable or function.
[SLIFunctionWrapper](../helpindex/cmds/SLIFunctionWrapper.md) Define a SLI
function with lots of comfort.
[trie](../helpindex/cmds/trie.md) Create a new
type-trie object.
[addtotrie](../helpindex/cmds/addtotrie.md) Add a function
variant to a trie-object.
[cva](../helpindex/cmds/cva.md) Convert
dictionary/trie to array.
[cva\_t](../helpindex/cmds/cva_t.md) Converts a type
trie to an equivalent array.
[cvt\_a](../helpindex/cmds/cvt_a.md) Converts an
array to the equivalent type trie.

### Type Conversions

[cva](../helpindex/cmds/cva.md) Convert dictionary/trie to array.
[cst](../helpindex/cmds/cst.md)
Convert string to array of tokens.
[cvi](../helpindex/cmds/cvi.md) Convert
double/string to integer.
[cvd](../helpindex/cmds/cvd.md) Convert integer/string
to double.
[cvlit](../helpindex/cmds/cvlit.md) Convert name/string/procedure to
literal/array.
[cvn](../helpindex/cmds/cvn.md) Convert literal/string to name.
[cvx](../helpindex/cmds/cvx.md) Convert array/string to procedure.

### Options

[Options](../helpindex/cmds/Options.md) Define a new set of options for a given
name.
[GetOption](../helpindex/cmds/GetOption.md) Get the value of a procedure
option.
[GetOptions](../helpindex/cmds/GetOptions.md) Get all options for a
given name.
[SetOptions](../helpindex/cmds/SetOptions.md) Set options for a
given name.
[SaveOptions](../helpindex/cmds/SaveOptions.md) Temporarily save
options of a command [ResetOptions](../helpindex/cmds/ResetOptions.md) Reset all
options of a command to their default values.
[RestoreOptions](../helpindex/cmds/RestoreOptions.md)
Restore the temporarily saved options of a command.
[ShowOptions](../helpindex/cmds/ShowOptions.md)
Display all options for a given name.
[OptionsDictionary](../helpindex/cmds/OptionsDictionary.md)
Dictionary for global options.

### Optimization

[bind](../helpindex/cmds/bind.md) Recursively replaces executable operator names
by their values.

## Error handling and debugging

If an error occurs, execution is usually interrupted and a diagnostic message is
printed. In most cases, the stack is restored to the state where it was
immediately before the command that raised the error was called.

If the error was raised from within a procedure, the stack might not be restored
to the state before the procedure was called.

[handleerror](../helpindex/cmds/handleerror.md) Default error handler.
[stop](../helpindex/cmds/stop.md)
Raise a stop signal.
[stopped](../helpindex/cmds/stopped.md) Returns true if
execution was stopped by stop.
[raiseerror](../helpindex/cmds/raiseerror.md)
Raise an error to the system.
[raiseagain](../helpindex/cmds/raiseagain.md)
Re-raise the last error.
[resume](../helpindex/cmds/resume.md) Resume
interrupted SLI program after a system signal.
[setguard](../helpindex/cmds/setguard.md)
Limit the number of interpreter cycles.
[break](../helpindex/cmds/break.md)
Interrupt the execution of a procedure for inspection.
[continue](../helpindex/cmds/continue.md)
Continue an interrupted procedure.
[assert](../helpindex/cmds/assert.md) Assert
that procedure returns true.
[currentname](../helpindex/cmds/currentname.md)
Returns the most recently resolved name.
[debug.sli](../helpindex/cmds/debug.sli.md)
Debugging support for SLI.

## Files and Streams

### Standard input and output

[cin](../helpindex/cmds/cin.md) Standard input stream.
[cout](../helpindex/cmds/cout.md)
Standard output stream.
[cerr](../helpindex/cmds/cerr.md) Standard error output
stream.
[setprecision](../helpindex/cmds/setprecision.md) Set precision for
decimal place of a stream.
[inspect](../helpindex/cmds/inspect.md) Inspect an
object.
[=](../helpindex/cmds/=.md) Display top operand stack object.
[==](../helpindex/cmds/==.md)
Display top operand stack object.
[==only](../helpindex/cmds/==only.md) Display
top operand stack object without linefeed.
[=only](../helpindex/cmds/=only.md)
Display top operand stack object without linefeed.
[print](../helpindex/cmds/print.md)
Print object to a stream.
[pprint](../helpindex/cmds/pprint.md)
Pretty print object to a stream.

### Opening and Closing

[file](../helpindex/cmds/file.md) Opens file for reading or writing.
[ifstream](../helpindex/cmds/ifstream.md)
Open file stream for reading.
[xifstream](../helpindex/cmds/xifstream.md) Create
an executable input-stream.
[searchfile](../helpindex/cmds/searchfile.md) Tries
to open a file for reading using the search-path.
[searchifstream](../helpindex/cmds/searchifstream.md)
Searches SLI's search path for a file.
[ofstream](../helpindex/cmds/ofstream.md)
Open a file stream for writing.
[ofsopen](../helpindex/cmds/ofsopen.md) Open an
existing file for appending or writing.
[osstream](../helpindex/cmds/osstream.md)
Create a string-stream object.
[pipe](../helpindex/cmds/pipe.md) Open up a pipe.
[mkfifo](../helpindex/cmds/mkfifo.md) Create a FIFO special file (named pipe).
[close](../helpindex/cmds/close.md) Close a stream.
[tmpnam](../helpindex/cmds/tmpnam.md)
Return valid nonexisting file name.
[dup2](../helpindex/cmds/dup2.md)
Duplicate a filestream's file descriptor onto another's.
[isatty](../helpindex/cmds/isatty.md)
Determine if a stream is connected to a terminal.

### Reading from streams

#### Checking for data

[setNONBLOCK](../helpindex/cmds/setNONBLOCK.md) Switch between blocking and
non-blocking I/O.
[in\_avail](../helpindex/cmds/in_avail.md) Check for available
data in an input stream's buffer.
[available](../helpindex/cmds/available.md)
Check if data is available from an istream.
[ignore](../helpindex/cmds/ignore.md)
Ignore any waiting data on an istream.

#### Unformatted string input

[getc](../helpindex/cmds/getc.md) Read single character from input stream.
[gets](../helpindex/cmds/gets.md) Read white space terminated string from
stream.
[getline](../helpindex/cmds/getline.md) Read a newline terminated string
from an input stream.
[readline](../helpindex/cmds/readline.md) Read and edit a
line from standard input.
[GNUreadline](../helpindex/cmds/GNUreadline.md) Read
and edit a line from standard input.
[GNUaddhistory](../helpindex/cmds/GNUaddhistory.md)
Add a string to the readline-history.
[oldgetline](../helpindex/cmds/oldgetline.md)
"Old", ignorant version of getline.

#### Formatted input

[str](../helpindex/cmds/str.md) Retrieve a string from a string-stream.
[token](../helpindex/cmds/token.md)
Read a token from a stream or string.
[token\_is](../helpindex/cmds/token_is.md)
Read a token from an input stream.
[token\_s](../helpindex/cmds/token_s.md) Read
a token from a string.
[Read](../helpindex/cmds/Read.md) Read an object of a
certain type from a stream.
[ReadDouble](../helpindex/cmds/ReadDouble.md) Read a
double from a stream.
[ReadInt](../helpindex/cmds/ReadInt.md) Read an integer
from a stream.
[ReadList](../helpindex/cmds/ReadList.md) Read a list of
specified format from a stream.
[ReadWord](../helpindex/cmds/ReadWord.md) Read
white space terminated string from stream.
[ReadModes](../helpindex/cmds/ReadModes.md)
Dictionary with type specifiers for read functions.

#### Formatted Output

[Export](../helpindex/cmds/Export.md) Save in a foreign file format.

### Writing to streams

[print](../helpindex/cmds/print.md) Print object to a stream.
[pprint](../helpindex/cmds/pprint.md) Pretty print object to a stream.
[endl](../helpindex/cmds/endl.md) Line break.
[flush](../helpindex/cmds/flush.md)
Force the buffer of a stream to be flushed.
[setprecision](../helpindex/cmds/setprecision.md)
Set precision for decimal place of a stream.

### Handling flags of streams

[iclear](../helpindex/cmds/iclear.md) Clear the state-flags of input stream.
[oclear](../helpindex/cmds/oclear.md) Clear the state-flags of output stream.
[ifail](../helpindex/cmds/ifail.md) Check the "fail"-flag of an input stream.
[ieof](../helpindex/cmds/ieof.md) Check the "eof"-flag of an input stream.
[oeof](../helpindex/cmds/oeof.md) Check the "eof"-flag of an output stream.
[eof](../helpindex/cmds/eof.md) Check eof status of a stream.
[igood](../helpindex/cmds/igood.md) Check the "good"-flag of a stream.
[ogood](../helpindex/cmds/ogood.md) Check the "good"-flag of an output stream.
[good](../helpindex/cmds/good.md) Check good status of a stream.

### Handling Image Files

[readPGM](../helpindex/cmds/readPGM.md) Read in grey-level image in PGM Format.
[writePGM](../helpindex/cmds/writePGM.md) Erite out a grey-level image in PGM
format.

### Editing Files

[edit](../helpindex/cmds/edit.md) .Edit a file.

## Executing SLI programs

[run](../helpindex/cmds/run.md) Execute a .sli file.
[spoon](../helpindex/cmds/spoon.md) Execute a parallel SLI-process.
[setpath](../helpindex/cmds/setpath.md) Set the SLI search path.
[addpath](../helpindex/cmds/addpath.md) Append a path to SLISearchPath.
[path](../helpindex/cmds/path.md) Return current search path as array.

## Interfacing the hosts file-system

[Directory](../helpindex/cmds/Directory.md) Return current working directory.
[cd](../helpindex/cmds/cd.md) Change working directory.
[SetDirectory](../helpindex/cmds/SetDirectory.md) Change working directory.
[ls](../helpindex/cmds/ls.md) Print contents of current working directory.
[FileNames](../helpindex/cmds/FileNames.md) Return contents of current working
directory.
[DeleteFile](../helpindex/cmds/DeleteFile.md) Delete a file.
[mkfifo](../helpindex/cmds/mkfifo.md) Create a FIFO special file (named pipe).
[MoveFile](../helpindex/cmds/MoveFile.md) Rename a file.
[LocateFileNames](../helpindex/cmds/LocateFileNames.md) Look up complete
pathnames of given file in given.
[tmpnam](../helpindex/cmds/tmpnam.md) Return valid nonexisting file name.
[MakeDirectory](../helpindex/cmds/MakeDirectory.md) Create a new directory.
[MoveDirectory](../helpindex/cmds/MoveDirectory.md) Rename a directory.
[RemoveDirectory](../helpindex/cmds/RemoveDirectory.md) Delete a directory.
[CompareFiles](../helpindex/cmds/CompareFiles.md) Compare two files for
equality.

## Process control

### Executing UNIX commands and external programs

[system](../helpindex/cmds/system.md) Execute a UNIX command in a parallel
process.
[spawn](../helpindex/cmds/spawn.md) Spawn a UNIX process and redirect stdin and
stdout.
[shpawn](../helpindex/cmds/shpawn.md) Spawn a UNIX process using a shell and
redirect stdin and stdout.
[sysexec](../helpindex/cmds/sysexec.md) Transfer control to a UNIX-command.
[kill](../helpindex/cmds/kill.md) Send a signal to another process.

### Low-level process control

[exec](../helpindex/cmds/exec.md) Execute an object.
[ctermid](../helpindex/cmds/ctermid.md) Return the path to the controlling
terminal of the process.
[fork](../helpindex/cmds/fork.md) Create a child process of SLI.
[spawn](../helpindex/cmds/spawn.md) Spawn a UNIX process and redirect stdin and
stdout.
[shpawn](../helpindex/cmds/shpawn.md) Spawn a UNIX process using a shell and
redirect stdin and stdout.
[spoon](../helpindex/cmds/spoon.md) Execute a parallel SLI-process.
[kill](../helpindex/cmds/kill.md) Send a signal to another process.
[wait](../helpindex/cmds/wait.md) Wait for any child process to terminate.
[waitPID](../helpindex/cmds/waitPID.md) Wait or check for a child process to
terminate.
[getPGRP](../helpindex/cmds/getPGRP.md) Get process group ID of the current
process.
[getPID](../helpindex/cmds/getPID.md) Get ID of the current process.

### Accessing the process environment

[environment](../helpindex/cmds/environment.md) Return the environment of the
current SLI process.
[setenvironment](../helpindex/cmds/setenvironment.md) Set the environment of the
current SLI process.
[getenv](../helpindex/cmds/getenv.md) Evaluates if a string is an evironment
variable.

### Unit Conversion

[unit\_conversion](../helpindex/cmds/unit_conversion.md) Conversion factors for
SI units.
[Hz](../helpindex/cmds/Hz.md) Specification in Hz (for readability).
[ms](../helpindex/cmds/ms.md) Specification in ms (for readability).
[pA](../helpindex/cmds/pA.md) Specification in pA (for readability).
[mV](../helpindex/cmds/mV.md) Specification in mV (for readability).
[spikes](../helpindex/cmds/spikes.md) Specification in spikes (for readability).

## SLI Interpreter control

### Startup and initialization

[nestrc](../helpindex/cmds/nestrc.md) Personal interpreter initialization file.
[start](../helpindex/cmds/start.md) Interpreter start symbol.
[executive](../helpindex/cmds/executive.md) Start interactive interpreter
session.
[quit](../helpindex/cmds/quit.md) Leave the SLI interpreter.
[welcome](../helpindex/cmds/welcome.md) Print SLI welcome message.

### Controlling the running interpreter

[cycles](../helpindex/cmds/cycles.md) Return the number of elapsed interpreter
cycles.
[clic](../helpindex/cmds/clic.md) Start measuring interpreter cycles.
[cloc](../helpindex/cmds/cloc.md) Return elapsed interpreter cycles since clic
was called.
[setguard](../helpindex/cmds/setguard.md) Limit the number of interpreter
cycles.
[removeguard](../helpindex/cmds/removeguard.md) Removes the limit on the number
of interpreter cycles.
[allocations](../helpindex/cmds/allocations.md) Return the number of array
reallocations.
[setcallback](../helpindex/cmds/setcallback.md) Install a callback object in the
interpreter cycle.
[clearcallback](../helpindex/cmds/clearcallback.md) Clear the
installed callback.
[parsestdin](../helpindex/cmds/parsestdin.md) Read and execute tokens from
standard input.
[pgetrusage](../helpindex/cmds/pgetrusage.md) Get resource consumption
information.
[exithook](../helpindex/cmds/exithook.md) Procedure executed if the executive
mode is left.
[noop](../helpindex/cmds/noop.md) No operation function.

### Interpreter messages

[message](../helpindex/cmds/message.md) Display an information message.
[verbosity](../helpindex/cmds/verbosity.md) Return the current verbosity level
for interpreter messages.
[setverbosity](../helpindex/cmds/setverbosity.md) Set
verbosity level for message.
[endl](../helpindex/cmds/endl.md) Line break.

### Measuring time

[clock](../helpindex/cmds/clock.md) Returns realtime.
[time](../helpindex/cmds/time.md) Return wall clock time in s since
1.1.1970 00:00.
[tic](../helpindex/cmds/tic.md) Start measuring usertime of a command.
[toc](../helpindex/cmds/toc.md) Return usertime since tic was called.
[usertime](../helpindex/cmds/usertime.md) Return clock time in ms
[realtime](../helpindex/cmds/realtime.md) Returns realtime.
[sleep](../helpindex/cmds/sleep.md) Pauses current process.
[systemtime](../helpindex/cmds/systemtime.md) Returns system time for current
process.
[ms2hms](../helpindex/cmds/ms2hms.md) Convert milliseconds to an array
\[h min sec\].
[pclocks](../helpindex/cmds/pclocks.md) Returns POSIX clocks for real, user,
system time.
[pclockspersec](../helpindex/cmds/pclockspersec.md) POSIX clock ticks per
second.
[ptimes](../helpindex/cmds/ptimes.md) Returns real, user, and system time.

## Parallel computing

[Rank](../helpindex/cmds/Rank.md) Return the MPI rank (MPI\_Comm\_rank.md) of
the process.
[SyncProcesses](../helpindex/cmds/SyncProcesses.md) Synchronize all
MPI processes.
[MPIProcessorName](../helpindex/cmds/MPIProcessorName.md) Return an unique
specifier for the compute node (MPI\_Get\_processor\_name).
[NumProcesses](../helpindex/cmds/NumProcesses.md) Return the number of MPI
processes (MPI\_Comm\_size).
