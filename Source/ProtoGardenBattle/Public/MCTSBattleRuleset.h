#pragma once

#include "CoreMinimal.h"
#include "MCTSAgent.h"
#include "MovesetGenerator.h"

class PROTOGARDENBATTLE_API FMCTSBattleRuleset : public IMCTSRuleSet
{
public:
    void IngestMoveSets(TArray<FGeneratedMove> playerMoveList, TArray<FGeneratedMove> opponentMoveList, TArray<FGeneratedMove> systemMoveList);

    FMCTSGameState NextState(const FMCTSGameState& state, const FMCTSMove& move);
    TArray<FMCTSMove> EnumerateMoves(const FMCTSGameState& state);
    bool IsTerminalState(const FMCTSGameState& state);
    bool EvaluateTerminalState(const FMCTSGameState& state, int _playerIndex);
private:
    void ApplyPlatformStatusTriggersToState(const int castersIndex, const FMCTSMove& move, FMCTSGameState& inputState, bool& overrideJump);
    TArray<FGeneratedMove> playerMoveList;
    TArray<FGeneratedMove> opponentMoveList;
    TArray<FGeneratedMove> systemMoveList;
};
