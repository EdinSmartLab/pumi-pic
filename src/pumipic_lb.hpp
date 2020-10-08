#pragma once

#include <Omega_h_array.hpp>
#include <Omega_h_for.hpp>
#include "pumipic_mesh.hpp"
#include <ngraph.h>
#include <engpar_support.h>
#include <engpar_weight_input.h>
#include <engpar.h>
#include <particle_structs.hpp>

namespace {
  typedef std::set<int> Parts;
  class PartsHash {
  public:
    size_t operator()(const Parts& res) const {
      int h = 0;
      std::hash<int> hasher;
      for (auto itr = res.begin(); itr != res.end(); ++itr)
        h ^= hasher(*itr);
      return h;
    }
  };
}
namespace pumipic {

  //Print particle imbalance statistics
  template <class PS>
  void printPtclImb(PS* ptcls, MPI_Comm comm = MPI_COMM_WORLD);

  class Mesh;
  class ParticlePlan;

  class ParticleBalancer {
  public:
    //Build Ngraph from sbars
    ParticleBalancer(Mesh& picparts);
    ~ParticleBalancer();

    /* Performs particle load balancing and redistributes particles
       picparts(in) - the picparts mesh
       ps(in) - the particle structure
       tol(in) - the target imbalance (5% would be a value of 1.05)
       new_elems(in) - the new elements each particle is moving to
       new_procs(in/out) - the new processes for each particle,
           will be changed to satisfy load balance
           Note: particles pushed outside the safe zone must have new process already set
       step_factor(in) - (optional) the rate of weight transfer
     */
    template <class PS>
    void repartition(Mesh& picparts, PS* ps, double tol,
                     typename PS::kkLidView new_elems,
                     typename PS::kkLidView new_procs,
                     double step_factor = 0.5);

    //Access the sbar ids per element
    Omega_h::LOs getSbarIDs(Mesh& picparts) const;

    /* Steps of repartition, can be called on their own for customization */

    //adds the weight of particles in ps to graph
    template <class PS>
    void addWeights(Mesh& picparts, PS* ps, typename PS::kkLidView new_elems,
                    typename PS::kkLidView new_procs);

    //run the weight balancer and return the plan
    ParticlePlan balance(double tol, double step_factor = 0.5);

    template <class PS>
    void selectParticles(Mesh& picparts, PS* ps, typename PS::kkLidView new_elems,
                         ParticlePlan plan, typename PS::kkLidView new_parts);
  private:
    typedef std::unordered_map<Parts, int, PartsHash> SBarUnmap;
    int max_sbar;
    SBarUnmap sbar_ids;
    Omega_h::HostWrite<int> elm_sbar;
    agi::Ngraph* weightGraph;
    std::unordered_map<agi::gid_t,int> vert_to_sbar;
    std::unordered_map<agi::gid_t, agi::part_t> vert_to_owner;
    Kokkos::UnorderedMap<int, agi::lid_t> sbar_to_vert;

    //select particles to migrate
    void makePlan();

    SBarUnmap::iterator insert(Parts& p);
    Omega_h::HostWrite<int> buildLocalSbarMap(int comm_rank, int nelms,
                                              Omega_h::HostWrite<Omega_h::LO> buffer_ranks,
                                              Omega_h::HostWrite<Omega_h::LO> safe_per_buffer);
    void sendCoreSbars(Omega_h::CommPtr comm, Omega_h::HostWrite<Omega_h::LO> buffer_ranks);
    void globalNumberSbars(Omega_h::CommPtr comm,
                           std::unordered_map<int,int>& sbar_local_to_global);
    void cleanSbars(int comm_rank);

    void numberElements(Mesh& picparts, Omega_h::HostWrite<int> elm_sbar,
                        std::unordered_map<int, int>& map);
    void buildNgraph(Omega_h::CommPtr comm);
  };

  class ParticlePlan {
  public:
    ParticlePlan(std::unordered_map<Omega_h::LO, Omega_h::LO>& sbar_index_map,
                 Omega_h::Write<Omega_h::LO> tgt_parts, Omega_h::Write<Omega_h::Real> wgts);
    friend class ParticleBalancer;
  private:
    Kokkos::UnorderedMap<int, int> sbar_to_index;
    Omega_h::LOs part_ids;
    Omega_h::Write<Omega_h::Real> send_wgts;
  };

  template <class PS>
  void ParticleBalancer::addWeights(Mesh& picparts, PS* ptcls,
                                    typename PS::kkLidView new_elems,
                                    typename PS::kkLidView new_procs) {
    MPI_Comm comm = picparts.comm()->get_impl();
    int comm_rank = picparts.comm()->rank();
    // Device map of number of particles already assigned to another process
    Kokkos::UnorderedMap<int, agi::wgt_t> forcedPtcls(picparts.numBuffers(picparts->dim()));
    Omega_h::Write<Omega_h::LO> buffered_ranks(picparts.bufferedRanks(picparts->dim()));
    Kokkos::parallel_for(buffered_ranks.size(), KOKKOS_LAMBDA(const int i) {
        forcedPtcls.insert(buffered_ranks[i], 0);
    });

    //Count particles in each sbar & count particles already being migrated
    Omega_h::Write<agi::wgt_t> weights(sbar_ids.size() + 1, 0);
    Omega_h::LOs elem_sbars = getSbarIDs(picparts);
    auto sbar_to_vert_local = sbar_to_vert;
    auto accumulateWeight = PS_LAMBDA(const int elm, const int ptcl, const bool mask) {
      if (mask) {
        const int new_rank = new_procs(ptcl);
        if (new_rank == comm_rank) {
          const int e = new_elems(ptcl);
          if (e != -1) {
            int sbar_index = elem_sbars[e];
            if (sbar_to_vert_local.exists(sbar_index)) {
              auto index = sbar_to_vert_local.find(sbar_index);
              const agi::lid_t vert_index = sbar_to_vert_local.value_at(index);
              Kokkos::atomic_add(&(weights[vert_index]), 1.0);
            }
          }
        }
        else {
          const auto index = forcedPtcls.find(new_rank);
          Kokkos::atomic_add(&(forcedPtcls.value_at(index)), 1.0);
        }
      }
    };
    parallel_for(ptcls, accumulateWeight, "accumulateWeight");

    //Transfer map to host
    Omega_h::Write<int> owners(buffered_ranks.size(), "owners");
    Omega_h::Write<agi::wgt_t> wgts(buffered_ranks.size(), "owners");
    Omega_h::Write<int> index(1, 0);
    Kokkos::parallel_for(forcedPtcls.capacity(), KOKKOS_LAMBDA (uint32_t i) {
      if( forcedPtcls.valid_at(i) ) {
        const int map_index = Kokkos::atomic_fetch_add(&(index[0]), 1);
        owners[map_index] = forcedPtcls.key_at(i);
        wgts[map_index] = forcedPtcls.value_at(i);
      }
    });
    Omega_h::HostWrite<int> owners_host(owners);
    Omega_h::HostWrite<agi::wgt_t> wgts_host(wgts);

    //Send wgts to peers
    int num_peers = owners_host.size();
    agi::wgt_t* peer_wgts = new agi::wgt_t[num_peers];
    MPI_Request* send_requests = new MPI_Request[num_peers];
    MPI_Request* recv_requests = new MPI_Request[num_peers];
    for (int i = 0; i < num_peers; ++i) {
      MPI_Irecv(peer_wgts + i, 1, MPI_DOUBLE, owners_host[i],
                0, comm, recv_requests + i);
      MPI_Isend(&(wgts_host[i]), 1, MPI_DOUBLE, owners_host[i],
                0, comm, send_requests + i);
    }
    MPI_Waitall(num_peers, recv_requests, MPI_STATUSES_IGNORE);
    delete [] recv_requests;

    //Accumulate all received weight on the last vertex
    Omega_h::HostWrite<Omega_h::Real> weights_host(weights);
    for (int i = 0; i < num_peers; ++i) {
      weights_host[weights_host.size() - 1] += peer_wgts[i];
    }

    weightGraph->setWeights(weights_host.data());
    MPI_Waitall(num_peers, send_requests, MPI_STATUSES_IGNORE);
    delete [] send_requests;
    delete [] peer_wgts;
  }

  template <class PS>
  void ParticleBalancer::selectParticles(Mesh& picparts, PS* ptcls,
                                         typename PS::kkLidView new_elems,
                                         ParticlePlan plan,
                                         typename PS::kkLidView new_parts) {


    int comm_rank = picparts.comm()->rank();
    Omega_h::LOs sbars = getSbarIDs(picparts);
    auto send_wgts = plan.send_wgts;
    auto sbar_to_index = plan.sbar_to_index;
    auto part_ids = plan.part_ids;

    auto selectParticles = PS_LAMBDA(const int elm, const int ptcl, const bool mask) {
      if (mask) {
        if (new_parts(ptcl) == comm_rank) {
          const int e = new_elems(ptcl);
          if (e != -1) {
            const Omega_h::LO sbar = sbars[e];
            if (sbar_to_index.exists(sbar)) {
              const auto map_index = sbar_to_index.find(sbar);
              const Omega_h::LO index = sbar_to_index.value_at(map_index);
              const Omega_h::LO part = part_ids[index];
              const Omega_h::Real wgt = Kokkos::atomic_fetch_add(&(send_wgts[index]), -1);
              if (part >= 0) {
                if (wgt == 0)
                  Kokkos::atomic_add(&(sbar_to_index.value_at(map_index)), 1);
                if (wgt > 0)
                  new_parts[ptcl] = part;
              }
            }
          }
        }
      }
    };
    parallel_for(ptcls, selectParticles, "selectParticles");
  }

  template <class PS>
  void ParticleBalancer::repartition(Mesh& picparts, PS* ptcls, double tol,
                                     typename PS::kkLidView new_elems,
                                     typename PS::kkLidView new_parts,
                                     double step_factor) {
    addWeights(picparts, ptcls, new_elems, new_parts);
    ParticlePlan plan = balance(tol, step_factor);
    selectParticles(picparts, ptcls, new_elems, plan, new_parts);
  }

  //Print particle imbalance statistics
  template <class PS>
  void printPtclImb(PS* ptcls, MPI_Comm comm) {
    int np = ptcls->nPtcls();
    int min_p, max_p, tot_p;
    MPI_Reduce(&np, &min_p, 1, MPI_INT, MPI_MIN, 0, comm);
    MPI_Reduce(&np, &max_p, 1, MPI_INT, MPI_MAX, 0, comm);
    MPI_Reduce(&np, &tot_p, 1, MPI_INT, MPI_SUM, 0, comm);

    int comm_rank;
    MPI_Comm_rank(comm, &comm_rank);
    int comm_size;
    MPI_Comm_size(comm, &comm_size);
    if (comm_rank == 0) {
      float avg = tot_p / comm_size;
      float imb = max_p / avg;
      printf("Ptcl LB <max, min, avg, imb>: %d %d %.3f %.3f\n", max_p, min_p, avg, imb);
    }

  }

}
