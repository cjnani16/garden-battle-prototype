#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include "Math/Vector2D.h"
#include "Math/IntPoint.h"
#include "MCTSAgent.generated.h"

// Structs

USTRUCT(BlueprintType)
struct FMCTSMonsterState {
    GENERATED_BODY()

    FMCTSMonsterState() : id(0), atk(0), def(0), spd(0), temp(0), hum(0), elev(0), ap(0), score(0), position(FVector2D(0)){}

    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    int id;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float atk;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float def;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float spd;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float temp;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float hum;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float elev;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    int ap;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    float score;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    FVector2D position;
};

UENUM(BlueprintType)
enum class EMCTSPlatformStatusTypes : uint8 {
    Lockdown,
    Freeze,
    Sandtrap,
    Ignite,
    Flood
};

USTRUCT(BlueprintType)
struct FMCTSPlatformState {
    GENERATED_BODY()

        FMCTSPlatformState() : temp(0), hum(0), elev(0), statuses({}) {}

    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        float temp;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        float hum;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        float elev;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        TArray<EMCTSPlatformStatusTypes> statuses;
};

USTRUCT(BlueprintType)
struct FMCTSGameState {
    GENERATED_BODY()

        FMCTSGameState() : turnCount(0), actingPlayerIndex(0), monsterStates({}), platformStates({}) {}

    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        int turnCount;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        int actingPlayerIndex;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        TArray<FMCTSMonsterState>     monsterStates;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        TArray<FMCTSPlatformState>    platformStates;
};

USTRUCT(BlueprintType)
struct FMCTSMoveTargetingData {
    GENERATED_BODY()

    FMCTSMoveTargetingData() : selectorIndex(0), target(FVector2D(0)) {}
    FMCTSMoveTargetingData(int _selectorIndex, FVector2D _target) : selectorIndex(0), target(_target) {}

    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        int                 selectorIndex;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
        FVector2D           target;
};

USTRUCT(BlueprintType)
struct FMCTSMove {
    GENERATED_BODY()

    FMCTSMove() : moveIndex(-1), targets({}), playerIndex(0), cost(0), selectionCount(0), winCount(0) {}

    FMCTSMove(int pIndex) : moveIndex(-1), targets({}), playerIndex(pIndex), cost(0), selectionCount(0), winCount(0) {}

    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    int moveIndex;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    TArray<FMCTSMoveTargetingData> targets;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    int playerIndex;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS")
    int cost;

    UPROPERTY(BlueprintReadWrite, Category = "MCTS Details")
    int selectionCount;
    UPROPERTY(BlueprintReadWrite, Category = "MCTS Details")
    int winCount;

public:
    FString ToString() const {
        FString ret = FString::Printf(TEXT("@%d-(%d)-#%d:"), playerIndex, cost, moveIndex);
        ret += TargetsToString();
        return ret;
    }
private:
    FString TargetsToString() const {
        FString ret = "";
        for (FMCTSMoveTargetingData t : targets) {
            //FIntPoint loc = t.target.IntPoint();
            ret += FString::Printf(TEXT("(%d,%d)"), static_cast<int>(t.target.X), static_cast<int>(t.target.Y));
        }
        return ret;
    }
};

// Interfaces
class IMCTSRuleSet {
public:
    virtual FMCTSGameState NextState(const FMCTSGameState& state, const FMCTSMove& move) = 0;
    virtual TArray<FMCTSMove> EnumerateMoves(const FMCTSGameState& state) = 0;
    virtual bool IsTerminalState(const FMCTSGameState& state) = 0;
    virtual bool EvaluateTerminalState(const FMCTSGameState& state, int _playerIndex) = 0; // return True if this is a win for given player
};

UINTERFACE(BlueprintType)
class UMCTSEvaluatorModel : public UInterface {
    GENERATED_BODY()
};

class IMCTSEvaluatorModel {
public:
    GENERATED_BODY()
    virtual TArray<float> Evaluate(const FMCTSGameState& state) = 0;
};

// Node class
class UMCTSNode {
public:
    UMCTSNode() : state(), children({}), parent(nullptr), selectionCount(0), winCount(0) {}
    UMCTSNode(const FMCTSGameState& state) : state(state),  children({}), parent(nullptr), selectionCount(0), winCount(0) {}

    void Update(bool win) {
        //selectionCount++;
        if (win)
            winCount++;

        if (parent)
            parent->Update(win != (parent->state.actingPlayerIndex != state.actingPlayerIndex));
    }

    void ReleaseChildren() {
        for (auto& pair : children) {
            pair.Value->ReleaseChildren();
            delete pair.Value;
        }
    }

    FMCTSGameState state;
    TMap<FString, UMCTSNode*> children; // Maps FMCTSMove (as string) to UMCTSNode.
    UMCTSNode* parent;
    int selectionCount;
    int winCount;
};

// Agent class
class UMCTSAgent {

public:
    IMCTSRuleSet* ruleSet;
    IMCTSEvaluatorModel* model;

    UMCTSAgent(int budget)
        : playerIndex(0), maxSimulationDepth(150), decisionBudget(budget), playoutBudget(10), rootNode(nullptr) {}

    //UMCTSAgent(int playerIndex, int maxSimulationDepth, int decisionBudget)
    //    : playerIndex(playerIndex), maxSimulationDepth(maxSimulationDepth), decisionBudget(decisionBudget) {}

    ~UMCTSAgent() {
        if (rootNode) {
            rootNode->ReleaseChildren();
            delete rootNode;
        }
    }

    TArray<FMCTSMove> Decide(const FMCTSGameState& state, int perspectiveIndex) {
        playerIndex = perspectiveIndex;

        // Default move if no ruleset set
        if (ruleSet == nullptr || ruleSet->IsTerminalState(state))
            return { FMCTSMove(playerIndex) };

        // Create a tree with all the scores resulting from MCTS algorithm
        if (!rootNode)
            rootNode = new UMCTSNode(state);

        for (int i = 0; i < decisionBudget; i++) {
            UMCTSNode* selectedNode = rootNode;
            UMCTSNode* expandedNode = nullptr;

            // UE_LOG(LogTemp, Display, TEXT("\n(#%d) Starting at root:\n%s"), i, *DebugNodeString(rootNode));

            selectedNode = Select(selectedNode);
            // UE_LOG(LogTemp, Display, TEXT("\nSelected:\n%s"), *DebugNodeString(selectedNode));
            
            expandedNode = Expand(selectedNode);
            
            if (expandedNode) {
                // UE_LOG(LogTemp, Display, TEXT("\nSimulating...:\n%s"), *DebugNodeString(expandedNode));
                for (int j = 0; j < playoutBudget; j++) {
                    bool win = Simulate(expandedNode);
                    // UE_LOG(LogTemp, Display, TEXT("\n...Result: %s.Sending back Update."), win ? TEXT("Win!") : TEXT("Lose."));
                    Update(expandedNode, win);
                }
            }
        }

        // Now, assemble a list of moves for the correct player by traversing the tree.
        // UE_LOG(LogTemp, Display, TEXT("\nStarting root at end of tree construction:\n%s"), *DebugNodeString(rootNode));

        // Play but ignore and preceding moves by other player.
        if (rootNode->state.actingPlayerIndex != playerIndex) {
            UE_LOG(LogTemp, Display, TEXT("Seems PID %d is going second, so skipping episode from first player. (Root acting PID=%d)"), playerIndex, rootNode->state.actingPlayerIndex);
            TArray<FMCTSMove> oppMoveList = {};
            TraverseEpisode(rootNode, oppMoveList);

            // "validate" the move, but return nothing indicating should retry
            ValidateMove(oppMoveList[0]);

            return {};
        }

        // Hopefully the above has resulted in us getting to the start of our turn
        if (!rootNode || rootNode->state.actingPlayerIndex != playerIndex) {
            UE_LOG(LogTemp, Error, TEXT("No more moves for PID=%d left in the tree... This L is guaranteed :("), playerIndex);
            UE_LOG(LogTemp, Error, TEXT("\nProblematic final root:\n%s"), *DebugNodeString(rootNode));
            return { FMCTSMove(playerIndex) };
        }

        // Root at this point should match playerIndex
        // UE_LOG(LogTemp, Display, TEXT("\nFinal result on root:\n%s"), *DebugNodeString(rootNode));

        // Now actually keep the best move list and return it.
        TArray<FMCTSMove> moveList = {};
        // rootNode = TraverseEpisode(rootNode, moveList); Don't reassign rootNode, so it's saved
        
        TraverseEpisode(rootNode, moveList);

        //Here's where I'd decay decision budget

        //Near end of game, may be asked to decide on a terminal state. In which case no moves will return.
        if (moveList.IsEmpty())
            moveList = { FMCTSMove(playerIndex) };

        //Auto-validate decisions as there's no need to go back to game thread to check.
        ValidateMove(moveList[0]);
        
        return moveList;
    }

private:

    void ValidateMove(FMCTSMove validMove) {
        if (rootNode && rootNode->children.Contains(validMove.ToString())) {
            UMCTSNode* newRoot = rootNode->children[validMove.ToString()];

            // Discard uneeded branches
            rootNode->children.Remove(validMove.ToString());
            rootNode->ReleaseChildren();
            delete rootNode;

            // Save new starting root
            rootNode = newRoot;
            newRoot->parent = nullptr;
        }

        UE_LOG(LogTemp, Display, TEXT("\nStarting root at end of tree construction:\n%s"), *DebugNodeString(rootNode));
    }

    UMCTSNode* TraverseEpisode(UMCTSNode* node, TArray<FMCTSMove>& bestMoves) {
        int depth = 0;
        UE_LOG(LogTemp, Display, TEXT("******Starting Episode Playout***********"));
        while (depth < maxSimulationDepth && !ruleSet->IsTerminalState(node->state)) {
            TArray<FMCTSMove> moves = ruleSet->EnumerateMoves(node->state);
            if (moves.IsEmpty())
                break;

            UE_LOG(LogTemp, Display, TEXT("Move #%d: %s"), depth, *DebugNodeString(node));

            // Find the move that leads to the best child
            int bestScore = -1;
            UMCTSNode* bestNode = nullptr;
            FMCTSMove bestMove;
            for (FMCTSMove m : moves) {
                FString moveKey = m.ToString();
                if (!node->children.Contains(moveKey))
                    break;

                UMCTSNode* child = node->children[moveKey];
                if (bestScore < child->selectionCount) {
                    bestScore = child->selectionCount;
                    bestMove = m;
                    bestNode = child;
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("Best score: %d"), bestScore);

            if (!bestNode) {
                UE_LOG(LogTemp, Error, TEXT("Failed to find best child here. Ending episode."));
                UE_LOG(LogTemp, Display, TEXT("******Ending Episode Playout***********"));
                return node;
            }

            bestMoves.Add(bestMove);
            node = bestNode;

            UE_LOG(LogTemp, Warning, TEXT("Chose move index %d : %s."), bestMove.moveIndex, *bestMove.ToString());

            // True here to force a return after just one move.
            if (true || bestMove.moveIndex == -1) {
                UE_LOG(LogTemp, Display, TEXT("******Ending Episode Playout***********"));
                return node;
            }
            depth++;
        }
        UE_LOG(LogTemp, Warning, TEXT("Hit terminal state or reached max depth. Ending episode."));
        UE_LOG(LogTemp, Error, TEXT("******Ending Episode Playout***********"));
        return node;
    }

    FString DebugNodeString(UMCTSNode* n) {
        auto moves = ruleSet->EnumerateMoves(n->state);

        FString ret = FString::Printf(TEXT("(%d/%d) - Turn %d. %d possible moves - Acting player: %i (@(%f,%f)) - AP left: %i.\n"), n->winCount, n->selectionCount, n->state.turnCount, moves.Num(), n->state.actingPlayerIndex, (n->state.monsterStates[n->state.actingPlayerIndex].position.X), (n->state.monsterStates[n->state.actingPlayerIndex].position.Y), n->state.monsterStates[n->state.actingPlayerIndex].ap);
        
        int i = 0;
        for (FMCTSMove m : moves ) {
            FString key = m.ToString();
            ret += n->children.Contains(key) ? FString::Printf(TEXT("%d:(%d/%d)[%s],  "), i, n->children[key]->winCount, n->children[key]->selectionCount, *key) : FString::Printf(TEXT("%d:(x)[%s],  "),i,*key);
            i++;
        }
        return ret;
    }

    UMCTSNode* Expand(UMCTSNode* node) {
        TArray<FMCTSMove> moves = ruleSet->EnumerateMoves(node->state);
        for (const FMCTSMove& move : moves) {
            if (!node->children.Contains(move.ToString())) {
                FMCTSGameState nextState = ruleSet->NextState(node->state, move);
                if (nextState.monsterStates.Num() < 2) {
                    FString culpritString = move.ToString();
                    UE_LOG(LogTemp, Error, TEXT("\nExpanded state empty! Culprit: %s"), *culpritString);
                    return nullptr;
                }
                UMCTSNode* childNode = new UMCTSNode(nextState);
                childNode->parent = node;
                node->children.Add(move.ToString(), childNode);
                return childNode;
            }
        }
        return nullptr;
    }

    bool Simulate(UMCTSNode* node) {
        FMCTSGameState currentState = node->state;
        int depth = 0;
        FMCTSGameState simmedState;
        // UE_LOG(LogTemp, Display, TEXT("\n**************Starting a Simulation**********************"));
        while (depth < maxSimulationDepth && !ruleSet->IsTerminalState(currentState)) {
            TArray<FMCTSMove> moves = ruleSet->EnumerateMoves(currentState);
            if (moves.IsEmpty())
                break;

            // Ideally, we use the trained model to evaluate the resulting state for each move, and choose the best.
            /*
            // GREEDY PLAYOUT POLICY
            
            int bestMoveIndex = -1;
            float bestEvaluation = -500;
            
            for (int i = 0; i < moves.Num(); i++) {
                // TArray<float> currentMoveEvaluations = model->Evaluate(currentState);
                FMCTSGameState currentMoveResultingState = ruleSet->NextState(currentState, moves[i]);
                float currentMoveEvaluation = currentMoveResultingState.monsterStates[currentState.actingPlayerIndex].score - currentMoveResultingState.monsterStates[1 - currentState.actingPlayerIndex].score;
                if (currentMoveEvaluation > bestEvaluation) {
                    bestEvaluation = currentMoveEvaluation;
                    bestMoveIndex = i;
                }
            }
            
            if (bestMoveIndex < 0)
                bestMoveIndex = FMath::RandRange(0, moves.Num()-1); // Traditional MCTS: Just run moves randomly during sim.
            */

            // RANDOM PLAYOUT POLICY
            int bestMoveIndex = FMath::RandRange(0, moves.Num() - 1); // Traditional MCTS: Just run moves randomly during sim.

            // FString sim = FString::Printf(TEXT("Turn %d. %d possible moves - Acting player: %i - AP left: %i.\n"), currentState.turnCount, moves.Num(), currentState.actingPlayerIndex, currentState.monsterStates[currentState.actingPlayerIndex].ap);

            // UE_LOG(LogTemp, Display, TEXT("\nSimulation Step %d: %s"), depth, *sim);

            if (currentState.monsterStates.Num() < 2) {
                UE_LOG(LogTemp, Error, TEXT("\nCurrent state empty before sim!"));
                return false;
            }

            const FMCTSMove& selectedMove = moves[bestMoveIndex];
            simmedState = ruleSet->NextState(currentState, selectedMove);
            if (simmedState.monsterStates.Num() < 2) {
                FString culpritString = selectedMove.ToString();
                UE_LOG(LogTemp, Error, TEXT("\nSimulated state empty! Culprit: %s"), *culpritString);
                return false;
            }
            currentState = simmedState;//ruleSet->NextState(currentState, selectedMove);
            depth++;
        }

        // if (depth >= maxSimulationDepth) 
            // UE_LOG(LogTemp, Display, TEXT("\nSimulation hit max depth. Guessing outcome from here."));

        // UE_LOG(LogTemp, Display, TEXT("\n*****************************Finished Simulation************************************"));

        return ruleSet->EvaluateTerminalState(currentState, currentState.actingPlayerIndex);
    }

    void Update(UMCTSNode* node, bool win) {
        node->Update(win);
    }

    UMCTSNode* Select(UMCTSNode* node, bool stopOnUnexplored = true) {
        node->selectionCount++;

        // keep selecting until we get to a node w/ unexplored children OR a terminal node.
        int selectionDepth = 0;
        while (selectionDepth < maxSimulationDepth && !ruleSet->IsTerminalState(node->state) && (!stopOnUnexplored || node->children.Num() == ruleSet->EnumerateMoves(node->state).Num())) {
            selectionDepth++;
            // UE_LOG(LogTemp, Display, TEXT("\nIn Selection, Traversing:\n%s"), *DebugNodeString(node));
            float UCB1Value = -1.0f;
            UMCTSNode* selectedChild = nullptr;
            FString UCBDebug = TEXT("UCB1 Scores: ");
            for (const auto& pair : node->children) {
                UMCTSNode* child = pair.Value;
                float ucb1 = UCB1(child);

                //disprefer idleness!
                if (pair.Key.Mid(8, 1).Equals(TEXT("-"))) {
                    ucb1 = -0.5f;
                }

                if (ucb1 > UCB1Value) {
                    UCB1Value = ucb1;
                    selectedChild = child;
                }
                UCBDebug += FString::Printf(TEXT("%f "), ucb1);
            }
            // UE_LOG(LogTemp, Display, TEXT("\n%s"), *UCBDebug);
            if (selectedChild) {
                node = selectedChild;
                node->selectionCount++;
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("Failed to select a child in this node:\n%s"), *DebugNodeString(node));
                break;
            }
        }

        //if we happen upon a terminal node, set it AND its parent's score to extremes?
        if (ruleSet->IsTerminalState(node->state)) {
            // UE_LOG(LogTemp, Warning, TEXT("\n[BUG] Selected a terminal node. Infinite wins here!"));
            if (ruleSet->EvaluateTerminalState(node->state, node->state.actingPlayerIndex)) {
                UMCTSNode* updatingNode = node;
                while (updatingNode && updatingNode->state.actingPlayerIndex == node->state.actingPlayerIndex) {
                    updatingNode->winCount = FP_INFINITE;
                    updatingNode->selectionCount = FP_INFINITE;
                    if (updatingNode->parent && updatingNode->parent->state.actingPlayerIndex != node->state.actingPlayerIndex)
                        node->parent->winCount = -FP_INFINITE;
                    updatingNode = updatingNode->parent;
                }
            }
        }
        
        return node;
    }

    float UCB1(UMCTSNode* node) {
        if (node->selectionCount <= 0)
            return std::numeric_limits<float>::infinity();
        float exploitation = static_cast<float>(node->winCount) / node->selectionCount;
        float exploration = 2 * std::sqrt(std::log(static_cast<float>(node->parent->selectionCount)) / node->selectionCount);
        return exploitation + exploration;
    }

    int playerIndex;
    int maxSimulationDepth;
    int decisionBudget;
    int playoutBudget;

    // Saved decision tree, used for follow-up decisions.
    UMCTSNode* rootNode;
};