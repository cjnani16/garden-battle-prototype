#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "MCTSAgent.h"
#include "MCTSTask.generated.h"

class FMCTSTask : public FNonAbandonableTask
{
public: 
	FMCTSTask(int _perspectiveIndex, FMCTSGameState _startingState) : perspectiveIndex(_perspectiveIndex), startingState(_startingState) {}

private:
	int perspectiveIndex;
	FMCTSGameState startingState;
};