#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <mpi.h>

namespace arb::shadow {

using cell_gid_type = std::uint32_t;
using cell_lid_type = std::uint32_t;
struct cell_member_type;
using time_type = float;
constexpr time_type terminal_time = std::numeric_limits<time_type>::max();

template <typename I>
struct basic_spike;
using spike = basic_spike<cell_member_type>;

std::vector<spike> gather_spikes(const std::vector<spike>& values, MPI_Comm comm);

int mpi_rank(MPI_Comm c);
int mpi_size(MPI_Comm c);
int broadcast(int local, MPI_Comm comm, int root);
unsigned broadcast(unsigned local, MPI_Comm comm, int root);
float broadcast(float local, MPI_Comm comm, int root);

struct comm_info;
comm_info get_comm_info(bool is_arbor, MPI_Comm comm);

} // namespace arb
