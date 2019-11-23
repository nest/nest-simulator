# NodeCollections in SLI and Python

It was decided in the [Open NEST VC 8 Aug](https://github.com/nest/nest-simulator/wiki/2016-08-08-Open-NEST-Developer-Video-Conference) to remove subnets which represent structure in the simulation kernel and move all structure representation to the script layer in form of NodeCollections (see also [#417](https://github.com/nest/nest-simulator/issues/417)).

## Design of NodeCollections

`NodeCollection`s were introduced as a compact representation of network nodes.

### Principles

`NodeCollection`s represent the nodes of a network on the script level (i.e., outside the kernel). They are as compact as possible and support efficient operations, especially connection generation and status changes or queries.

The script controls the lifetime of the `NodeCollection`, i.e., a `NodeCollection` is deleted when the corresponding object is deleted at the script level, either explicitly or by going out of scope.

Consideration is given first and foremost to the Python level, since it is the primary user interface.

### Terminology
- A `NodeCollection` is _contiguous_ if it represents a contiguous range of node IDs.
- A `NodeCollection` is _homogeneous_ if all node IDs in the collection refer to nodes of the same type (same model ID).
- A `NodeCollection` is _primitive_ if it is contiguous and homogeneous.
- A `NodeCollection` is _composite_ if it is not primitive.

### Assumptions

The following assumptions apply to non-explicit `NodeCollection`s:
1. `NodeCollection` objects are created rarely but looked up frequently
1. `NodeCollection` objects are immutable
1. `NodeCollection` objects will first and foremost be created by `Create` calls; each such call will return a primitive collection
1. A `NodeCollection` holds information about the model ID of each node ID in the collection
1. A `NodeCollection` may hold additional metadata (see section on Topology below for discussion on Metadata)
1. A node ID occurs in a `NodeCollection` at most once
1. node IDs are ordered in ascending order in a `NodeCollection`
1. If a composite `NodeCollection` contains at least one primitive `NodeCollection` with metadata, then all primitive `NodeCollection`s in the `NodeCollection` must hold the same metadata. Sameness means in this case that the metadata pointers are identical.
1. Primitive `NodeCollection`s with metadata are never coalesced into a single primitive `NodeCollection`, even if their ranges are adjacent and they have the same model type.

`NodeCollection`s are made to be able to compactly represent a large number of nodes. Often over 1000 nodes, and, in extreme cases (4g benchmark), billions of nodes.


## Operations on `NodeCollection`s

`NodeCollection`s support the following operations:
1. Test of membership
1. Test whether one `NodeCollection` is equal to another (contains the same node IDs)
1. Concatenation of two non-overlapping `NodeCollection`s
1. Iteration
1. Indexing
1. Slicing
1. Conversion to and from lists

### Examples

#### SLI

```
/Enrns /iaf_psc_alpha 800 Create def  % (1, 800)
/Inrns /iaf_psc_alpha 200 Create def  % (801, 1000)

% Concatenated into a new primitive NodeCollection, (1, 1000)
/nrns Enrns Inrns join def

Enrns nrns << /rule /fixed_indegree /indegree 100 >> Connect

Enrns 10 MemberQ =  % Test of membership

% Create composite NodeCollection from an array
/nc [ 10 20 30 ] cvnodecollection def
/Ilist Inrns cva def  % Convert NodeCollection to array

Enrns Inrns eq =  % Test of equality

Enrns { ShowStatus } forall  % Iteration

Inrns 10 get ==  % Indexing, gives a new NodeCollection
Enrns 20 Take ==  % Slicing, gives a new NodeCollection
```

#### PyNEST

``` Python
# Create two primitive NodeCollections
Enrns = nest.Create('iaf_psc_alpha', 800)
Inrns = nest.Create('iaf_psc_alpha', 200)
nrns = Enrns + Inrns  # Concatenated into a new primitive NodeCollection

nest.Connect(Enrns, nrns, {'rule': 'fixed_indegree', 'indegree': 100})

10 in Enrns  # Test of membership

# Create composite NodeCollection from a list
nc = nest.NodeCollection([10, 20, 30])
Ilist = list(nc)  # Convert NodeCollection to a Python list

# Iteration
for node_id in Enrns:
    print(node_id)

print(Inrns[10])  # Indexing, gives a new NodeCollection
print(Enrns[:20])  # Slicing, gives a new NodeCollection
```

### Topology and Metadata

This first use of metadata for `NodeCollection`s is the subnet-free reimplementation of Topology ([#481](https://github.com/nest/nest-simulator/issues/481)). Layers containing only a single neuron model are represented by primitive `NodeCollection`s, while layers with composite elements require composite `NodeCollection`s with one primitive `NodeCollection` per component. All these primitive `NodeCollection`s are part of the same layer and thus share the same geometry. They therefore have all the same metadata. This motivates the requirement that all primitive `NodeCollection`s in a composite `NodeCollection` must have the same metadata.

`NodeCollection`s with metadata are never coalesced, even if they have adjacent ranges and identical model ids, since users may have specified composite elements with identical models.

For minor changes to the user interface, see [#481](https://github.com/nest/nest-simulator/issues/481).

### Remarks
- Tests can be implemented efficiently by exploiting sortedness.
- Concatenation permitted if uniqueness is preserved and will preserve sortedness.
- Iteration, indexing and slicing will be based on sortedness and are thus deterministic and independent of how the `NodeCollection` was constructed.
- The operations are required to permit existing Python scripts employing operations on lists of node IDs to continue to work (`neurons = Eneurons + Ineurons`, `Connect(E_neurons[:50], spike_det)`)

## Implementation

### Python interface
- Representation at the Python level is just a pointer to the underlying C++ representation.
- All operations listed above are available.

### Composite `NodeCollection`s

Non-primitive `NodeCollection`s are represented as lists of pointers to the `NodeCollection`s they are constructed from. Any immediately adjacent `NodeCollection`s of the same node type are combined into a primitive `NodeCollection`.

### Model ID information
- Each primitive `NodeCollection` stores the model ID of the node IDs it represents.
- The main purpose of storing model ID information in `NodeCollection`s is to reduce node ID-based lookups for model type, existence of thread-siblings, etc in connection, setting and getting routines.

### Metadata implementation

To provide necessary flexibility, a `NodeCollectionMetadata` abstract base class is provided. This class has only a minimal interface, all details are added by derived classes special to the use of the metadata, e.g., in topology:

``` C++
class NodeCollectionMetadata
{
  public:
     NodeCollectionMetadata() {}
     virtual ~NodeCollectionMetadata() = 0;
};
```

Since all elements of a composite `NodeCollection` must contain the same `NodeCollection` pointer, the composite `NodeCollection` can also return a metadata pointer.

For an example of metadata, see [#481](https://github.com/nest/nest-simulator/issues/481).


## Remarks based on Design Discussion 8 Nov 2016

@terhorstd, @stinebuu, @hakonsbm, and @heplesser discussed design via VC on 8 Nov. Here are remarks based on decisions from this VC.

- We have not included "explicit collections" (general unsorted non-unique arrays) in `NodeCollection` interface.
- NodeCollectionComposite is always a flat container for NodeCollectionPrimitives. A NodeCollectionComposite can therefore not contain NodeCollectionComposites.
- Joining of primitives is allowed only for same metadata (pointer equality).
- `NodeCollection`s are homogeneous containers.
- Structure between different populations has to be taken care of by the user.
- If structure handling becomes problematic, a new container class for the specific case can be added (external of `NodeCollection`s).


## Fingerprints

After calling `ResetKernel()` all nodes and metadata is gone, but we still have the `NodeCollection`s. It is important that we are not able to use these `NodeCollection`s in functions such as `Connect`, because the user might have created a new set of nodes or metadata that does not correspond to the old `NodeCollection`, and errors will occur.

To solve this problem we have introduced fingerprints. This will make it possible to check whether the `NodeCollection` is valid. A fingerprint is introduced in the Kernel, and in the `NodeCollection` class. We use a timestamp as the fingerprint. Every time `ResetKernel()` is called, the fingerprint in the Kernel is updated. When a `NodeCollection` is created, we retrieve the fingerprint from the Kernel, and store it in the `NodeCollection` class. It is then possible to test against the Kernel fingerprint and see whether the `NodeCollection` is valid when we use `Connect` or other functions that rely on correctly created `NodeCollection`s. If we have an invalid `NodeCollection` (i.e. the fingerprint does not match that of the Kernel), an `IncorrectNodeCollection` exception is raised.
