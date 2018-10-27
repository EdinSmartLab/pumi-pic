#include "Push.h"
#include "psParams.h"
#include <Kokkos_Core.hpp>

void printTiming(const char* name, int np, double t) {
  fprintf(stderr, "kokkos %s (seconds) %f\n", name, t);
  fprintf(stderr, "kokkos %s (particles/seconds) %f\n", name, np/t);
  fprintf(stderr, "kokkos %s (TFLOPS) %f\n", name, (np/t/TERA)*PARTICLE_OPS);
}

void push_array(int np, fp_t* xs, fp_t* ys, fp_t* zs,
    int* ptcl_to_elem, elemCoords& elems,
    fp_t distance, fp_t dx, fp_t dy, fp_t dz,
    fp_t* new_xs, fp_t* new_ys, fp_t* new_zs) {
  for (int i = 0; i < np; ++i) {
    int e = ptcl_to_elem[i];
    fp_t c = elems.x[e]   + elems.y[e]   + elems.z[e]   +
               elems.x[e+1] + elems.y[e+1] + elems.z[e+1] +
               elems.x[e+2] + elems.y[e+2] + elems.z[e+2] +
               elems.x[e+3] + elems.y[e+3] + elems.z[e+3];
    c /= 4; // get schwifty!
    new_xs[i] = xs[i] + c * distance * dx;
    new_ys[i] = ys[i] + c * distance * dy;
    new_zs[i] = zs[i] + c * distance * dz;
  }
}

#ifdef KOKKOS_ENABLED
typedef Kokkos::DefaultExecutionSpace exe_space;

//TODO Figure out how to template these helper fns
typedef Kokkos::View<fp_t*, exe_space::device_type> kkFpView;
/** \brief helper function to transfer a host array to a device view
 */
void hostToDeviceFp(kkFpView d, fp_t* h) {
  kkFpView::HostMirror hv = Kokkos::create_mirror_view(d);
  for (size_t i=0; i<hv.size(); ++i)
    hv(i) = h[i];
  Kokkos::deep_copy(d,hv);
}
/** \brief helper function to transfer a device view to a host array
 */
void deviceToHostFp(kkFpView d, fp_t* h) {
  kkFpView::HostMirror hv = Kokkos::create_mirror_view(d);
  Kokkos::deep_copy(hv,d);
  for(size_t i=0; i<hv.size(); ++i)
    h[i] = hv(i);
}

typedef int lid_t;
typedef Kokkos::View<lid_t*, exe_space::device_type> kkLidView;
/** \brief helper function to transfer a host array to a device view
 */
void hostToDeviceLid(kkLidView d, lid_t* h) {
  kkLidView::HostMirror hv = Kokkos::create_mirror_view(d);
  for (size_t i=0; i<hv.size(); ++i)
    hv(i) = h[i];
  Kokkos::deep_copy(d,hv);
}

void push_array_kk(int np, fp_t* xs, fp_t* ys, fp_t* zs,
    int* ptcl_to_elem, elemCoords& elems,
    fp_t distance, fp_t dx, fp_t dy, fp_t dz,
    fp_t* new_xs, fp_t* new_ys, fp_t* new_zs) {
  Kokkos::Timer timer;
  kkFpView xs_d("xs_d", np);
  hostToDeviceFp(xs_d, xs);

  kkFpView ys_d("ys_d", np);
  hostToDeviceFp(ys_d, ys);

  kkLidView ptcl_to_elem_d("ptcl_to_elem_d", np);
  hostToDeviceLid(ptcl_to_elem_d, ptcl_to_elem);

  kkFpView ex_d("ex_d", elems.num_elems*elems.verts_per_elem);
  hostToDeviceFp(ex_d, elems.x);
  kkFpView ey_d("ey_d", elems.num_elems*elems.verts_per_elem);
  hostToDeviceFp(ey_d, elems.y);
  kkFpView ez_d("ez_d", elems.num_elems*elems.verts_per_elem);
  hostToDeviceFp(ez_d, elems.z);

  kkFpView zs_d("zs_d", np);
  hostToDeviceFp(zs_d, zs);

  kkFpView new_xs_d("new_xs_d", np);
  hostToDeviceFp(new_xs_d, new_xs);

  kkFpView new_ys_d("new_ys_d", np);
  hostToDeviceFp(new_ys_d, new_ys);

  kkFpView new_zs_d("new_zs_d", np);
  hostToDeviceFp(new_zs_d, new_zs);

  fp_t disp[4] = {distance,dx,dy,dz};
  kkFpView disp_d("direction_d", 4);
  hostToDeviceFp(disp_d, disp);
  fprintf(stderr, "array host to device transfer (seconds) %f\n", timer.seconds());

  #if defined(KOKKOS_ENABLE_CXX11_DISPATCH_LAMBDA)
  double avgTime = 0;
  double max = 0;
  double min = 1000;
  for( int iter=0; iter < NUM_ITERATIONS; iter++) {
    timer.reset();
    Kokkos::parallel_for (np, KOKKOS_LAMBDA (const int i) {
        int e = ptcl_to_elem_d(i);
        fp_t c = ex_d(e)   + ey_d(e)   + ez_d(e)   +
                   ex_d(e+1) + ey_d(e+1) + ez_d(e+1) +
                   ex_d(e+2) + ey_d(e+2) + ez_d(e+2) +
                   ex_d(e+3) + ey_d(e+3) + ez_d(e+3);
        c /= 4; // get schwifty!
        new_xs_d(i) = xs_d(i) + c * disp_d(0) * disp_d(1);
        new_ys_d(i) = ys_d(i) + c * disp_d(0) * disp_d(2);
        new_zs_d(i) = zs_d(i) + c * disp_d(0) * disp_d(3);
    });
    double t = timer.seconds();
    avgTime+=t;
    if( t > max ) max = t;
    if( t < min ) min = t;
  }
  avgTime /= NUM_ITERATIONS;
  printTiming("array push avg", np, avgTime);
  printTiming("array push min", np, min);
  printTiming("array push max", np, max);
  #endif

  timer.reset();
  deviceToHostFp(new_xs_d,new_xs);
  deviceToHostFp(new_ys_d,new_ys);
  deviceToHostFp(new_zs_d,new_zs);
  fprintf(stderr, "array device to host transfer (seconds) %f\n", timer.seconds());
}
#endif //kokkos enabled

void push_scs(SellCSigma* scs, fp_t* xs, fp_t* ys, fp_t* zs,
    int* ptcl_to_elem, elemCoords& elems,
    fp_t distance, fp_t dx, fp_t dy, fp_t dz,
    fp_t* new_xs, fp_t* new_ys, fp_t* new_zs) {
  for (int i = 0; i < scs->num_chunks; ++i) {
    int index = scs->offsets[i];
    //loop over elements in the chunk
    while (index != scs->offsets[i + 1]) {
      //loop over rows of the chunk
      for (int j = 0; j < scs->C; ++j) {
        if (scs->id_list[index] != -1) {
          int id = scs->id_list[index];
          int e = i * scs->C + j;
          fp_t c = elems.x[e]   + elems.y[e]   + elems.z[e]   +
                   elems.x[e+1] + elems.y[e+1] + elems.z[e+1] +
                   elems.x[e+2] + elems.y[e+2] + elems.z[e+2] +
                   elems.x[e+3] + elems.y[e+3] + elems.z[e+3];
          c /= 4; // get schwifty!
          new_xs[id] = xs[id] + c * distance * dx;
          new_ys[id] = ys[id] + c * distance * dy;
          new_zs[id] = zs[id] + c * distance * dz;
        }
        ++index;
      } // end for
    } // end while
  }
}

#ifdef KOKKOS_ENABLED
void push_scs_kk(SellCSigma* scs, int np, fp_t* xs, fp_t* ys, fp_t* zs, fp_t distance, fp_t dx,
              fp_t dy, fp_t dz, fp_t* new_xs, fp_t* new_ys, fp_t* new_zs) {
  Kokkos::Timer timer;
  kkLidView offsets_d("offsets_d", scs->num_chunks+1);
  hostToDeviceLid(offsets_d, scs->offsets);

  kkLidView ids_d("ids_d", scs->offsets[scs->num_chunks]);
  hostToDeviceLid(ids_d, scs->id_list);

  kkLidView chunksz_d("chunksz_d", 1);
  hostToDeviceLid(chunksz_d, &scs->C);

  kkFpView xs_d("xs_d", np);
  hostToDeviceFp(xs_d, xs);

  kkFpView ys_d("ys_d", np);
  hostToDeviceFp(ys_d, ys);

  kkFpView zs_d("zs_d", np);
  hostToDeviceFp(zs_d, zs);

  kkFpView new_xs_d("new_xs_d", np);
  hostToDeviceFp(new_xs_d, new_xs);

  kkFpView new_ys_d("new_ys_d", np);
  hostToDeviceFp(new_ys_d, new_ys);

  kkFpView new_zs_d("new_zs_d", np);
  hostToDeviceFp(new_zs_d, new_zs);

  fp_t disp[4] = {distance,dx,dy,dz};
  kkFpView disp_d("direction_d", 4);
  hostToDeviceFp(disp_d, disp);
  fprintf(stderr, "kokkos scs host to device transfer (seconds) %f\n", timer.seconds());

  using Kokkos::TeamPolicy;
  using Kokkos::TeamThreadRange;
  using Kokkos::parallel_for;
  typedef Kokkos::TeamPolicy<> team_policy;
  typedef typename team_policy::member_type team_member;
  #if defined(KOKKOS_ENABLE_CXX11_DISPATCH_LAMBDA)
  const int league_size = scs->num_chunks;
  const int team_size = scs->C;
  const team_policy policy(league_size, team_size);

  double avgTime = 0;
  double max = 0;
  double min = 1000;
  for( int iter=0; iter<NUM_ITERATIONS; iter++) {
    timer.reset();
    parallel_for(policy, KOKKOS_LAMBDA(const team_member& thread) {
        const int i = thread.league_rank();
        for( int index = offsets_d(i); index != offsets_d(i+1); index+=chunksz_d(0) ) {
        parallel_for(TeamThreadRange(thread, chunksz_d(0)), [=] (int& j) {
          int id = ids_d(index+j);
          if (id != -1) {
          new_xs_d(id) = xs_d(id) + disp_d(0) * disp_d(1);
          new_ys_d(id) = ys_d(id) + disp_d(0) * disp_d(2);
          new_zs_d(id) = zs_d(id) + disp_d(0) * disp_d(3);
          }
          });
        }
    });
    double t = timer.seconds();
    avgTime+=t;
    if( t > max ) max = t;
    if( t < min ) min = t;
  }
  avgTime/=NUM_ITERATIONS;
  printTiming("scs push avg", np, avgTime);
  printTiming("scs push min", np, min);
  printTiming("scs push max", np, max);
  #endif

  timer.reset();
  deviceToHostFp(new_xs_d,new_xs);
  deviceToHostFp(new_ys_d,new_ys);
  deviceToHostFp(new_zs_d,new_zs);
  fprintf(stderr, "array device to host transfer (seconds) %f\n", timer.seconds());
}
#endif //kokkos enabled
