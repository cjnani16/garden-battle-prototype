// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "MCTSAgent.h"
#include "MCTSBattleRuleset.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Async/AsyncWork.h"
#include "MCTSPlayerController.generated.h"

// delegate for whatever node comes after the MCTS finishes.
DECLARE_DYNAMIC_DELEGATE_OneParam(FMCTSDelegate, FMCTSMove, ChosenMove);

/**
 *
 */
UCLASS()
class PROTOGARDENBATTLE_API AMCTSPlayerController : public AAIController, public IMCTSRuleSet
{
    GENERATED_BODY()
public:
    
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MCTS")
        FMCTSGameState NextState(const FMCTSGameState& state, const FMCTSMove& move);
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MCTS")
        TArray<FMCTSMove> EnumerateMoves(const FMCTSGameState& state);
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MCTS")
        bool IsTerminalState(const FMCTSGameState& state);
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "MCTS")
        bool EvaluateTerminalState(const FMCTSGameState& state, int _playerIndex);
    

    UFUNCTION(BlueprintCallable, Category = "MCTS")
        void SetupBattleMovesets(TArray<FGeneratedMove> playerMoveList, TArray<FGeneratedMove> opponentMoveList, TArray<FGeneratedMove> systemMoveList);
    UFUNCTION(BlueprintCallable, Category = "MCTS", meta = (BlueprintThreadSafe))
        void DecideNextMove(FMCTSDelegate Out, const FMCTSGameState& inputState, const int playerIndex, const int iterationBudget);
    UFUNCTION(BlueprintCallable, Category = "MCTS")
        void DecideNextMoveSync(FMCTSDelegate Out, const FMCTSGameState& inputState, const int playerIndex, const int iterationBudget);

protected:
    FMCTSBattleRuleset battleRuleSet;
    bool useBlueprint = true;
};