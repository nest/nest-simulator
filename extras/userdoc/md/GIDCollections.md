# GIDCollections in SLI and Python

It was decided in the [Open NEST VC 8 Aug](https://github.com/nest/nest-simulator/wiki/2016-08-08-Open-NEST-Developer-Video-Conference) to remove subnets which represent structure in the simulation kernel and move all structure representation to the script layer in form of GIDCollections (see also [#417](https://github.com/nest/nest-simulator/issues/417)).

## Design of GIDCollections

`GIDCollection`s were introduced as a compact representation of network nodes.

### Principles

`GIDCollection`s represent the nodes of a network on the script level (i.e., outside the kernel). They are as compact as possible and support efficient operations, especially connection generation and status changes or queries.

The script controls the lifetime of the `GIDCollection`, i.e., a `GIDCollection` is deleted when the corresponding object is deleted at the script level, either explicitly or by going out of scope.

Consideration is given first and foremost to the Python level, since it is the primary user interface.

### Terminology
- A `GIDCollection` is _contiguous_ if it represents a contiguous range of GIDs.
- A `GIDCollection` is _homogeneous_ if all GIDs in the collection refer to nodes of the same type (same model ID).
- A `GIDCollection` is _primitive_ if it is contiguous and homogeneous.
- A `GIDCollection` is _composite_ if it is not primitive.

### Assumptions

The following assumptions apply to non-explicit `GIDCollection`s:
1. `GIDCollection` objects are created rarely but looked up frequently
1. `GIDCollection` objects are immutable
1. `GIDCollection` objects will first and foremost be created by `Create` calls; each such call will return a primitive collection
1. A `GIDCollection` holds information about the model ID of each GID in the collection
1. A `GIDCollection` may hold additional metadata (see section on Topology below for discussion on Metadata)
1. A GID occurs in a `GIDCollection` at most once
1. GIDs are ordered in ascending order in a `GIDCollection`
1. If a composite `GIDCollection` contains at least one primitive `GIDCollection` with metadata, then all primitive `GIDCollection`s in the `GIDCollection` must hold the same metadata. Sameness means in this case that the metadata pointers are identical.
1. Primitive `GIDCollection`s with metadata are never coalesced into a single primitive `GIDCollection`, even if their ranges are adjacent and they have the same model type.

`GIDCollection`s are made to be able to compactly represent a large number of nodes. Often over 1000 nodes, and, in extreme cases (4g benchmark), billions of nodes.


## Operations on `GIDCollection`s

`GIDCollection`s support the following operations:
1. Test of membership
1. Test whether one `GIDCollection` is equal to another (contains the same GIDs)
1. Concatenation of two non-overlapping `GIDCollection`s
1. Iteration
1. Indexing
1. Slicing
1. Conversion to and from lists

### Examples

#### SLI

```
/Enrns /iaf_psc_alpha 800 Create def  % (1, 800)
/Inrns /iaf_psc_alpha 200 Create def  % (801, 1000)

% Concatenated into a new primitive GIDCollection, (1, 1000)
/nrns Enrns Inrns join def

Enrns nrns << /rule /fixed_indegree /indegree 100 >> Connect

Enrns 10 MemberQ =  % Test of membership

% Create composite GIDCollection from an array
/gc [ 10 20 30 ] cvgidcollection def
/Ilist Inrns cva def  % Convert GIDCollection to array

Enrns Inrns eq =  % Test of equality

Enrns { ShowStatus } forall  % Iteration

Inrns 10 get ==  % Indexing, gives a new GIDCollection
Enrns 20 Take ==  % Slicing, gives a new GIDCollection
```

#### PyNEST

``` Python
# Create two primitive GIDCollections
Enrns = nest.Create('iaf_psc_alpha', 800)
Inrns = nest.Create('iaf_psc_alpha', 200)
nrns = Enrns + Inrns  # Concatenated into a new primitive GIDCollection

nest.Connect(Enrns, nrns, {'rule': 'fixed_indegree', 'indegree': 100})

10 in Enrns  # Test of membership

# Create composite GIDCollection from a list
gc = nest.GIDCollection([10, 20, 30])
Ilist = list(gc)  # Convert GIDCollection to a Python list

# Iteration
for gid in Enrns:
    print(gid)

print(Inrns[10])  # Indexing, gives a new GIDCollection
print(Enrns[:20])  # Slicing, gives a new GIDCollection
```

### Topology and Metadata

This first use of metadata for `GIDCollection`s is the subnet-free reimplementation of Topology ([#481](https://github.com/nest/nest-simulator/issues/481)). Layers containing only a single neuron model are represented by primitive `GIDCollection`s, while layers with composite elements require composite `GIDCollection`s with one primitive `GIDCollection` per component. All these primitive `GIDCollection`s are part of the same layer and thus share the same geometry. They therefore have all the same metadata. This motivates the requirement that all primitive `GIDCollection`s in a composite `GIDCollection` must have the same metadata.

`GIDCollection`s with metadata are never coalesced, even if they have adjacent ranges and identical model ids, since users may have specified composite elements with identical models.

For minor changes to the user interface, see [#481](https://github.com/nest/nest-simulator/issues/481).

### Remarks
- Tests can be implemented efficiently by exploiting sortedness.
- Concatenation permitted if uniqueness is preserved and will preserve sortedness.
- Iteration, indexing and slicing will be based on sortedness and are thus deterministic and independent of how the `GIDCollection` was constructed.
- The operations are required to permit existing Python scripts employing operations on lists of GIDs to continue to work (`neurons = Eneurons + Ineurons`, `Connect(E_neurons[:50], spike_det)`)

## Implementation

### Python interface
- Representation at the Python level is just a pointer to the underlying C++ representation.
- All operations listed above are available.

### Composite `GIDCollection`s

Non-primitive `GIDCollection`s are represented as lists of pointers to the `GIDCollection`s they are constructed from. Any immediately adjacent `GIDCollection`s of the same node type are combined into a primitive `GIDCollection`.

### Model ID information
- Each primitive `GIDCollection` stores the model ID of the GIDs it represents.
- The main purpose of storing model ID information in `GIDCollection`s is to reduce GID-based lookups for model type, existence of thread-siblings, etc in connection, setting and getting routines.

### Metadata implementation

To provide necessary flexibility, a `GIDCollectionMetadata` abstract base class is provided. This class has only a minimal interface, all details are added by derived classes special to the use of the metadata, e.g., in topology:

``` C++
class GIDCollectionMetadata 
{
  public:
     GIDCollectionMetadata() {}
     virtual ~GIDCollectionMetadata() = 0;
};
```

Since all elements of a composite `GIDCollection` must contain the same `GIDCollection` pointer, the composite `GIDCollection` can also return a metadata pointer.

For an example of metadata, see [#481](https://github.com/nest/nest-simulator/issues/481).


## Remarks based on Design Discussion 8 Nov 2016

@terhorstd, @stinebuu, @hakonsbm, and @heplesser discussed design via VC on 8 Nov. Here are remarks based on decisions from this VC.

- We have not included "explicit collections" (general unsorted non-unique arrays) in `GIDCollection` interface.
- GIDCollectionComposite is always a flat container for GIDCollectionPrimitives. A GIDCollectionComposite can therefore not contain GIDCollectionComposites.
- Joining of primitives is allowed only for same metadata (pointer equality).
- `GIDCollection`s are homogeneous containers.
- Structure between different populations has to be taken care of by the user.
- If structure handling becomes problematic, a new container class for the specific case can be added (external of `GIDCollection`s).


## Fingerprints

After calling `ResetKernel()` all nodes and metadata is gone, but we still have the `GIDCollection`s. It is important that we are not able to use these `GIDCollection`s in functions such as `Connect`, because the user might have created a new set of nodes or metadata that does not correspond to the old `GIDCollection`, and errors will occur.

To solve this problem we have introduced fingerprints. This will make it possible to check whether the `GIDCollection` is valid. A fingerprint is introduced in the Kernel, and in the `GIDCollection` class. We use a timestamp as the fingerprint. Every time `ResetKernel()` is called, the fingerprint in the Kernel is updated. When a `GIDCollection` is created, we retrieve the fingerprint from the Kernel, and store it in the `GIDCollection` class. It is then possible to test against the Kernel fingerprint and see whether the `GIDCollection` is valid when we use `Connect` or other functions that rely on correctly created `GIDCollection`s. If we have an invalid `GIDCollection` (i.e. the fingerprint does not match that of the Kernel), an `IncorrectGIDCollection` exception is raised.
