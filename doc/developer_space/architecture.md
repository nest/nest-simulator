# Architecture Overview {#devdoc_architecture}

This page gives a high-level description of the NEST simulator architecture.

## Subsystems

### Kernel

The simulation kernel manages the lifecycle of nodes (neurons and devices),
connections (synapses), and the simulation loop. It is implemented in
`nestkernel/` and exposed directly to PyNEST through a compiled C Python
extension module (no intermediate interpreter layer since NEST 3.10).

Key classes:
- `nest::KernelManager` — singleton that owns and coordinates all kernel
  components
- `nest::NodeManager` — creates and stores neuron and device instances
- `nest::ConnectionManager` — stores and manages synaptic connections
- `nest::SimulationManager` — drives the time-stepped simulation loop

### PyNEST

The Python interface (`pynest/`) wraps the C++ kernel via a compiled C
extension module. High-level API functions are implemented in Python in
`pynest/nest/lib/`. PyNEST calls into the kernel directly using the
`nest::` C++ API.

### Models

Neuron and synapse models live in `models/`. Each model is a self-contained C++
class registered with the `ModelManager`.

## Data Flow

```
User (PyNEST) -> pynest C extension -> nestkernel C++ -> NodeManager / ConnectionManager
                                                       -> SimulationManager (time loop)
                                                            -> Node::update() per neuron
```

### Subsystem component diagram

\startuml
skinparam componentStyle rectangle
skinparam backgroundColor white

package "pynest/" {
  [Python API\n(nest/lib/)] as pyapi
  [C Extension Module] as cext
}

package "nestkernel/" {
  [KernelManager] as km
  [NodeManager] as nm
  [ConnectionManager] as cm
  [SimulationManager] as sm
  [MPIManager] as mpi
  [VPManager] as vp
}

package "models/" {
  [Neuron models] as neurons
  [Synapse models] as synapses
}

[User code] --> pyapi
pyapi --> cext : Python/C API
cext --> km : nest:: C++ calls

km --> nm
km --> cm
km --> sm
km --> mpi
km --> vp

nm --> neurons : instantiates
cm --> synapses : instantiates

sm -[hidden]-> mpi
\enduml

## Parallelism

NEST supports both shared-memory (OpenMP threads) and distributed-memory (MPI)
parallelism. Threads own disjoint subsets of local nodes. MPI processes each
simulate a subset of the network and exchange spike data via MPI collectives at
the end of each min-delay interval.

See also: `nestkernel/vp_manager.h`, `nestkernel/mpi_manager.h`.

## Further Reading

- \ref devdoc_design_decisions "Design Decisions"
- \ref devdoc_coding_conventions "Coding Conventions"
