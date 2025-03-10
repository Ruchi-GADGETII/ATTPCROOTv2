void C14_dp_sim(Int_t nEvents = 10, TString mcEngine = "TGeant4")
{

  TString dir = getenv("VMCWORKDIR");

  // Output file name
  TString outFile ="heliossim.root";

  // Parameter file name
  TString parFile="heliospar.root";

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  AtVertexPropagator *vertex_prop = new AtVertexPropagator();

  // -----   Create simulation run   ----------------------------------------
  FairRunSim* run = new FairRunSim();
  run->SetName(mcEngine);              // Transport engine
  run->SetOutputFile(outFile);          // Output file
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  // ------------------------------------------------------------------------


  // -----   Create media   -------------------------------------------------
  run->SetMaterials("media.geo");       // Materials
  // ------------------------------------------------------------------------

  // -----   Create geometry   ----------------------------------------------

  FairModule* cave= new AtCave("CAVE");
  cave->SetGeometryFileName("cave.geo");
  run->AddModule(cave);

  FairDetector* HELIOS = new AtSiArray("HELIOS", kTRUE);
  HELIOS->SetGeometryFileName("HELIOS_SiArray_v1.0.root");
  //HELIOS->SetModifyGeometry(kTRUE);
  run->AddModule(HELIOS);

 // ------------------------------------------------------------------------


    // -----   Magnetic field   -------------------------------------------
    // Constant Field
    AtConstField  *fMagField = new AtConstField();
    fMagField->SetField(0., 0. ,-20. ); // values are in kG
    fMagField->SetFieldRegion(-100, 100,-100, 100, -250,250); // values are in cm
    fMagField->Print();
                          //  (xmin,xmax,ymin,ymax,zmin,zmax)
    run->SetField(fMagField);
    // --------------------------------------------------------------------



  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();




                  // Beam Information
                Int_t z = 6;  // Atomic number
	        Int_t a = 14; // Mass number
	        Int_t q = 0;   // Charge State
	        Int_t m = 1;   // Multiplicity  NOTE: Due the limitation of the TGenPhaseSpace accepting only pointers/arrays the maximum multiplicity has been set to 10 particles.
	        Double_t px = 0.000/a;  // X-Momentum / per nucleon!!!!!!
	        Double_t py = 0.000/a;  // Y-Momentum / per nucleon!!!!!!
	        Double_t pz = 1.916/a;  // Z-Momentum / per nucleon!!!!!!
  	        Double_t BExcEner = 0.0;
                Double_t Bmass = 14.003241; //Mass in GeV
                Double_t NomEnergy = 1.2; //Used to force the beam to stop within a certain energy range.
                Double_t TargetMass = 1.87614;//Mass in GeV


	          //ATTPCIonGenerator* ionGen = new ATTPCIonGenerator("Ion",z,a,q,m,px,py,pz,BExcEner,Bmass,NomEnergy);
	          //ionGen->SetSpotRadius(0,-100,0);
	          // add the ion generator

	         // primGen->AddGenerator(ionGen);

  		  //primGen->SetBeam(1,1,0,0); //These parameters change the position of the vertex of every track added to the Primary Generator
		  // primGen->SetTarget(30,0);




		 // Variables for 2-Body kinematics reaction
                  std::vector<Int_t> Zp; // Zp
		  std::vector<Int_t> Ap; // Ap
                  std::vector<Int_t> Qp;//Electric charge
                  Int_t mult;  //Number of particles
 		  std::vector<Double_t> Pxp; //Px momentum X
		  std::vector<Double_t> Pyp; //Py momentum Y
		  std::vector<Double_t> Pzp; //Pz momentum Z
                  std::vector<Double_t> Mass; // Masses
		  std::vector<Double_t> ExE; // Excitation energy
 		  Double_t ResEner; // Energy of the beam (Useless for the moment)


		  // Note: Momentum will be calculated from the phase Space according to the residual energy of the beam


	          mult = 4; //Number of Nuclei involved in the reaction (Should be always 4) THIS DEFINITION IS MANDATORY (and the number of particles must be the same)
                  ResEner = 140.0; // For fixed target mode (Si Array) in MeV

                  // ---- Beam ----
                  Zp.push_back(z); // 40Ar TRACKID=0
            	  Ap.push_back(a); //
            	  Qp.push_back(q);
                  Pxp.push_back(px);
            	  Pyp.push_back(py);
            	  Pzp.push_back(pz);
            	  Mass.push_back(14.003241);
            	  ExE.push_back(BExcEner);

                  // ---- Target ----
                 Zp.push_back(1); // t
		 Ap.push_back(2); //
		 Qp.push_back(0); //
		 Pxp.push_back(0.0);
                 Pyp.push_back(0.0);
		 Pzp.push_back(0.0);
                 Mass.push_back(2.0135532);
		 ExE.push_back(0.0);//In MeV

                  //--- Scattered -----
                Zp.push_back(6); // 32Mg TRACKID=1
                Ap.push_back(15); //
                Qp.push_back(0);
          	Pxp.push_back(0.0);
          	Pyp.push_back(0.0);
          	Pzp.push_back(0.0);
          	Mass.push_back(15.0106);
          	ExE.push_back(0.0);


                 // ---- Recoil -----
		 Zp.push_back(1); // p  TRACKID=2
		 Ap.push_back(1); //
		 Qp.push_back(0); //
		 Pxp.push_back(0.0);
                 Pyp.push_back(0.0);
		 Pzp.push_back(0.0);
                 Mass.push_back(1.00783);
		 ExE.push_back(0.0);//In MeV


                 Double_t ThetaMinCMS = 10.0;
                 Double_t ThetaMaxCMS = 40.0;

                 AtTPC2Body *TwoBody = new AtTPC2Body("TwoBody", &Zp, &Ap, &Qp, mult, &Pxp, &Pyp, &Pzp, &Mass, &ExE,
                                                      ResEner, ThetaMinCMS, ThetaMaxCMS);
                 TwoBody->SetFixedTargetPosition(0.0, 0.0, 0.0);
                 TwoBody->SetFixedBeamMomentum(0.0, 0.0, pz * a);
                 primGen->AddGenerator(TwoBody);

                 run->SetGenerator(primGen);

                 // ------------------------------------------------------------------------

                 //---Store the visualiztion info of the tracks, this make the output file very large!!
                 //--- Use it only to display but not for production!
                 run->SetStoreTraj(kTRUE);

                 // -----   Initialize simulation run   ------------------------------------
                 run->Init();
                 // ------------------------------------------------------------------------

                 // Trajectory filters

                 // -----   Runtime database   ---------------------------------------------

                 Bool_t kParameterMerged = kTRUE;
                 FairParRootFileIo *parOut = new FairParRootFileIo(kParameterMerged);
                 parOut->open(parFile.Data());
                 rtdb->setOutput(parOut);
                 rtdb->saveOutput();
                 rtdb->print();
                 // ------------------------------------------------------------------------

                 // -----   Start run   ----------------------------------------------------
                 run->Run(nEvents);

                 // You can export your ROOT geometry ot a separate file
                 run->CreateGeometryFile("geofile_helios_full.root");
                 // ------------------------------------------------------------------------

                 // -----   Finish   -------------------------------------------------------
                 timer.Stop();
                 Double_t rtime = timer.RealTime();
                 Double_t ctime = timer.CpuTime();
                 cout << endl << endl;
                 cout << "Macro finished succesfully." << endl;
                 cout << "Output file is " << outFile << endl;
                 cout << "Parameter file is " << parFile << endl;
                 cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
                 // ------------------------------------------------------------------------
}
