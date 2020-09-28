#ifndef CROSS_DIFFUSION_H
#define CROSS_DIFFUSION_H
#include "GitrmMesh.hpp"
#include "GitrmParticles.hpp"


#define PERPDIFF 1
inline void gitrm_cross_diffusion(PS* ptcls, const GitrmMesh& gm,
 const GitrmParticles& gp, double dt, const o::LOs& elm_ids, int debug=0) {
  auto pid_ps = ptcls->get<PTCL_ID>();
  auto pid_ps_global=ptcls->get<PTCL_ID_GLOBAL>();
  auto x_ps_d = ptcls->get<PTCL_POS>();
  auto xtgt_ps_d = ptcls->get<PTCL_NEXT_POS>();
  auto vel_ps_d = ptcls->get<PTCL_VEL>();
  auto charge_ps_d = ptcls->get<PTCL_CHARGE>();
  if(debug)
    printf("Cross field Diffusion\n");

  //Setting up of 2D magnetic field data
  const auto& bField_2d = gm.getBfield2d();
  const auto& bGridX = gm.getBfield2dGrid(1);
  const auto& bGridZ = gm.getBfield2dGrid(2);
  const int USEPERPDIFFUSION = 1;
  OMEGA_H_CHECK(PERPDIFF == 1);
  OMEGA_H_CHECK((USEPERPDIFFUSION == 1));
  const double diffusionCoefficient = gm.getPerpDiffusionCoeft();
  auto useConstantBField = gm.isUsingConstBField();
  auto use2dInputFields = USE2D_INPUTFIELDS;
  auto use3dField = USE3D_BFIELD;
  bool cylSymm = true;

  auto& mesh = gm.mesh;
  const auto coords = mesh.coords();
  const auto mesh2verts = mesh.ask_elem_verts();
  const auto BField = o::Reals(); //o::Reals(mesh.get_array<o::Real>(o::VERT, "BField"));

  const auto& testGitrPtclStepData = gp.testGitrPtclStepData;
  const auto testGDof = gp.testGitrStepDataDof;
  const auto testGNT = gp.testGitrStepDataNumTsteps;
  const auto iTimeStep = iTimePlusOne - 1;
  const auto diff_rnd1 = gp.testGitrCrossFieldDiffRndInd;
  auto& xfaces =gp.wallCollisionFaceIds;
  const int useGitrRnd = gp.useGitrRndNums;
  auto& rpool = gp.rand_pool;

  const bool useCudaRnd = gp.useCudaRnd;
  auto* cuStates =  gp.cudaRndStates;

  auto update_diffusion = PS_LAMBDA(const int& e, const int& pid, const bool& mask) {
    if(mask > 0  && elm_ids[pid] >= 0) {
      o::LO el            = elm_ids[pid];
      auto ptcl_global    = pid_ps_global(pid);
      auto ptcl           = pid_ps(pid);
      auto charge         = charge_ps_d(pid);
      auto fid            = xfaces[pid];
      if(!charge || fid >=0)
        return;
      auto posit          = p::makeVector3(pid, x_ps_d);
      auto posit_next     = p::makeVector3(pid, xtgt_ps_d);
      auto vel            = p::makeVector3(pid, vel_ps_d);
      auto bField         = o::zero_vector<3>();
      auto bField_plus    = o::zero_vector<3>();
      auto bField_deriv   = o::zero_vector<3>();
      o::Real phi_random;
      if (use2dInputFields || useConstantBField){
          p::interp2dVector_wgrid(bField_2d, bGridX, bGridZ, posit_next, bField, cylSymm);
      }
      else if (use3dField){
          auto bcc = o::zero_vector<4>();
          p::findBCCoordsInTet(coords, mesh2verts, posit_next, el, bcc);
          p::interpolate3dFieldTet(mesh2verts, BField, el, bcc, bField);
      }
      auto perpVector         = o::zero_vector<3>();
      auto b_mag  = Omega_h::norm(bField);
      Omega_h::Vector<3> b_unit = bField/b_mag;
      double r3 = 0.0;
      //o::Real r4 = 0.75;
      double step=sqrt(6*diffusionCoefficient*dt);

#if PERPDIFF >1
      if(USEPERPDIFFUSION>1){
        double plus_minus1=floor(r4+0.5)*2-1.0;
        double h=0.001;
        auto posit_next_plus  = o::zero_vector<3>();
        posit_next_plus=posit_next_plus + h*bField;
        o::Real theta_plus = atan2(posit_next_plus[1], posit_next_plus[0]);
        bField_plus[0] = cos(theta_plus)*bField_radial[0] - sin(theta_plus)*bField_radial[1];
        bField_plus[1] = sin(theta_plus)*bField_radial[0] + cos(theta_plus)*bField_radial[1];
        bField_plus[2] = bField_radial[2];
        auto b_plus_mag  = Omega_h::norm(bField_plus);
        bField_deriv=(bField_plus-bField)/h;
        auto denom = Omega_h::norm(bField_deriv);
        double R = 1.0e4;
        if(( abs(denom) > 1e-10) & ( abs(denom) < 1e10) )
        {
            R = b_mag/denom;
        }
        double initial_guess_theta = 3.14159265359*0.5;
        double eps = 0.01;
        double error = 2.0;
        double s = step;
        double drand = r3;
        double theta0 = initial_guess_theta;
        double theta1 = 0.0;
        double f = 0.0;
        double f_prime = 0.0;
        int nloops = 0;
        if(R > 1.0e-4){

          while ((error > eps)&(nloops<10)){

            f = (2*R*theta0-s*sin(theta0))/(2*3.14159265359*R) - drand;
            f_prime = (2*R-s*cos(theta0))/(2*3.14159265359*R);
            theta1 = theta0 - f/f_prime;
            error = abs(theta1-theta0);
            theta0=theta1;
            nloops++;
          }

          if(nloops > 9){
            theta0 = 2*3.14159265359*drand;
          }
        }
        else{
          R = 1.0e-4;
          theta0 = 2*3.14159265359*drand;
        }

        //TO USE CONDITIONAL STATEMENT
        if(plus_minus1 < 0){
          theta0 = 2*3.14159265359-theta0;
        }

        perpVector              = bField_deriv/Omega_h::norm(bField_deriv);
        auto y_dir              = o::zero_vector<3>();
        y_dir                   = Omega_h::cross(bField, bField_deriv);
        double x_comp = s*cos(theta0);
        double y_comp = s*sin(theta0);
        auto transform         = o::zero_vector<3>();
        transform              = x_comp*perpVector+y_comp*y_dir;
        if (abs(denom) > 1.0e-8){

          xtgt_ps_d(pid,0)=posit_next[0]+transform[0];
          xtgt_ps_d(pid,1)=posit_next[1]+transform[1];
          xtgt_ps_d(pid,2)=posit_next[2]+transform[2];
          //exit(0);
        }
      }
#endif //PERPDIFFUSION > 1

      if( USEPERPDIFFUSION==1){

        if(useGitrRnd){
          r3  = testGitrPtclStepData[ptcl_global*testGNT*testGDof + iTimeStep*testGDof + diff_rnd1];
        } else if (useCudaRnd) {
          auto localState = cuStates[ptcl_global];
          r3 = curand_uniform(&localState);
          cuStates[ptcl_global] = localState;
        } else{
          auto rnd = rpool.get_state();
          r3 = rnd.drand();
          rpool.free_state(rnd);
        }

        phi_random = 2*3.14159265*r3;
        perpVector[0] =  cos(phi_random);
        perpVector[1] =  sin(phi_random);
        perpVector[2] = (-perpVector[0]*b_unit[0] - perpVector[1]*b_unit[1])/b_unit[2];

        if (b_unit[2] == 0) {
          perpVector[2] = perpVector[1];
          perpVector[1] = (-perpVector[0]*b_unit[0] - perpVector[2]*b_unit[2])/b_unit[1];
        }
        if ((p::almost_equal(b_unit[0], 1) && p::almost_equal(b_unit[1], 0) &&
            p::almost_equal(b_unit[2], 0)) || (p::almost_equal(b_unit[0], -1) &&
            p::almost_equal(b_unit[1], 0) && p::almost_equal(b_unit[2], 0))) {
          perpVector[2] = perpVector[0];
          perpVector[0] = 0;
          perpVector[1] = sin(phi_random);
        }
        else if ((p::almost_equal(b_unit[0], 0) && p::almost_equal(b_unit[1], 1) &&
            p::almost_equal(b_unit[2], 0)) || (p::almost_equal(b_unit[0], 0) &&
            p::almost_equal(b_unit[1], -1) && p::almost_equal(b_unit[2], 0))) {
          perpVector[1] = 0.0;
        }
        else if ((p::almost_equal(b_unit[0], 0) && p::almost_equal(b_unit[1], 0) &&
          p::almost_equal(b_unit[2], 1)) || (p::almost_equal(b_unit[0], 0) &&
          p::almost_equal(b_unit[1], 0) && p::almost_equal(b_unit[2], -1.0))) {
          perpVector[2] = 0;
        }
        if (debug > 2)
          printf("Diff: ptcl %d perpVec %.15f %0.15f %0.15f \n", ptcl, perpVector[0],
              perpVector[1],perpVector[2]);
        perpVector = perpVector/Omega_h::norm(perpVector);
        xtgt_ps_d(pid,0) = posit_next[0]+step*perpVector[0];
        xtgt_ps_d(pid,1) = posit_next[1]+step*perpVector[1];
        xtgt_ps_d(pid,2) = posit_next[2]+step*perpVector[2];

      }
      if (debug > 1) {
        printf("Diffusion: ptcl %d tstep %d pos %.15f %.15f %.15f => %.15f %.15f %.15f\n",
          ptcl, iTimeStep, posit_next[0], posit_next[1], posit_next[2], xtgt_ps_d(pid,0),
          xtgt_ps_d(pid,1), xtgt_ps_d(pid,2));
      }
      if(debug > 2) {
        printf("Diff: ptcl %d perpVector-norm : %.15f %0.15f %0.15f phi_rand %.15f \n", ptcl,
            perpVector[0],perpVector[1],perpVector[2], phi_random);
        printf("Diff: gitrRnd ptcl %d tstep %d r3 %g numbers %d  %d  %d  %d  %d\n",ptcl, iTimeStep,
            r3, testGNT, testGDof, testGDof, diff_rnd1);
        printf("Diff: coefficient and ptcl %d : cft %.15f step %.15f \n", ptcl, diffusionCoefficient, step);
      }
    }
  };

  p::parallel_for(ptcls, update_diffusion, "diffusion_kernel");
}
#endif