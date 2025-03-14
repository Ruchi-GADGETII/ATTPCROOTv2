void run_eve_el(TString InputDataFile = "output_digi_gs.root", TString OutputDataFile = "output_el.reco_display.root",
                TString unpackDir = "/Simulation/ATTPC/10Be_dp/")
{
   FairLogger *fLogger = FairLogger::GetLogger();
   fLogger->SetLogToScreen(kTRUE);
   fLogger->SetLogVerbosityLevel("MEDIUM");
   TString dir = getenv("VMCWORKDIR");
   TString geoFile = "ATTPC_D600torr_v2_geomanager.root";

   TString InputDataPath = dir + "/macro/" + unpackDir + InputDataFile;
   TString OutputDataPath = dir + "/macro/" + unpackDir + OutputDataFile;
   TString GeoDataPath = dir + "/geometry/" + geoFile;

   FairRunAna *fRun = new FairRunAna();
   fRun->SetInputFile(InputDataPath);
   fRun->SetOutputFile(OutputDataPath);
   fRun->SetGeomFile(GeoDataPath);

   FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
   FairParRootFileIo *parIo1 = new FairParRootFileIo();
   // parIo1->open("param.dummy.root");
   rtdb->setFirstInput(parIo1);

   FairRootManager *ioman = FairRootManager::Instance();

   AtEventManager *eveMan = new AtEventManager();
   AtEventDrawTask *eve = new AtEventDrawTask();
   eve->Set3DHitStyleBox();
   eve->SetMultiHit(100); // Set the maximum number of multihits in the visualization
   eve->SetSaveTextData();
   eve->UnpackHoughSpace();

   eveMan->AddTask(eve);
   eveMan->Init();
}
