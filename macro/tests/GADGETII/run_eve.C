void run_eve(TString InputDataFile = "./data/output.root", TString OutputDataFile = "./data/output.reco_display.root",
             TString unpackDir = "/Unpack_GETDecoder2/GADGETII/")
{
   FairLogger *fLogger = FairLogger::GetLogger();
   fLogger->SetLogToScreen(kTRUE);
   fLogger->SetLogVerbosityLevel("MEDIUM");
   TString dir = getenv("VMCWORKDIR");
   // TString geoFile = "ATTPC_v1.1_geomanager.root";
   TString geoFile = "GADGET_II_lp_geomanager.root";
   TString mapFile = "LookupGADGET08232021.xml";

   TString InputDataPath = dir + "/macro/" + unpackDir + InputDataFile;
   TString OutputDataPath = dir + "/macro/" + unpackDir + OutputDataFile;
   TString GeoDataPath = dir + "/geometry/" + geoFile;
   TString mapDir = dir + "/scripts/" + mapFile;

   FairRunAna *fRun = new FairRunAna();
   FairRootFileSink *sink = new FairRootFileSink(OutputDataFile);
   FairFileSource *source = new FairFileSource(InputDataFile);
   fRun->SetSource(source);
   fRun->SetSink(sink);
   fRun->SetGeomFile(GeoDataPath);

   FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
   FairParRootFileIo *parIo1 = new FairParRootFileIo();
   // parIo1->open("param.dummy.root");
   rtdb->setFirstInput(parIo1);

   AtEventManager *eveMan = new AtEventManager();
   AtEventDrawTask *eve = new AtEventDrawTask();
   auto fMap = std::make_shared<AtGadgetIIMap>();
   fMap->ParseXMLMap(mapDir.Data());
   eve->SetMap(fMap);
   eve->Set3DHitStyleBox();
   eve->SetMultiHit(100); // Set the maximum number of multihits in the visualization
                          // eve->SetSaveTextData();
   eve->UnpackHoughSpace();

   eveMan->AddTask(eve);
   eveMan->Init();
   // eveMan->GoToEvent(0);
}
