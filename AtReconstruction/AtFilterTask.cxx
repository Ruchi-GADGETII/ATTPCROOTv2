#include "AtFilterTask.h"

#include "AtAuxPad.h"
#include "AtFilter.h"
#include "AtRawEvent.h"

#include <FairLogger.h>
#include <FairRootManager.h>
#include <FairTask.h>

#include <TClonesArray.h>
#include <TObject.h>

#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class AtPad;

AtFilterTask::AtFilterTask(AtFilter *filter, const char *name)
   : FairTask(name), fOutputEventArray(new TClonesArray("AtRawEvent")), fFilter(filter)
{
}

InitStatus AtFilterTask::Init()
{
   FairRootManager *ioManager = FairRootManager::Instance();

   if (ioManager == nullptr) {
      LOG(ERROR) << "Cannot find RootManager!" << std::endl;
      return kERROR;
   }

   // Get the old data from the io manager
   fInputEventArray = dynamic_cast<TClonesArray *>(ioManager->GetObject(fInputBranchName));
   if (fInputEventArray == nullptr) {
      LOG(fatal) << "AtFilterTask: Cannot find AtRawEvent array!";
      return kFATAL;
   }

   // Set the raw event array, and new output event array
   ioManager->Register(fOutputBranchName, "AtTPC", fOutputEventArray, fIsPersistent);

   fFilter->Init();

   return kSUCCESS;
}

void AtFilterTask::Exec(Option_t *opt)
{
   fOutputEventArray->Delete();

   if (fInputEventArray->GetEntriesFast() == 0)
      return;

   auto rawEvent = dynamic_cast<AtRawEvent *>(fInputEventArray->At(0));
   fFilter->InitEvent(rawEvent); // Can modify rawEvent if necessary (shouldn't touch traces)
   auto filteredEvent = fFilter->ConstructOutputEvent(fOutputEventArray, rawEvent);

   if (!rawEvent->IsGood())
      return;

   if (fFilterAux)
      for (auto &padIt : filteredEvent->fAuxPadMap) {
         AtPad *pad = &(padIt.second);
         fFilter->Filter(pad);
      }

   // This is destroying data in next pad in the array
   for (auto &pad : filteredEvent->fPadList)
      fFilter->Filter(pad.get());

   auto isGood = filteredEvent->IsGood() && fFilter->IsGoodEvent();
   filteredEvent->SetIsGood(isGood);
}
