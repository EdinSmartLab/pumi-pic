#ifndef GITRM_SURFACE_MODEL_HPP
#define GITRM_SURFACE_MODEL_HPP

#include <cstdarg>
#include <pumipic_adjacency.hpp>
#include "GitrmParticles.hpp" 
#include "GitrmMesh.hpp"

namespace o = Omega_h;
namespace p = pumipic;

namespace gitrm {
//TODO get from config
const int BOUNDARY_ATOM_Z = surfaceAndMaterialModelZ;
const o::Real DELTA_SHIFT_BDRY_REFL = 1.0e-4; 
}

class GitrmSurfaceModel {
public:
  GitrmSurfaceModel(GitrmMesh& gm, std::string ncFile);
  void getConfigData(std::string ncFile);
  void initSurfaceModelData(std::string ncFile, bool debug=false);
  void prepareSurfaceModelData();
  void setFaceId2SurfaceIdMap();
  void writeSurfaceDataFile(std::string fileName="surface.nc");

  void getSurfaceModelData(const std::string fileName,
   const std::string dataName, const std::vector<std::string>& shapeNames,
   const std::vector<int> shapeInds, o::Reals& data, int* size=nullptr);

  GitrmMesh& gm;
  o::Mesh& mesh;
  std::string ncFile;
  int numDetectorSurfaceFaces = 0;
  o::HostWrite<o::LO> surfaceAndMaterialModelIds;
  int numSurfMaterialFaces = 0;
  o::LOs surfaceAndMaterialOrderedIds;
  int nDetectSurfaces = 0;
  o::LOs detectorSurfaceOrderedIds;
  o::Reals bdryFaceMaterialZs;

  //from NC file
  std::string fileString{};
  std::string nEnSputtRefCoeffStr{};
  std::string nAngSputtRefCoeffStr{};
  std::string nEnSputtRefDistInStr{};
  std::string nAngSputtRefDistInStr{};
  std::string nEnSputtRefDistOutStr{};
  std::string nEnSputtRefDistOutRefStr{};
  std::string nAngSputtRefDistOutStr{};
  std::string enSputtRefCoeffStr{};
  std::string angSputtRefCoeffStr{};
  std::string enSputtRefDistInStr{};
  std::string angSputtRefDistInStr{};
  std::string enSputtRefDistOutStr{};
  std::string enSputtRefDistOutRefStr{};
  std::string angPhiSputtRefDistOutStr{};
  std::string angThetaSputtRefDistOutStr{};
  std::string sputtYldStr{};
  std::string reflYldStr{};
  std::string enDistYStr{};
  std::string angPhiDistYStr{};
  std::string angThetaDistYStr{};
  std::string enDistRStr{};
  std::string angPhiDistRStr{};
  std::string angThetaDistRStr{};

  int nEnSputtRefCoeff = 0; //"nE"
  int nAngSputtRefCoeff = 0; // "nA"
  int nEnSputtRefDistIn = 0; //"nE"
  int nAngSputtRefDistIn = 0; // "nA"
  int nEnSputtRefDistOut = 0; //"nEdistBins"
  int nEnSputtRefDistOutRef = 0; //"nEdistBinsRef"
  int nAngSputtRefDistOut = 0; // "nAdistBins"

  //input surface model data
  // shapes are given as comments
  o::Reals enSputtRefCoeff; //nEnSputtRefCoeff
  o::Reals angSputtRefCoeff; // nAngSputtRefCoeff
  o::Reals enSputtRefDistIn; //nEnSputtRefDistIn
  o::Reals angSputtRefDistIn; //nAngSputtRefDistIn
  o::Reals enSputtRefDistOut; // nEnSputtRefDistOut
  o::Reals enSputtRefDistOutRef; //nEnSputtRefDistOutRef
  o::Reals angPhiSputtRefDistOut; //nAngSputtRefDistOut
  o::Reals angThetaSputtRefDistOut; //nAngSputtRefDistOut
  o::Reals sputtYld; // nEnSputtRefCoeff X nAngSputtRefCoeff
  o::Reals reflYld; //  nEnSputtRefCoeff X nAngSputtRefCoeff
  // nEnSputtRefCoeff X nAngSputtRefCoeff X nEnSputtRefDistOut
  o::Reals enDist_Y;
  // nEnSputtRefCoeff X nAngSputtRefCoeff X nEnSputtRefDistOutRef 
  o::Reals enDist_R;
  //nEnSputtRefCoeff X nAngSputtRefCoeff X nAngSputtRefDistOut
  o::Reals angPhiDist_Y;
  o::Reals angThetaDist_Y; // ""
  o::Reals angPhiDist_R; // ""
  o::Reals angThetaDist_R; // ""
  o::Reals enLogSputtRefCoef; // nEnSputtRefCoeff
  o::Reals enLogSputtRefDistIn; //nEnSputtRefDistIn
  o::Reals energyDistGrid01; //nEnSputtRefDistOut
  o::Reals energyDistGrid01Ref; //nEnSputtRefDistOutRef
  o::Reals angleDistGrid01; //nAngSputtRefDistOut
  o::Reals enDist_CDF_Y_regrid; //EDist_CDF_Y_regrid(nDistE_surfaceModel)
  o::Reals angPhiDist_CDF_Y_regrid;  //AphiDist_CDF_R_regrid(nDistA_surfaceModel)
  o::Reals angPhiDist_CDF_R_regrid;  //APhiDist_CDF_R_regrid(nDistA_surfaceModel)
  o::Reals enDist_CDF_R_regrid;  //EDist_CDF_R_regrid(nDistE_surfaceModelRef)
  o::Reals angThetaDist_CDF_R_regrid; //
  o::Reals angThetaDist_CDF_Y_regrid;//
  int nDistEsurfaceModel = 0;
  int nDistEsurfaceModelRef = 0;
  int nDistAsurfaceModel = 0;
  int nEnDist = 0;
  double en0Dist = 0;
  double enDist = 0;
  int nAngDist = 0; 
  double ang0Dist = 0;
  double angDist = 0; 
  double dEdist = 0;
  double dAdist = 0;
  //size/bdry_face in comments. Distribute data upon partitioning.
  o::Write<o::Real> energyDistribution; //9k/detFace
  o::Write<o::Real> sputtDistribution;
  o::Write<o::Real> reflDistribution;

  void regrid2dCDF(const int nX, const int nY, const int nZ, 
    const o::HostWrite<o::Real>& xGrid, const int nNew, const o::Real maxNew, 
    const o::HostWrite<o::Real>& cdf, o::HostWrite<o::Real>& cdf_regrid);

  void make2dCDF(const int nX, const int nY, const int nZ, 
    const o::HostWrite<o::Real>& distribution, o::HostWrite<o::Real>& cdf);

  o::Real interp1dUnstructured(const o::Real samplePoint, const int nx, 
    const o::Real max_x, const o::Real* data, int& lowInd);
};


OMEGA_H_DEVICE o::Real screeningLength(const o::Real projectileZ, 
    const o::Real targetZ) {
  o::Real bohrRadius = 5.29177e-11; //TODO
  return 0.885341*bohrRadius*pow(pow(projectileZ,(2.0/3.0)) + 
      pow(targetZ,(2.0/3.0)),(-1.0/2.0));
}

OMEGA_H_DEVICE o::Real stoppingPower (const o::Vector<3>& vel, const o::Real targetM, 
  const o::Real targetZ, const o::Real screenLength) {
  o::Real elCharge = gitrm::ELECTRON_CHARGE;
  o::Real ke2 = 14.4e-10; //TODO
  o::Real amu = gitrm::PTCL_AMU;
  o::Real atomZ = gitrm::PARTICLE_Z;
  auto protonMass = gitrm::PROTON_MASS;
  o::Real E0 = 0.5*amu*protonMass *1.0/elCharge * o::norm(vel);
  o::Real reducedEnergy = E0*(targetM/(amu+targetM))* (screenLength/(atomZ*targetZ*ke2));
  o::Real stopPower = 0.5*log(1.0 + 1.2288*reducedEnergy)/(reducedEnergy +
          0.1728*sqrt(reducedEnergy) + 0.008*pow(reducedEnergy, 0.1504));
  return stopPower;
}

//not used ? 
inline void surfaceErosion(PS* ptcls, o::Write<o::Real>& erosionData) {
  const auto psCapacity = ptcls->capacity();
  int atomZ = gitrm::PARTICLE_Z;
  auto amu = gitrm::PTCL_AMU;
  auto elCharge = gitrm::ELECTRON_CHARGE; //1.60217662e-19
  auto protonMass = gitrm::PROTON_MASS; //1.6737236e-27
  //TODO get from config
  o::Real q = 18.6006;
  o::Real lambda = 2.2697;
  o::Real mu = 3.1273;
  o::Real Eth = 24.9885;
  o::Real targetZ = 74.0;
  o::Real targetM = 183.84;

  auto pid_ps = ptcls->get<PTCL_ID>();
  auto vel_ps = ptcls->get<PTCL_VEL>();
  auto lamb = PS_LAMBDA(const int& e, const int& pid, const int& mask) {
    auto ptcl = pid_ps(pid);
    auto screenLength = screeningLength(atomZ, targetZ);
    auto vel = p::makeVector3(pid, vel_ps);
    o::Real stopPower = stoppingPower(vel, targetM, targetZ, screenLength);
    o::Real E0 = 0.5*amu*protonMass* 1/elCharge * o::norm(vel);
    o::Real term = pow((E0/Eth - 1), mu);
    o::Real Y0 = q*stopPower*term/(lambda + term);
    erosionData[pid] = Y0;
  };
  p::parallel_for(ptcls, lamb, "surfaceErosion");
}

//Note, elem_ids indexed by pids of ps, not ptcl. Don't rebuild after search_mesh 
inline void gitrm_surfaceReflection(PS* ptcls, GitrmSurfaceModel& sm,
    GitrmParticles& gp, const GitrmMesh& gm, int debug=0 ) {
  if(debug)
    printf("Surface model \n");

  // if no ptcl hit boundary. NOTE the default should be < 0.
  if(o::get_max(gp.wallCollisionFaceIds) < 0)
    return;
  auto& rpool = gp.rand_pool;

  bool useGitrRnd = USE_GITR_RND_NUMS;
  if(!gp.ranSurfaceReflection)
    gp.ranSurfaceReflection = true;
  const auto& testGitrPtclStepData = gp.testGitrPtclStepData;
  const auto testGDof = gp.testGitrStepDataDof;
  const auto testGNT = gp.testGitrStepDataNumTsteps;
  const auto testGitrReflInd = gp.testGitrReflectionRndInd;
  const auto iTimeStep = iTimePlusOne - 1;
  if(useGitrRnd)
    OMEGA_H_CHECK(gp.testGitrOptSurfaceModel);
  else
    OMEGA_H_CHECK(!gp.testGitrOptSurfaceModel);

  auto bdrys = gm.bdryFaceOrderedIds;
  o::Real pi = o::PI;
  o::Real shiftRefl = gitrm::DELTA_SHIFT_BDRY_REFL;
  auto amu = gitrm::PTCL_AMU;
  auto elCharge = gitrm::ELECTRON_CHARGE;
  auto protonMass = gitrm::PROTON_MASS;
  auto mesh = sm.mesh;
  const auto coords = mesh.coords();
  const auto face_verts = mesh.ask_verts_of(2);
  const auto f2r_ptr = mesh.ask_up(o::FACE, o::REGION).a2ab;
  const auto f2r_elem = mesh.ask_up(o::FACE, o::REGION).ab2b;
  const auto side_is_exposed = mark_exposed_sides(&mesh);
  const auto mesh2verts = mesh.ask_elem_verts();
  const auto down_r2fs = mesh.ask_down(3, 2).ab2b;
  //input data
  const auto nEnSputtRefCoeff = sm.nEnSputtRefCoeff; // nE_sputtRefCoeff
  const auto nAngSputtRefCoeff = sm.nAngSputtRefCoeff; // nA_sputtRefCoeff 
  const auto& angSputtRefCoeff = sm.angSputtRefCoeff;  // A_sputtRefCoeff
  const auto& enLogSputtRefCoef = sm.enLogSputtRefCoef; //Elog_sputtRefCoeff
  const auto& sputtYld = sm.sputtYld; //spyl_surfaceModel 
  const auto& reflYld = sm.reflYld; // rfyl_surfaceModel
  const auto nEnSputtRefDistOut = sm.nEnSputtRefDistOut; // nE_sputtRefDistOut
  const auto nEnSputtRefDistOutRef = sm.nEnSputtRefDistOutRef; //nE_sputtRefDistOutRef
  const auto nAngSputtRefDistOut = sm.nAngSputtRefDistOut; //nA_sputtRefDistOut
  const auto nEnSputtRefDistIn = sm.nEnSputtRefDistIn; // nE_sputtRefDistIn
  const auto nAngSputtRefDistIn = sm.nAngSputtRefDistIn; // nA_sputtRefDistIn
  const auto& enSputtRefDistIn = sm.enSputtRefDistIn; // E_sputtRefDistIn
  const auto& angSputtRefDistIn = sm.angSputtRefDistIn; // A_sputtRefDistIn 
  const auto& enSputtRefDistOut = sm.enSputtRefDistOut;// E_sputtRefDistOut
  const auto& enSputtRefDistOutRef = sm.enSputtRefDistOutRef; //E_sputtRefDistOutRef
  const auto& angPhiSputtRefDistOut = sm.angPhiSputtRefDistOut; // A_sputtRefDistOut
  const auto& energyDistGrid01 = sm.energyDistGrid01; //energyDistGrid01
  const auto& energyDistGrid01Ref = sm.energyDistGrid01Ref; // energyDistGrid01Ref 
  const auto& angleDistGrid01 = sm.angleDistGrid01; // angleDistGrid01
  const auto& enLogSputtRefDistIn = sm.enLogSputtRefDistIn;
  const auto& enDist_CDF_Y_regrid = sm.enDist_CDF_Y_regrid; //EDist_CDF_Y_regrid
  const auto& angPhiDist_CDF_Y_regrid = sm.angPhiDist_CDF_Y_regrid; //ADist_CDF_Y_regrid
  const auto& enDist_CDF_R_regrid = sm.enDist_CDF_R_regrid;  //EDist_CDF_R_regrid
  const auto& angPhiDist_CDF_R_regrid = sm.angPhiDist_CDF_R_regrid; //ADist_CDF_R_regrid

  const auto nEnDist = sm.nEnDist; //nEdist
  const auto en0Dist = sm.en0Dist; // E0dist
  const auto enDist = sm.enDist; // Edist
  const auto nAngDist = sm.nAngDist; // nAdist
  const auto ang0Dist = sm.ang0Dist; // A0dist
  //const auto angDist = sm.angDist; // Adist
  const auto dEdist = sm.dEdist;
  const auto dAdist = sm.dAdist; 
  //data collection
  //auto surfaces = mesh.get_array<o::LO>(o::FACE, "SurfaceIndex");
  auto sumPtclStrike = o::deep_copy(
    mesh.get_array<o::Int>(o::FACE, "SumParticlesStrike"),"sumPtclStrike");
  auto sputtYldCount = o::deep_copy(
    mesh.get_array<o::Int>(o::FACE, "SputtYldCount"), "sputtYldCount"); 
  auto sumWtStrike = o::deep_copy(
    mesh.get_array<o::Real>(o::FACE, "SumWeightStrike"), "sumWtStrike");
  auto grossDeposition = o::deep_copy(
    mesh.get_array<o::Real>(o::FACE, "GrossDeposition"), "grossDeposition");
  auto grossErosion = o::deep_copy(
    mesh.get_array<o::Real>(o::FACE, "GrossErosion"), "grossErosion");
  auto aveSputtYld = o::deep_copy(
    mesh.get_array<o::Real>(o::FACE, "AveSputtYld"), "aveSputtYld");  
  const auto& xpoints = gp.wallCollisionPts; //idexed by pid of scs
  const auto& xfaces = gp.wallCollisionFaceIds;
  auto energyDist = sm.energyDistribution;
  auto energyDist_size = energyDist.size();
  auto sputtDist = sm.sputtDistribution;
  auto reflDist = sm.reflDistribution;
  auto surfaceIds = sm.surfaceAndMaterialOrderedIds;
  auto materials = sm.bdryFaceMaterialZs;

  auto pid_ps = ptcls->get<PTCL_ID>();
  auto next_pos_ps = ptcls->get<PTCL_NEXT_POS>();
  auto pos_ps = ptcls->get<PTCL_POS>();
  auto vel_ps = ptcls->get<PTCL_VEL>();
  auto ps_weight = ptcls->get<PTCL_WEIGHT>();
  auto ps_charge = ptcls->get<PTCL_CHARGE>();
  auto scs_hitNum = ptcls->get<PTCL_HIT_NUM>();
  auto ps_newVelMag = ptcls->get<PTCL_VMAG_NEW>();
  auto lamb = PS_LAMBDA(const int& elem, const int& pid, const int& mask) {
    if(mask >0  && xfaces[pid] >= 0) {
      auto elemId = elem;
      auto ptcl = pid_ps(pid);
      auto fid = xfaces[pid];
      if(debug>1 && side_is_exposed[fid])
        printf(" surf0 timestep %d ptcl %d fid %d\n", iTimeStep, ptcl, fid);
      OMEGA_H_CHECK(side_is_exposed[fid]);
      auto weight = ps_weight(pid);
      auto newWeight = weight; 
      auto surfId = surfaceIds[fid]; //surfaces[fid]; //ids 0..
      auto gridId = fid;
      //TODO pass elem_ids if it is valid or get by this method
      auto pelem = p::elem_id_of_bdry_face_of_tet(fid, f2r_ptr, f2r_elem);
      if(elemId != pelem)
        elemId = pelem;
      auto vel = p::makeVector3(pid, vel_ps );
      auto xpoint = o::zero_vector<3>();
      for(o::LO i=0; i<3; ++i)
        xpoint[i] = xpoints[pid*3+i];
      //firstColl(pid) = 1; //scs
      auto magPath = o::norm(vel);
      auto E0 = 0.5*amu*protonMass*(magPath*magPath)/elCharge;

      if(debug>1) {
        auto face1 = p::get_face_coords_of_tet(face_verts, coords, fid);
        printf(" surf1 faceid %d face %g %g %g : %g %g %g : %g %g %g\n", 
          fid, face1[0][0], face1[0][1], face1[0][2],
          face1[1][0], face1[1][1], face1[1][2], face1[2][0], face1[2][1], face1[2][2]);
      }
       
      if(debug>1)
        printf(" surf2 timestep %d ptcl %d xpoint= pos  %g %g  %g elemId %d "
          "vel %g %g %g amu %g  weight %g mag %g E0 %g nEnDist %d\n", 
          iTimeStep, ptcl, xpoint[0],xpoint[1], xpoint[2], 
          elemId, vel[0], vel[1], vel[2], amu, weight, magPath, E0, nEnDist);

      OMEGA_H_CHECK(nEnDist > 0);
      if(E0 > enDist) //1000 
        E0 = enDist - enDist/nEnDist; // 990;   
      //boundary normal points outwards
      auto surfNormOut = p::face_normal_of_tet(fid, elemId, coords, mesh2verts, 
        face_verts, down_r2fs); 
      auto surfNormIn = -surfNormOut;
      //debug>1 only
      auto bFaceNorm = p::bdry_face_normal_of_tet(fid, coords, face_verts);
      auto abc = p::get_face_coords_of_tet(face_verts, coords, fid);
       if(debug>1)
        printf(" surf3 timestep %d ptcl %d surfNormOut %g %g %g bfaceNorm "
          "%g %g %g bdry %g %g %g : %g %g %g : %g %g %g \n",iTimeStep, ptcl,
          surfNormOut[0], surfNormOut[1], surfNormOut[2], bFaceNorm[0], bFaceNorm[1],
          bFaceNorm[2], abc[0][0], abc[0][1], abc[0][2], abc[1][0], abc[1][1], 
          abc[1][2], abc[2][0], abc[2][1], abc[2][2]);
      //end debug>1

      auto magSurfNorm = o::norm(surfNormIn);
      auto normVel = o::normalize(vel);
      auto ptclProj = o::inner_product(normVel, surfNormIn);
      auto thetaImpact = acos(ptclProj);
      if(thetaImpact > pi*0.5)
         thetaImpact = abs(thetaImpact - pi);
      thetaImpact = thetaImpact*180.0/pi;
      if(thetaImpact < 0) 
        thetaImpact = 0;
      if(o::are_close(E0, 0))
        thetaImpact = 0;
      o::Real Y0 = 0;
      o::Real R0 = 0;

      auto materialZ = materials[fid];
      if(debug>1)
        printf(" surf3+ timestep %d ptcl %d materialZ %g normVel %g %g %g "
          "surfNormOut %g %g %g E0 %g thetaImpact %g : sizes %d %d %d \n", iTimeStep, 
          ptcl, materialZ, normVel[0], normVel[1],normVel[2], surfNormOut[0], surfNormOut[1], 
          surfNormOut[2], E0, thetaImpact, sputtYld.size(),angSputtRefCoeff.size(), 
          enLogSputtRefCoef.size());

      if(materialZ > 0) {
        Y0 = p::interpolate2d_wgrid(sputtYld, angSputtRefCoeff, enLogSputtRefCoef,
           nAngSputtRefCoeff, nEnSputtRefCoeff, thetaImpact, log10(E0), true,1,0);
        R0 = p::interpolate2d_wgrid(reflYld, angSputtRefCoeff, enLogSputtRefCoef,
          nAngSputtRefCoeff, nEnSputtRefCoeff, thetaImpact, log10(E0), true,1,0);
      }
      if(debug>1)
        printf(" surf4 timestep %d ptcl %d interpolated Y0 %g R0 %g\n", iTimeStep,
         ptcl, Y0, R0);
      auto totalYR = Y0 + R0;
      //particle either reflects or deposits
      auto sputtProb = (totalYR >0) ? Y0/totalYR : 0;
      int didReflect = 0;

      double rand7 = 0, rand8 = 0, rand9 = 0, rand10 = 0;
      if(useGitrRnd) {
        auto beg = ptcl*testGNT*testGDof + iTimeStep*testGDof + testGitrReflInd;
        rand7 = testGitrPtclStepData[beg];
        rand8 = testGitrPtclStepData[beg+1];
        rand9 = testGitrPtclStepData[beg+2];
        rand10 = testGitrPtclStepData[beg+3];
      } else {
        auto rnd = rpool.get_state();
        rand7 = rnd.drand();
        rand8 = rnd.drand();
        rand9 = rnd.drand();
        rand10 = rnd.drand();
        rpool.free_state(rnd);
      }
      o::Real eInterpVal = 0;
      o::Real aInterpVal = 0;
      o::Real addGrossDep = 0; 
      o::Real addGrossEros = 0;
      o::Real addAveSput = 0;
      o::Int addSpYCount = 0;
      o::Real addSumWtStk = 0;
      o::Int addSumPtclStk = 0;

      if(debug>1)
        printf(" surf5 timestep %d ptcl %d totalYR %g surfId %d gridId %d "
          "sputtProb %g rand7 %g bdry %d \n", 
          iTimeStep, ptcl, totalYR, surfId, gridId, sputtProb, rand7, bdrys[fid]);
      if(totalYR > 0) {
        newWeight = weight*totalYR;
        if(rand7 > sputtProb) { //reflect
          if(debug>1)
            printf(" surf51 timestep %d ptcl %d nA %d nASOut %d nESIn %d "
              " nE %d nASIn %d nESIn %d rand8 %g thetaImpact %g log10(E0) %g \n", 
              iTimeStep, ptcl, nAngSputtRefDistOut, nAngSputtRefDistIn, 
              nEnSputtRefDistIn, nEnSputtRefDistOutRef, nAngSputtRefDistIn, 
              nEnSputtRefDistIn, rand8, thetaImpact, log10(E0)); 

          didReflect = 1;
          aInterpVal = p::interpolate3d_field(rand8, thetaImpact, log10(E0),
             nAngSputtRefDistOut, nAngSputtRefDistIn, nEnSputtRefDistIn, 
             angleDistGrid01, angSputtRefDistIn, enLogSputtRefDistIn, 
             angPhiDist_CDF_R_regrid);
          eInterpVal = p::interpolate3d_field(rand9, thetaImpact, log10(E0),
             nEnSputtRefDistOutRef, nAngSputtRefDistIn, nEnSputtRefDistIn, 
             energyDistGrid01Ref, angSputtRefDistIn, enLogSputtRefDistIn, 
             enDist_CDF_R_regrid);
          if(debug>1)
            printf(" surf6 reflects timestep %d ptcl %d  weight %g newWeight %g "
              "totalYR %g aInterpVal %g  eInterpVal %g \n", 
              iTimeStep, ptcl, weight, newWeight, totalYR, aInterpVal, eInterpVal);
          int eDistInd = floor((eInterpVal-en0Dist)/dEdist);
          int aDistInd = floor((aInterpVal-ang0Dist)/dAdist);
          if(surfId >=0 && eDistInd >= 0 && eDistInd < nEnDist && 
             aDistInd >= 0 && aDistInd < nAngDist) {
            auto idx = surfId*nEnDist*nAngDist + eDistInd*nAngDist + aDistInd;
            if(debug>1)
              printf(" surf61 timestep %d ptcl %d reflDist @ %d newWeight %g"
                " prev %g \n", iTimeStep, ptcl, idx, newWeight, reflDist[idx]);
            Kokkos::atomic_fetch_add(&(reflDist[idx]), newWeight);
          }
 
          if(surfId >= 0) { //id 0..
            if(debug>1)
              printf(" surf7 surfId %d GrossDep+ %g addGrossDep %g  \n", 
                  surfId, weight*(1.0-R0), addGrossDep);
            addGrossDep += weight*(1.0-R0);
          }
        } else {//sputters
          if(debug>1) {
            printf(" surf8 timestep %d ptcl %d interpolate3d E0 %g\n", iTimeStep, ptcl, E0);
            printf("rand81 %g thetaImpact %g log10(E0) %g nAngSputtRefDistOut %d "
              "nAngSputtRefDistIn %d nEnSputtRefDistIn %d\n", rand8, thetaImpact, log10(E0), 
              nAngSputtRefDistOut, nAngSputtRefDistIn, nEnSputtRefDistIn);              
          }
          //TODO merge with the above same 
          aInterpVal = p::interpolate3d_field(rand8, thetaImpact, log10(E0),
            nAngSputtRefDistOut, nAngSputtRefDistIn, nEnSputtRefDistIn,
            angleDistGrid01, angSputtRefDistIn, enLogSputtRefDistIn, angPhiDist_CDF_Y_regrid);
          eInterpVal = p::interpolate3d_field(rand9,thetaImpact, log10(E0),
            nEnSputtRefDistOut, nAngSputtRefDistIn, nEnSputtRefDistIn,
            energyDistGrid01, angSputtRefDistIn, enLogSputtRefDistIn, enDist_CDF_Y_regrid);
          if(debug>1)
            printf(" surf81 timestep %d ptcl %d nA %d nASOut %d nESIn %d "
              " nE %d nASIn %d nESIn %d interp3d eInterpVal %g aInterpVal %g \n", 
              iTimeStep, ptcl, nAngSputtRefDistOut, nAngSputtRefDistIn, nEnSputtRefDistIn,
              nEnSputtRefDistOutRef, nAngSputtRefDistIn, nEnSputtRefDistIn, eInterpVal, aInterpVal); 
          if(debug>1)
             printf(" surf8 sputters timestep %d ptcl %d weight %g newWeight %g "
              " sputtProb %g aInterpVal %g eInterpVal %g\n", iTimeStep, ptcl, weight,
              newWeight, sputtProb, aInterpVal, eInterpVal);

          int eDistInd = floor((eInterpVal-en0Dist)/dEdist);
          int aDistInd = floor((aInterpVal-ang0Dist)/dAdist);
          if(surfId >= 0 && eDistInd >= 0 && eDistInd < nEnDist && 
             aDistInd >= 0 && aDistInd < nAngDist) {
            auto idx = surfId*nEnDist*nAngDist + eDistInd*nAngDist + aDistInd;
            if(debug>1)
              printf(" surf9 timestep %d ptcl %d sputtDist idx  %d newWeight %g prev %g %g \n", 
                iTimeStep, ptcl, idx, newWeight, sputtDist[idx], sputtDist[10]);
            Kokkos::atomic_fetch_add(&(sputtDist[idx]), newWeight);
          }
          if(o::are_close(sputtProb, 0))
            newWeight = 0;
          if(surfId >= 0) {
            addGrossDep += weight*(1.0-R0);
            addGrossEros += newWeight;
            addAveSput += Y0;
            if(weight > 0) {
              addSpYCount += 1;
            }
          }
          if(debug>1)
            printf(" surf10 surfId %d timestep %d ptcl %d newWeight %g GrossDep %g "
              " GrossEros %g AveSput %g  SpYCount %d \n", surfId, iTimeStep,
              ptcl, newWeight, addGrossDep, addGrossEros, addAveSput, addSpYCount);
        }
      } else { // totalYR
        newWeight = 0;
        scs_hitNum(pid) = 2;
        double grossDep_ = 0;
        if(surfId >= 0) {
          addGrossDep += weight;  //TODO Dep ?
          grossDep_ = weight;
        }
        if(debug>1)
          printf(" surf11 totalYR timestep %d ptcl %d weight %g surfId %d "
            " newWeight %g grossDep+ %g totalGrossDep %g \n", iTimeStep, ptcl,
            weight,surfId, newWeight, grossDep_, addGrossDep);
      }

      if(eInterpVal <= 0) {
        if(debug>1)
          printf(" surf11+ eInterpVal <= 0 timestep %d ptcl %d didReflect %d "
            " weight %g \n", iTimeStep, ptcl, didReflect, weight );
        newWeight = 0;
        scs_hitNum(pid) = 2;
        if(surfId >= 0 && didReflect) {
          addGrossDep += weight; //TODO Dep ?
        }
      }
      if(surfId >=0) {
        addSumWtStk += weight;
        addSumPtclStk += 1;

        int eDistInd = floor((E0-en0Dist)/dEdist);
        int aDistInd = floor((thetaImpact-ang0Dist)/dAdist);
        if(surfId >= 0 && eDistInd >= 0 && eDistInd < nEnDist && 
            aDistInd >= 0 && aDistInd < nAngDist) {
          auto idx = surfId*nEnDist*nAngDist + eDistInd*nAngDist + aDistInd;

          if(debug>1)
            printf("surf12 timestep %d ptcl %d surfId %d nEnDist %d nAngDist %d en0Dist %g enDist %g "
            "ang0Dist %g dEdist %g dAdist %g eDistInd %d aDistInd %d idx %d energyDist_size %d \n", 
            iTimeStep, ptcl, surfId, nEnDist, nAngDist,  en0Dist, enDist, ang0Dist,  dEdist, 
            dAdist, eDistInd, aDistInd, idx, energyDist_size);
         
          Kokkos::atomic_fetch_add(&(energyDist[idx]), weight);
        }
      } //surface

      if(debug>1)
        printf(" surf13 timestep %d ptcl %d Atomics @id %d dep %g erosion %g "
         "avesput %g spYld %d wtStrike %g ptclStrike %d\n", iTimeStep, ptcl, 
         gridId, addGrossDep, addGrossEros, addAveSput, addSpYCount, 
         addSumWtStk, addSumPtclStk); 
      Kokkos::atomic_fetch_add(&(grossDeposition[gridId]), addGrossDep); 
      Kokkos::atomic_fetch_add(&(grossErosion[gridId]), addGrossEros);
      Kokkos::atomic_fetch_add(&(aveSputtYld[gridId]), addAveSput);
      Kokkos::atomic_fetch_add(&(sputtYldCount[gridId]), addSpYCount); 
      Kokkos::atomic_fetch_add(&(sumWtStrike[gridId]), addSumWtStk);
      Kokkos::atomic_fetch_add(&(sumPtclStrike[gridId]), addSumPtclStk);
      if(debug>1)
        printf(" surf14 timestep %d ptcl %d materialZ %d newWeight %g\n",
            iTimeStep, ptcl, materialZ, newWeight);
      if(materialZ > 0 && newWeight > 0) {
        ps_weight(pid) = newWeight;
        scs_hitNum(pid) = 0;
        ps_charge(pid) = 0;
        o::Real elCharge = 1.602e-19; //FIXME  
        o::Real protonMass = 1.66e-27;//FIXME
        auto v0 = sqrt(2*eInterpVal*elCharge/(amu*protonMass));
        ps_newVelMag(pid) = v0;
        auto vSampled = o::zero_vector<3>(); 
        o::Real pi = 3.1415;//FIXME
        vSampled[0] = v0*sin(aInterpVal*pi/180)*cos(2.0*pi*rand10);
        vSampled[1] = v0*sin(aInterpVal*pi/180)*sin(2.0*pi*rand10);
        vSampled[2] = v0*cos(aInterpVal*pi/180);
        auto face = p::get_face_coords_of_tet(face_verts, coords, fid);
        auto surfPar = o::normalize(face[1] - face[0]); // bdry face vtx
        auto vecY = o::cross(surfNormIn, surfPar);
        if(debug>1)
          printf(" surf15 timestep %d ptcl %d V0 %g rand10 %g vSampled %g %g %g "
            "norm %g %g %g surfPar %g %g %g  vecY %g %g %g \n", 
            iTimeStep, ptcl,v0, rand10, vSampled[0], vSampled[1], vSampled[2], 
            surfNormOut[0], surfNormOut[1], surfNormOut[2], surfPar[0], surfPar[1], 
            surfPar[2], vecY[0], vecY[1],vecY[2]);

        auto v = vSampled[0]*surfPar + vSampled[1]*vecY + vSampled[2]*surfNormIn;
        vSampled = v;
        for(int i=0; i<3; ++i)
          vel_ps(pid, i) = vSampled[i]*surfNormIn[i];
        //move reflected particle inwards
        auto newPos =  xpoint + shiftRefl*surfNormIn;
        for(o::LO i=0; i<3; ++i)
          next_pos_ps(pid,i) = newPos[i];

        if(debug>1)
          printf(" surf16 timestep %d ptcl %d xpt= pos %g %g %g => "
            " %g %g %g vel  %g %g %g =>  %g %g %g "
            " vsampled final  %g %g %g\n", 
            iTimeStep, ptcl, xpoint[0], xpoint[1], xpoint[2], newPos[0], newPos[1], 
            newPos[2], vel[0], vel[1], vel[2], vel_ps(pid, 0), vel_ps(pid, 1),
            vel_ps(pid, 2), vSampled[0], vSampled[1], vSampled[2]); 
      } else { //materialZ, newWeight
        scs_hitNum(pid) = 2;
      }
    } //mask
  }; //lambda
  p::parallel_for(ptcls, lamb, "surfaceModel");
  mesh.add_tag(o::FACE, "SumParticlesStrike", 1, o::Read<o::Int>(sumPtclStrike));
  mesh.add_tag(o::FACE, "SputtYldCount", 1, o::Read<o::Int>(sputtYldCount));
  mesh.add_tag(o::FACE, "SumWeightStrike", 1, o::Reals(sumWtStrike));
  mesh.add_tag(o::FACE, "GrossDeposition", 1, o::Reals(grossDeposition));
  mesh.add_tag(o::FACE, "GrossErosion", 1, o::Reals(grossErosion));
  mesh.add_tag(o::FACE, "AveSputtYld", 1, o::Reals(aveSputtYld));
}

#endif