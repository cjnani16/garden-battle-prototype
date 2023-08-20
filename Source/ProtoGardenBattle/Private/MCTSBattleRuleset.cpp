#include "MCTSBattleRuleset.h"
#include "Math/Vector2D.h"
#include "GenericPlatform/GenericPlatformMath.h"

void FMCTSBattleRuleset::IngestMoveSets(TArray<FGeneratedMove> _playerMoveList, TArray<FGeneratedMove> _opponentMoveList, TArray<FGeneratedMove> _systemMoveList)
{
	playerMoveList = _playerMoveList;
	opponentMoveList = _opponentMoveList;
	systemMoveList = _systemMoveList;
}

void FMCTSBattleRuleset::ApplyPlatformStatusTriggersToState(const int castersIndex, const FMCTSMove& move, FMCTSGameState& inputState, bool& overrideJump)
{
	overrideJump = false;

	// make sure move is a jump
	if (move.moveIndex != 0) {
		return;
	}

	// get platform indeces for jump off and landing platforms
	TArray<FVector2D> platformCoordinatesArray = {
		FVector2D(0,0), FVector2D(0,1), FVector2D(0,2),
		FVector2D(1,0), FVector2D(1,1), FVector2D(1,2),
		FVector2D(2,0), FVector2D(2,1), FVector2D(2,2)
	};

	int jumpPlatformIndex = platformCoordinatesArray.Find(inputState.monsterStates[castersIndex].position);
	FVector2D jumpTarget = move.targets[0].target;
	int landPlatformIndex = platformCoordinatesArray.Find(jumpTarget);

	// check statuses on the jump off platform
	for (EMCTSPlatformStatusTypes status : inputState.platformStates[jumpPlatformIndex].statuses) {
		switch (status) {
			case EMCTSPlatformStatusTypes::Sandtrap:
				overrideJump = true;
				return;
		}
	}

	// check statuses on the landing platform
	TArray<FMCTSMove> additionalMoves = {};
	for (EMCTSPlatformStatusTypes status : inputState.platformStates[landPlatformIndex].statuses) {
		switch (status) {
			case EMCTSPlatformStatusTypes::Freeze: {
				FMCTSMove slip = FMCTSMove(castersIndex);
				slip.targets = { FMCTSMoveTargetingData(0, jumpTarget) };
				slip.moveIndex = -2;
				additionalMoves.Add(slip);
				break;
			}
			case EMCTSPlatformStatusTypes::Ignite: {
				FMCTSMove stamp = FMCTSMove(castersIndex);
				stamp.targets = { FMCTSMoveTargetingData(0, jumpTarget) };
				stamp.moveIndex = -3;
				additionalMoves.Add(stamp);
				break;
			}
			case EMCTSPlatformStatusTypes::Flood: {
				FMCTSMove splash = FMCTSMove(castersIndex);
				splash.targets = { FMCTSMoveTargetingData(0, jumpTarget) };
				splash.moveIndex = -4;
				additionalMoves.Add(splash);
				break;
			}
		}
	}

	for (auto m : additionalMoves) {
		inputState = NextState(inputState, m);
	}
}

TArray<FMCTSMoveTargetingData> FillMoveTargets(const TArray<FMCTSMoveTargetingData> targets, const TArray<EGeneratedMoveTargetSelectorTypes> selectors, const FVector2D ownPosition, const FVector2D opponentPosition)
{
	TArray<FMCTSMoveTargetingData> newTargetingData = {};

	for (int targetingDataIndex = 0; targetingDataIndex < targets.Num(); targetingDataIndex++) {
		FMCTSMoveTargetingData currentTargetingData = targets[targetingDataIndex];
		
		TArray<FVector2D> newTargets = {};

		switch (selectors[currentTargetingData.selectorIndex]) {
			case EGeneratedMoveTargetSelectorTypes::RandomAny:
			{
				TArray<FVector2D> randomTargetOptions = {
					FVector2D(0,0), FVector2D(0,1), FVector2D(0,2),
					FVector2D(1,0), FVector2D(1,1), FVector2D(1,2),
					FVector2D(2,0), FVector2D(2,1), FVector2D(2,2)
				};
				int randomChoice = FMath::RandRange(0, randomTargetOptions.Num()-1);
				newTargets.Add(randomTargetOptions[randomChoice]);
			}
				
			case EGeneratedMoveTargetSelectorTypes::RandomOccupied:
			{
				TArray<FVector2D> randomTargetOptions = {
					ownPosition, opponentPosition
				};
				int randomChoice = FMath::RandRange(0, randomTargetOptions.Num() - 1);
				newTargets.Add(randomTargetOptions[randomChoice]);
			}

			case EGeneratedMoveTargetSelectorTypes::RandomAdjacent:
			{
				TArray<FVector2D> randomTargetOptions = {
					FVector2D(1,0), FVector2D(0,1),
					FVector2D(-1,0), FVector2D(0,-1)
				};
				int randomChoice = FMath::RandRange(0, randomTargetOptions.Num() - 1);
				FVector2D adjacentPosition = (ownPosition + randomTargetOptions[randomChoice]).ClampAxes(0,2);
				if (adjacentPosition != ownPosition)
					newTargets.AddUnique(adjacentPosition);
			}

			case EGeneratedMoveTargetSelectorTypes::Opponent:
				newTargets.Add(opponentPosition);

			case EGeneratedMoveTargetSelectorTypes::Own:
				newTargets.Add(ownPosition);

			case EGeneratedMoveTargetSelectorTypes::Line2:
			{
				//if (ownPosition == FVector2D(2.0f, 2.0f)) {
				//	UE_LOG(LogTemp, Error, TEXT("\nFilling a line targeter at (%f, %f) using initial target (%f, %f)"), ownPosition.X, ownPosition.Y, targets[targetingDataIndex].target.X, targets[targetingDataIndex].target.Y);
				//}

				FVector2D direction = targets[targetingDataIndex].target - ownPosition;
				FVector2D newPoint = (targets[targetingDataIndex].target + direction).ClampAxes(0, 2);

				newTargets.Add(targets[targetingDataIndex].target);
				newTargets.AddUnique(newPoint);
			}

			case EGeneratedMoveTargetSelectorTypes::Line3:
			{
				//if (ownPosition == FVector2D(2.0f, 2.0f)) {
				//	UE_LOG(LogTemp, Error, TEXT("\nFilling a line targeter at (%f, %f) using initial target (%f, %f)"), ownPosition.X, ownPosition.Y, targets[targetingDataIndex].target.X, targets[targetingDataIndex].target.Y);
				//}

				FVector2D direction = targets[targetingDataIndex].target - ownPosition;
				FVector2D newPoint = (targets[targetingDataIndex].target + direction).ClampAxes(0, 2);

				newTargets.Add(targets[targetingDataIndex].target);
				newTargets.AddUnique(newPoint);

				FVector2D newPoint2 = (newPoint + direction).ClampAxes(0, 2);
				newTargets.AddUnique(newPoint2);
			}

			case EGeneratedMoveTargetSelectorTypes::AllAdjacent:
			{
				TArray<FVector2D> adjacencyOffsets = {
					FVector2D(1,0), FVector2D(0,1),
					FVector2D(-1,0), FVector2D(0,-1)
				};
				for (FVector2D offset : adjacencyOffsets) {
					FVector2D adjacentPosition = (ownPosition + offset).ClampAxes(0, 2);
					if (adjacentPosition != ownPosition)
						newTargets.AddUnique(adjacentPosition);
				}
			}

			default:
			{
				// Any selectors that don't need to be filled pass through here unchanged.
				newTargets.Add(targets[targetingDataIndex].target);
			}
		}

		for (FVector2D t : newTargets) {
			newTargetingData.Add(FMCTSMoveTargetingData(currentTargetingData.selectorIndex, t));
		}
	}

	

	

	return newTargetingData;
}

FMCTSPlatformState GetChangedPlatformState(const FMCTSPlatformState inputState, EGeneratedMoveEffectTypes currentEffectType, float modulatedCurrentEffectPower)
{
	FMCTSPlatformState outputState = inputState;

	switch (currentEffectType) {
		case EGeneratedMoveEffectTypes::ChangeTemp:
			outputState.temp = FMath::Clamp(outputState.temp + modulatedCurrentEffectPower, 0.0f, 1.0f);
			if (outputState.temp >= 1.0f) {
				outputState.statuses.AddUnique(EMCTSPlatformStatusTypes::Ignite);
			}
			else if (modulatedCurrentEffectPower < 0) {
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Ignite);
			}

			if (outputState.temp <= 0.0f) {
				outputState.statuses.AddUnique(EMCTSPlatformStatusTypes::Freeze);
			}
			else if (modulatedCurrentEffectPower > 0) {
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Freeze);
			}
			break;
		case EGeneratedMoveEffectTypes::ChangeHum:
			outputState.hum = FMath::Clamp(outputState.hum + modulatedCurrentEffectPower, 0.0f, 1.0f);
			if (outputState.hum >= 1.0f) {
				outputState.statuses.AddUnique(EMCTSPlatformStatusTypes::Flood);
			}
			else if (modulatedCurrentEffectPower < 0) {
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Flood);
			}

			if (outputState.hum <= 0.0f) {
				outputState.statuses.AddUnique(EMCTSPlatformStatusTypes::Sandtrap);
			}
			else if (modulatedCurrentEffectPower > 0) {
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Sandtrap);
			}
			break;
		case EGeneratedMoveEffectTypes::ChangeElev:
			outputState.elev = FMath::Clamp(outputState.elev + modulatedCurrentEffectPower, 0.0f, 1.0f);
			break;
		case EGeneratedMoveEffectTypes::Lockdown:
			if (modulatedCurrentEffectPower > 0)
				outputState.statuses.Add(EMCTSPlatformStatusTypes::Lockdown);
			else
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Lockdown);
			break;
		case EGeneratedMoveEffectTypes::Freeze:
			if (modulatedCurrentEffectPower > 0)
				outputState.statuses.Add(EMCTSPlatformStatusTypes::Freeze);
			else
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Freeze);
			break;
		case EGeneratedMoveEffectTypes::Sandtrap:
			if (modulatedCurrentEffectPower > 0)
				outputState.statuses.Add(EMCTSPlatformStatusTypes::Sandtrap);
			else
				outputState.statuses.Remove(EMCTSPlatformStatusTypes::Sandtrap);
			break;
	}

	return outputState;
}

FMCTSMonsterState GetChangedMonsterState(const FMCTSMonsterState inputState, FVector2D targetCoords, EGeneratedMoveEffectTypes currentEffectType, float modulatedCurrentEffectPower)
{
	FMCTSMonsterState outputState = inputState;

	switch (currentEffectType) {
		case EGeneratedMoveEffectTypes::ChangeAtk:
			outputState.atk += FGenericPlatformMath::Max(0, modulatedCurrentEffectPower * 100.0f);
			break;
		case EGeneratedMoveEffectTypes::ChangeDef:
			outputState.def += FGenericPlatformMath::Max(0, modulatedCurrentEffectPower * 100.0f);
			break;
		case EGeneratedMoveEffectTypes::ChangeSpd:
			outputState.spd += FGenericPlatformMath::Max(0, modulatedCurrentEffectPower * 100.0f);
			break;
		case EGeneratedMoveEffectTypes::ChangeTemp:
			outputState.temp += modulatedCurrentEffectPower * outputState.temp;
			break;
		case EGeneratedMoveEffectTypes::ChangeHum:
			outputState.hum += modulatedCurrentEffectPower * outputState.hum;
			break;
		case EGeneratedMoveEffectTypes::ChangeElev:
			outputState.elev += modulatedCurrentEffectPower * outputState.elev;
			break;
		case EGeneratedMoveEffectTypes::PullPush:
		case EGeneratedMoveEffectTypes::MoveTo:
			outputState.position = targetCoords;
			break;
	}

	return outputState;
}

FMCTSMonsterState ComputeMonsterStateFromPlatformState(const FMCTSMonsterState inputState, const FMCTSPlatformState platformState)
{
	FMCTSMonsterState outputState = inputState;

	float newScore = (
		FGenericPlatformMath::Abs(platformState.temp - outputState.temp) +
		FGenericPlatformMath::Abs(platformState.hum - outputState.hum) +
		FGenericPlatformMath::Abs(platformState.elev - outputState.elev)
		) / 3.0f;

	newScore /= 0.01f * outputState.def;

	newScore = FMath::Lerp(0.0f, 1.0f, newScore);
	newScore = FMath::Clamp(newScore, 0.0f, 1.0f) * 100.0f;

	float scoreRatio = (newScore / outputState.score);

	outputState.atk = outputState.atk * scoreRatio;
	// outputState.def = outputState.def * scoreRatio;
	outputState.spd = outputState.spd * scoreRatio;

	return outputState;
}

FMCTSGameState FMCTSBattleRuleset::NextState(const FMCTSGameState& state, const FMCTSMove& move)
{
	FMCTSGameState resultingState = state;

	bool actingPlayerIsFaster = resultingState.monsterStates[state.actingPlayerIndex].spd > resultingState.monsterStates[1-state.actingPlayerIndex].spd;

	if (move.moveIndex == -1) {
		if (!actingPlayerIsFaster)
			resultingState.turnCount++;
		resultingState.actingPlayerIndex = 1 - resultingState.actingPlayerIndex;
		resultingState.monsterStates[0].ap = 2;
		resultingState.monsterStates[1].ap = 2;
		return resultingState;
	}

	int castersIndex = state.actingPlayerIndex;
	int opponentsIndex = 1 - castersIndex; 

	TArray<FVector2D> platformCoordinatesArray = { 
		FVector2D(0,0), FVector2D(0,1), FVector2D(0,2), 
		FVector2D(1,0), FVector2D(1,1), FVector2D(1,2), 
		FVector2D(2,0), FVector2D(2,1), FVector2D(2,2) 
	};

	TArray<FGeneratedEffectList> effectLists;
	TArray<EGeneratedMoveTargetSelectorTypes> selectors;

	if (move.moveIndex >= 0) {
		effectLists =
			(state.actingPlayerIndex == 0 ?
				playerMoveList[move.moveIndex].effectLists :
				opponentMoveList[move.moveIndex].effectLists);

		selectors =
			(state.actingPlayerIndex == 0 ?
				playerMoveList[move.moveIndex].selectors :
				opponentMoveList[move.moveIndex].selectors);
	}
	else {
		int systemMoveListIndex =  (0 - move.moveIndex) - 2;
		effectLists = systemMoveList[systemMoveListIndex].effectLists;
		selectors = systemMoveList[systemMoveListIndex].selectors;
	}

	// Take the move targeting data and fill in any additional targets based on the selector
	// TODO: if positions are changing throughout the below effects list, this should be updated. 
	TArray<FMCTSMoveTargetingData> filledMoveTargets = FillMoveTargets(move.targets, selectors, resultingState.monsterStates[castersIndex].position, resultingState.monsterStates[opponentsIndex].position);

	for (FMCTSMoveTargetingData moveTargetingData : filledMoveTargets) {
		FVector2D currentTargetCoords = moveTargetingData.target;
		int currentTargetPlatformIndex = platformCoordinatesArray.Find(currentTargetCoords);

		if (currentTargetPlatformIndex < 0) {
			UE_LOG(LogTemp, Error, TEXT("Couldn't find platform index for coordinates: (%d, %d)"), 
				static_cast<int>(currentTargetCoords.X), static_cast<int>(currentTargetCoords.Y));
		}


		bool gateNextEffect = false;
		bool overwriteNextEffectPower = false;
		float storedEffectPower = 0;
		for (FGeneratedEffect currentEffect : effectLists[moveTargetingData.selectorIndex].effects) {

			// Get the current effect's info
			float currentEffectPower = currentEffect.power;
			EGeneratedMoveEffectTypes currentEffectType = currentEffect.type;

			// Skip this effect if there's a pending gate
			if (gateNextEffect) {
				gateNextEffect = false;
				continue;
			}

			// Overwrite this effect's power if there's a pending stored power
			if (overwriteNextEffectPower) {
				overwriteNextEffectPower = false;
				currentEffectPower = storedEffectPower;
			}

			// Snapshot the current state of monsters (may have been changed by prior effects in this loop)
			FMCTSMonsterState casterMonsterState = resultingState.monsterStates[castersIndex];
			FMCTSMonsterState opponentMonsterState = resultingState.monsterStates[opponentsIndex];

			// Modulate current effect power based on stats, and move from 0-100 range to 0.0f - 1.0f range.
			currentEffectPower = currentEffectPower / 100.0f;
			float modulatedCurrentEffectPower = currentEffectPower * ((casterMonsterState.atk * 0.01f) + 0.5f);

			switch (currentEffectType) {

			// Affect platform state
			case EGeneratedMoveEffectTypes::ChangeTemp:
			case EGeneratedMoveEffectTypes::ChangeHum:
			case EGeneratedMoveEffectTypes::ChangeElev:
			case EGeneratedMoveEffectTypes::Lockdown:
			case EGeneratedMoveEffectTypes::Freeze:
			case EGeneratedMoveEffectTypes::Sandtrap:
				resultingState.platformStates[currentTargetPlatformIndex] = GetChangedPlatformState(resultingState.platformStates[currentTargetPlatformIndex], currentEffectType, modulatedCurrentEffectPower);
				break;


			// Change power of next effect
			case EGeneratedMoveEffectTypes::StoreTemp:
				storedEffectPower = resultingState.platformStates[currentTargetPlatformIndex].temp * modulatedCurrentEffectPower;
				overwriteNextEffectPower = true;
				break;
			case EGeneratedMoveEffectTypes::StoreHum:
				storedEffectPower = resultingState.platformStates[currentTargetPlatformIndex].hum * modulatedCurrentEffectPower;
				overwriteNextEffectPower = true;
				break;
			case EGeneratedMoveEffectTypes::StoreElev:
				storedEffectPower = resultingState.platformStates[currentTargetPlatformIndex].elev * modulatedCurrentEffectPower;
				overwriteNextEffectPower = true;
				break;

			// Gate based on thresholds
			case EGeneratedMoveEffectTypes::GateTemp:
				gateNextEffect = currentEffectPower < 0 ?
					resultingState.platformStates[currentTargetPlatformIndex].temp >= -currentEffectPower :
					resultingState.platformStates[currentTargetPlatformIndex].temp <= currentEffectPower;
				break;
			case EGeneratedMoveEffectTypes::GateHum:
				gateNextEffect = currentEffectPower < 0 ?
					resultingState.platformStates[currentTargetPlatformIndex].hum >= -currentEffectPower :
					resultingState.platformStates[currentTargetPlatformIndex].hum <= currentEffectPower;
				break;
			case EGeneratedMoveEffectTypes::GateElev:
				gateNextEffect = currentEffectPower < 0 ?
					resultingState.platformStates[currentTargetPlatformIndex].elev >= -currentEffectPower :
					resultingState.platformStates[currentTargetPlatformIndex].elev <= currentEffectPower;
				break;

			// Affect monster at targeted platform
			case EGeneratedMoveEffectTypes::ChangeAtk:
			case EGeneratedMoveEffectTypes::ChangeDef:
			case EGeneratedMoveEffectTypes::ChangeSpd:
				for (int monsterStateIndex = 0; monsterStateIndex < resultingState.monsterStates.Num(); monsterStateIndex++) {
					if (resultingState.monsterStates[monsterStateIndex].position == currentTargetCoords)
						resultingState.monsterStates[monsterStateIndex] = GetChangedMonsterState(resultingState.monsterStates[monsterStateIndex], currentTargetCoords, currentEffectType, modulatedCurrentEffectPower);
				}
				break;

			// Affect opponent monster
			case EGeneratedMoveEffectTypes::PullPush: 
				{FVector2D resultingLocation = (resultingState.monsterStates[opponentsIndex].position - resultingState.monsterStates[opponentsIndex].position * modulatedCurrentEffectPower).ClampAxes(0, 2);
				resultingState.monsterStates[opponentsIndex] = GetChangedMonsterState(resultingState.monsterStates[opponentsIndex], resultingLocation, currentEffectType, modulatedCurrentEffectPower);}
				break;

			// Affect caster monster
			case EGeneratedMoveEffectTypes::MoveTo: {
					resultingState.monsterStates[castersIndex] = GetChangedMonsterState(resultingState.monsterStates[castersIndex], currentTargetCoords, currentEffectType, modulatedCurrentEffectPower);
				}
				break;

			}
			
		}
	}

	// Subtract the cost of the move from their AP
	resultingState.monsterStates[castersIndex].ap -= move.cost;

	// Apply platform states to monster states
	for (int monsterStateIndex = 0; monsterStateIndex < resultingState.monsterStates.Num(); monsterStateIndex++) {
		const FVector2D currentMonsterPosition = resultingState.monsterStates[monsterStateIndex].position;
		int currentPlatformIndex = platformCoordinatesArray.Find(currentMonsterPosition);

		if (currentPlatformIndex < 0) {
			UE_LOG(LogTemp, Error, TEXT("Couldn't find platform index for coordinates: (%d, %d)"),
				static_cast<int>(currentMonsterPosition.X), static_cast<int>(currentMonsterPosition.Y));
		}

		resultingState.monsterStates[monsterStateIndex] = ComputeMonsterStateFromPlatformState(resultingState.monsterStates[monsterStateIndex], resultingState.platformStates[currentPlatformIndex]);
	}

	bool undoMove = false;
	ApplyPlatformStatusTriggersToState(castersIndex, move, resultingState, undoMove);

	if (undoMove) {
		resultingState = state;
		resultingState.monsterStates[castersIndex].ap -= 1;
		return resultingState;
	}

	return resultingState;
}

TArray<FMCTSMove> FMCTSBattleRuleset::EnumerateMoves(const FMCTSGameState& state) 
{
	int actingPlayerIndex = state.actingPlayerIndex;
	FVector2D playerPosition = state.monsterStates[actingPlayerIndex].position;
	FVector2D opponentPosition = state.monsterStates[1 - actingPlayerIndex].position;

	TArray<FMCTSMove> possibleMoves = {};

	auto moveList = state.actingPlayerIndex == 0 ? playerMoveList : opponentMoveList;

	for (int currentMoveIndex = 0; currentMoveIndex < moveList.Num(); currentMoveIndex++) {
		FGeneratedMove currentMove = moveList[currentMoveIndex];

		if (currentMove.cost > state.monsterStates[actingPlayerIndex].ap) 
			continue;

		for (int currentSelectorIndex = 0; currentSelectorIndex < currentMove.selectors.Num(); currentSelectorIndex++) {

			EGeneratedMoveTargetSelectorTypes currentMoveTargetSelector = currentMove.selectors[currentSelectorIndex];

			// TODO: Move this outside of the selectors list
			TArray<FVector2D> targetsToTry = {};

			switch (currentMoveTargetSelector) {
			case EGeneratedMoveTargetSelectorTypes::Any: 
				{
					for (int x = 0; x <= 2; x++) {
						for (int y = 0; y <= 2; y++) {
							targetsToTry.Add(FVector2D(x, y));
						}
					}
				}
				break;

			case EGeneratedMoveTargetSelectorTypes::Adjacent:
			case EGeneratedMoveTargetSelectorTypes::Line2:
			case EGeneratedMoveTargetSelectorTypes::Line3:
				{
					for (int x = -1; x <= 1; x++) {
						for (int y = -1; y <= 1; y++) {
							if (x == y || !(x == 0 || y == 0)) continue;
							FVector2D targetedPosition = FVector2D(x, y) + playerPosition;
							targetedPosition = targetedPosition.ClampAxes(0, 2);
							if (targetedPosition.X != playerPosition.X || targetedPosition.Y != playerPosition.Y) {
								targetsToTry.AddUnique(targetedPosition);
								if (playerPosition == FVector2D(2.0f, 2.0f) && targetedPosition == FVector2D(0, 0) && (currentMoveTargetSelector == EGeneratedMoveTargetSelectorTypes::Line2 || currentMoveTargetSelector == EGeneratedMoveTargetSelectorTypes::Line3)) {
									UE_LOG(LogTemp, Error, TEXT("\nEnumerating options for a line targeter at (%f, %f): adding initial target (%f, %f)"), playerPosition.X, playerPosition.Y, targetedPosition.X, targetedPosition.Y);
								}
							}

						}
					}
				}
				break;

			case EGeneratedMoveTargetSelectorTypes::Occupied:
				{
					targetsToTry.Add(playerPosition);
					targetsToTry.Add(opponentPosition);
				}
				break;

			case EGeneratedMoveTargetSelectorTypes::RandomAny:
			case EGeneratedMoveTargetSelectorTypes::RandomOccupied:
			case EGeneratedMoveTargetSelectorTypes::RandomAdjacent:
			case EGeneratedMoveTargetSelectorTypes::Opponent:
			case EGeneratedMoveTargetSelectorTypes::Own:
			case EGeneratedMoveTargetSelectorTypes::AllAdjacent:
				{
					targetsToTry.Add(FVector2D(0, 0));
				}
				break;
			}

			// TODO: Move this outside of the selectors list.
			for (FVector2D target : targetsToTry) {
				FMCTSMove newMove = FMCTSMove(actingPlayerIndex);
				newMove.moveIndex = currentMoveIndex;
				newMove.targets = {FMCTSMoveTargetingData(currentSelectorIndex, target)};
				newMove.playerIndex = actingPlayerIndex;
				newMove.cost = currentMove.cost;
				possibleMoves.Add(newMove);
			}
		}
	}

	// Add the 0-cost "End Turn" move to the possible moves list as well.
	possibleMoves.Add(FMCTSMove(actingPlayerIndex));

	return possibleMoves;
}

bool FMCTSBattleRuleset::IsTerminalState(const FMCTSGameState& state)
{
	return state.turnCount > 10;
}

bool FMCTSBattleRuleset::EvaluateTerminalState(const FMCTSGameState& state, int _playerIndex)
{
	return state.monsterStates[_playerIndex].score > state.monsterStates[1 - _playerIndex].score;
}

