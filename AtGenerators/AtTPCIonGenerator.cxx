// -------------------------------------------------------------------------
// -----            Based on FairIonGenerator source file              -----
// -----            Created 30/01/15  by Y. Ayyad                      -----
// -------------------------------------------------------------------------
#include "AtTPCIonGenerator.h"

#include "AtVertexPropagator.h"

#include <FairIon.h>
#include <FairParticle.h>
#include <FairPrimaryGenerator.h>
#include <FairRunSim.h>

#include <TDatabasePDG.h>
#include <TMath.h>
#include <TObjArray.h>
#include <TObject.h> // for TObject
#include <TParticle.h>
#include <TParticlePDG.h>
#include <TRandom.h>
#include <TString.h>

#include <algorithm> // for clamp
#include <cmath>
#include <iostream>
#include <limits> // for numeric_limits

constexpr auto cRED = "\033[1;31m";
constexpr auto cYELLOW = "\033[1;33m";
constexpr auto cNORMAL = "\033[0m";
constexpr auto cGREEN = "\033[1;32m";
constexpr auto cBLUE = "\033[1;34m";

using std::cout;
using std::endl;

// -----   Initialsisation of static variables   --------------------------
Int_t AtTPCIonGenerator::fgNIon = 0;
// ------------------------------------------------------------------------

// -----   Default constructor   ------------------------------------------
AtTPCIonGenerator::AtTPCIonGenerator()
   : fMult(0), fPx(0.), fPy(0.), fPz(0.), fR(0.), fz(0.), fOffset(0.), fVx(0.), fVy(0.), fVz(0.), fIon(nullptr), fQ(0),
     fBeamOpt(0), fWhmFocus(0.), fDiv(0.), fZFocus(0.), fRHole(0.)
{
   //  cout << "-W- AtTPCIonGenerator: "
   //      << " Please do not use the default constructor! " << endl;
}
// ------------------------------------------------------------------------

AtTPCIonGenerator::AtTPCIonGenerator(const Char_t *ionName, Int_t mult, Double_t px, Double_t py, Double_t pz)
   : fMult(0), fPx(0.), fPy(0.), fPz(0.), fR(0.), fz(0.), fOffset(0.), fVx(0.), fVy(0.), fVz(0.), fIon(nullptr), fQ(0),
     fBeamOpt(0), fWhmFocus(0.), fDiv(0.), fZFocus(0.), fRHole(0.)
{

   FairRunSim *fRun = FairRunSim::Instance();
   TObjArray *UserIons = fRun->GetUserDefIons();
   TObjArray *UserParticles = fRun->GetUserDefParticles();
   FairParticle *part = nullptr;
   fIon = dynamic_cast<FairIon *>(UserIons->FindObject(ionName)); // NOLINT

   if (fIon) {
      fgNIon++;
      fMult = mult;
      fPx = Double_t(fIon->GetA()) * px;
      fPy = Double_t(fIon->GetA()) * py;
      fPz = Double_t(fIon->GetA()) * pz;
      // fVx   = vx;
      // fVy   = vy;
      // fVz   = vz;
      // }

   } else {
      part = dynamic_cast<FairParticle *>(UserParticles->FindObject(ionName));
      if (part) {
         fgNIon++;
         TParticle *particle = part->GetParticle();
         fMult = mult;
         fPx = Double_t(particle->GetMass() / 0.92827231) * px;
         fPy = Double_t(particle->GetMass() / 0.92827231) * py;
         fPz = Double_t(particle->GetMass() / 0.92827231) * pz;
         // fVx   = vx;
         // fVy   = vy;
         // fVz   = vz;
      }
   }
   if (fIon == nullptr && part == nullptr) {
      cout << "-E- AtTPCIonGenerator: Ion or Particle is not defined !" << endl;
      Fatal("AtTPCIonGenerator", "No FairRun instantised!");
   }
}
// ------------------------------------------------------------------------

// -----   Default constructor   ------------------------------------------
AtTPCIonGenerator::AtTPCIonGenerator(const char *name, Int_t z, Int_t a, Int_t q, Int_t mult, Double_t px, Double_t py,
                                     Double_t pz, Double_t Ex, Double_t m, Double_t ener, Double_t eLoss)
   : fMult(mult), fPx(Double_t(a) * px), fPy(Double_t(a) * py), fPz(Double_t(a) * pz), fR(0.), fz(0.), fOffset(0.),
     fVx(0.), fVy(0.), fVz(0.), fIon(nullptr), fQ(0), fBeamOpt(0), fNomEner(ener), fMaxEnLoss(eLoss < 0 ? ener : eLoss),
     fWhmFocus(0.), fDiv(0.), fZFocus(0.), fRHole(0.)
{
   fgNIon++;

   fIon = new FairIon(TString::Format("FairIon%d", fgNIon).Data(), z, a, q, Ex, m); // NOLINT
   cout << " Beam Ion mass : " << fIon->GetMass() << endl;
   AtVertexPropagator::Instance()->SetBeamMass(fIon->GetMass());
   AtVertexPropagator::Instance()->SetBeamNomE(ener);
   FairRunSim *run = FairRunSim::Instance();
   if (!run) {
      cout << "-E- FairIonGenerator: No FairRun instantised!" << endl;
      Fatal("FairIonGenerator", "No FairRun instantised!");
      return;
   }
   run->AddNewIon(fIon);
}
//_________________________________________________________________________

AtTPCIonGenerator::AtTPCIonGenerator(const AtTPCIonGenerator &right)
   : fMult(right.fMult), fPx(right.fPx), fPy(right.fPy), fPz(right.fPz), fR(right.fR), fz(right.fz),
     fOffset(right.fOffset), fVx(right.fVx), fVy(right.fVy), fVz(right.fVz), fIon(right.fIon), fQ(right.fQ),
     fBeamOpt(right.fBeamOpt), fWhmFocus(right.fWhmFocus), fDiv(right.fDiv), fZFocus(right.fZFocus),
     fRHole(right.fRHole)
{
}

// -----   Public method SetExcitationEnergy   ----------------------------
void AtTPCIonGenerator::SetExcitationEnergy(Double_t eExc)
{
   fIon->SetExcEnergy(eExc);
}
//_________________________________________________________________________

// -----   Public method SetMass   ----------------------------------------
void AtTPCIonGenerator::SetMass(Double_t mass)
{
   fIon->SetMass(mass);
}

// -----   Private method SetEmittance   ----------------------------------
void AtTPCIonGenerator::SetEmittance()
{

   Double_t x = 0., y = 0., xFocus = 0., yFocus = 0., theta = 0., phi = 0.;
   Double_t ptot = sqrt(pow(fPx, 2) + pow(fPy, 2) + pow(fPz, 2));

   /*
   Ex.
           fWhmFocus = 1.; //cm, FWHM of Gaussian
           fDiv = 10.*1E-3; //radians
           fZFocus = 50; //cm, focus distance from entrance
   */
   // x is coordinates of beam particle at AtTPC entrance, xFocus is coordinates at focus.
   xFocus = gRandom->Gaus(0, fWhmFocus / 2.355);
   yFocus = gRandom->Gaus(0, fWhmFocus / 2.355);

   do {
      theta = gRandom->Uniform(-fDiv, fDiv);
      phi = gRandom->Uniform(-fDiv, fDiv);
      x = xFocus - fZFocus * tan(phi);
      y = yFocus - sqrt(pow(fZFocus, 2) + pow(xFocus - x, 2)) * tan(theta);
   } while (sqrt(pow(x, 2) + pow(y, 2)) > fRHole && sqrt(pow(tan(theta), 2) + pow(tan(phi), 2)) > tan(fDiv));

   fVx = x;
   fVy = y;
   fVz = 0.;

   fPx = ptot * cos(theta) * sin(phi);
   fPy = ptot * sin(theta);
   fPz = sqrt(ptot * ptot - fPx * fPx - fPy * fPy);

   AtVertexPropagator::Instance()->Setd2HeVtx(fVx, fVy, theta, phi);
}
//_________________________________________________________________________

// -----   Private method SetBeamOrigin   ---------------------------------
void AtTPCIonGenerator::SetBeamOrigin()
{
   double pi = 2 * asin(1.0);

   Double_t radius = std::clamp(gRandom->Gaus(0, fR / 3), 0.0, fR);
   Double_t phi_R = gRandom->Uniform(0, 2 * pi);
   fVx = radius * cos(phi_R);
   fVy = radius * sin(phi_R);

   Double_t theta = gRandom->Uniform(0, fTheta);
   Double_t pr = fPz * sin(theta);
   fPz *= cos(theta);
   fPx = pr * cos(phi_R);
   fPy = pr * sin(phi_R);
}
//_________________________________________________________________________

// -----   Public method ReadEvent   --------------------------------------
Bool_t AtTPCIonGenerator::ReadEvent(FairPrimaryGenerator *primGen)
{

   // if ( ! fIon ) {
   //   cout << "-W- FairIonGenerator: No ion defined! " << endl;
   //   return kFALSE;
   // }

   TParticlePDG *thisPart = TDatabasePDG::Instance()->GetParticle(fIon->GetName());
   if (!thisPart) {
      cout << "-W- FairIonGenerator: Ion " << fIon->GetName() << " not found in database!" << endl;
      return kFALSE;
   }

   int pdgType = thisPart->PdgCode();

   switch (fBeamOpt) {
   case 1: {
      auto Phi = gRandom->Uniform(0, 360) * TMath::DegToRad();
      auto SpotR = gRandom->Uniform(0, fR);

      fVx = SpotR * cos(Phi);           // gRandom->Uniform(-fx,fx);
      fVy = fOffset + SpotR * sin(Phi); // gRandom->Uniform(-fy,fy);
      fVz = fz;
      break;
   }
   case 2:
      SetEmittance(); // parameters: fWhmFocus, fDiv, fZFocus, fRHole, fPx, fPy, fPz
      // changes: fVx, fVy, fVz, fPx, fPy, fPz, d2HeVtx
      break;
   case 3: SetBeamOrigin(); break;
   default:
      fVx = 0.0;
      fVy = 0.0;
      fVz = 0.0;
   }
   /*
     cout << "-I- FairIonGenerator: Generating " << fMult <<" with mass "<<thisPart->Mass() << " ions of type "
          << fIon->GetName() << " (PDG code " << pdgType << ")" << endl;
     cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
          << ") Gev from vertex (" << fVx << ", " << fVy
          << ", " << fVz << ") cm" << endl;
   */
   AtVertexPropagator::Instance()->IncBeamEvtCnt();

   if (AtVertexPropagator::Instance()->GetBeamEvtCnt() % 2 != 0) {
      if (fDoReact) {
         Double_t Er = gRandom->Uniform(0., fMaxEnLoss);
         AtVertexPropagator::Instance()->SetRndELoss(Er);
         // std::cout << cGREEN << " Random Energy AtTPCIonGenerator : " << Er << cNORMAL << std::endl;
      } else
         AtVertexPropagator::Instance()->SetRndELoss(std::numeric_limits<double>::max());
   }

   for (Int_t i = 0; i < fMult; i++)
      primGen->AddTrack(pdgType, fPx, fPy, fPz, fVx, fVy, fVz);

   return kTRUE;
}

//_____________________________________________________________________________

ClassImp(AtTPCIonGenerator)
