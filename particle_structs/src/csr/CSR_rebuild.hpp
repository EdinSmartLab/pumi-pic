#pragma once

//#include "psMemberType.h"
//For now using '2' version due to issue with dependency tree
#include "psMemberType2.h"

namespace pumipic{
  template<class DataTypes,typename MemSpace>
  void CSR<DataTypes,MemSpace>::rebuild(kkLidView new_element,
                                        kkLidView new_particle_elements,
                                        MTVs new_particles){
    Kokkos::Profiling::pushRegion("CSR Rebuild");
    fprintf(stderr, "CSR Rebuild begun\n");
    
    //Counting of particles on process
    lid_t particles_on_process = countParticlesOnProcess(new_element) + 
                                 countParticlesOnProcess(new_particle_elements);
    fprintf(stderr,"print on next line\n");
    printf("particles on process: %d\n", particles_on_process);
    num_ptcls = particles_on_process;


    //Alocate new (temp) MTV
    MTVs particle_info;
    CreateViews<device_type,DataTypes>(particle_info, particles_on_process);


    //fresh filling of particles_per_element
    kkLidView particles_per_element = kkLidView("particlesPerElement", num_elems+1);
    Kokkos::parallel_for("fill particlesPerElement1", new_element.size(),
        KOKKOS_LAMBDA(const int& i){
          if(new_element[i] > -1)
            Kokkos::atomic_increment(&particles_per_element[new_element[i]]);
        });
    Kokkos::parallel_for("fill particlesPerElement2", new_particle_elements.size(),
        KOKKOS_LAMBDA(const int& i){
          assert(new_particle_elements[i] > -1);
          Kokkos::atomic_increment(&particles_per_element[new_particle_elements[i]]);
        });

    Kokkos::fence();
    printView(particles_per_element);
    fprintf(stderr,"Ptcls per elem set\n");


    //refill offset here 
    auto offsets_new = kkLidView("offsets", num_elems+1);
    Kokkos::deep_copy(offsets_new, offsets);
    exclusive_scan(particles_per_element, offsets_new);
    //assert(capacity_ == getLastValue(offsets)); 
    printView(offsets_new);

    //Determine new_indices for all of the exisitng particles
    //auto offset_cpy = offsets;
    kkLidView row_indices("row indices", num_elems+1);
    Kokkos::deep_copy(row_indices,offsets_new);
    kkLidView new_indices("new indices", new_element.size());
    Kokkos::parallel_for("new_indices", new_element.size(), KOKKOS_LAMBDA(const int& i){
      const lid_t new_elem = new_element(i);
      if(new_elem != -1){
        new_indices(i) = Kokkos::atomic_fetch_add(&row_indices(new_elem),1);
      }
      else
        new_indices(i) = -1;
    });

    printf("new_element size: %d\n", new_element.size());
    printView(new_element);


    printf("new_indices size: %d\n", new_indices.size());
    printView(new_indices);
    fprintf(stderr,"\nnew indices printed \n");

    printf("particle info size: %d\n", ps::getMemberView<DataTypes,0>(particle_info).size());
    printf("particle data size: %d\n", ps::getMemberView<DataTypes,0>(ptcl_data).size());
    printf("new element   size: %d\n", new_element.size());
    printf("new indices   size: %d\n", new_indices.size());
    

    //Copy existing particles to their new location in the temp MTV
    Kokkos::fence();
    CopyPSToPS2< CSR<DataTypes,MemSpace> , DataTypes >(this, particle_info, ptcl_data, new_element, new_indices);
    Kokkos::fence();

    fprintf(stderr,"copy ps to ps complete");
    //Reallocate ptcl_data
    destroyViews<DataTypes>(ptcl_data);
    CreateViews<device_type,DataTypes>(ptcl_data, capacity_);
    fprintf(stderr,"create new ptcl_data MTV complete");

    //If there are new particles
    lid_t num_new_ptcls = new_particle_elements.size();
    kkLidView new_particle_indices("new_particle_indices", num_new_ptcls); 

    //Determine new particle indices in the MTVs
    Kokkos::parallel_for("new_patricles_indices", num_new_ptcls,
                            KOKKOS_LAMBDA(const int& i){
      lid_t new_elem = new_particle_elements(i);
      new_particle_indices(i) = Kokkos::atomic_fetch_add(&row_indices(new_elem),1);
    });

    if(num_new_ptcls > 0){
      CopyViewsToViews<kkLidView,DataTypes>(particle_info, new_particles, 
                                                          new_particle_indices);
    }

    //Resassign all member variables
    ptcl_data = particle_info;
    capacity_ = getLastValue<lid_t>(offsets);
    num_ptcls = capacity_;
    offsets   = offsets_new;
    printMetrics();
    fprintf(stderr, "ptcl_data:\n");
    printView(ps::getMemberView<DataTypes,0>(ptcl_data));

    Kokkos::Profiling::popRegion();
    fprintf(stderr, "CSR rebuild complete\n");
  }
}
