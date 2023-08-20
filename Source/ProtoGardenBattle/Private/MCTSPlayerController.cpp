#include "MCTSPlayerController.h"

void AMCTSPlayerController::SetupBattleMovesets(
    TArray<FGeneratedMove> playerMoveList,
    TArray<FGeneratedMove> opponentMoveList,
    TArray<FGeneratedMove> systemMoveList
)
{
    battleRuleSet = FMCTSBattleRuleset();
    battleRuleSet.IngestMoveSets(playerMoveList, opponentMoveList, systemMoveList);
    useBlueprint = false;
}

void AMCTSPlayerController::DecideNextMove(
    FMCTSDelegate Out,
    const FMCTSGameState& inputState, 
    const int playerIndex,
    const int iterationBudget
)
{
    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [Out, inputState, playerIndex, iterationBudget, this]()
        {
            UMCTSAgent agent = UMCTSAgent(iterationBudget);
            if (useBlueprint)
                agent.ruleSet = this;
            else
                agent.ruleSet = &battleRuleSet;

            // Keep making decisions until a stop is decided.
            TArray<FMCTSMove> decision = {};
            int totalDecisionBudget = 10;
            while (totalDecisionBudget > 0 && (decision.IsEmpty() || decision.Last().moveIndex != -1)) {
                decision = agent.Decide(inputState, playerIndex);

                for (FMCTSMove move : decision) {
                    AsyncTask(ENamedThreads::GameThread, [Out, move]()
                        {
                            // We execute the delegate along with the param
                            Out.ExecuteIfBound(move);
                        }
                    );
                }

                totalDecisionBudget--;
            }

        }
    );
}

void AMCTSPlayerController::DecideNextMoveSync(
    FMCTSDelegate Out,
    const FMCTSGameState& inputState,
    const int playerIndex,
    const int iterationBudget
)
{
    UMCTSAgent agent = UMCTSAgent(iterationBudget);
    agent.ruleSet = &battleRuleSet;
    TArray<FMCTSMove> decision = agent.Decide(inputState, playerIndex);

    if (decision.IsEmpty() || decision.Last().moveIndex != -1)
        decision.Add(FMCTSMove(playerIndex));

    for (FMCTSMove move : decision) {
        // We execute the delegate along with the param
        Out.ExecuteIfBound(move);
    }
}