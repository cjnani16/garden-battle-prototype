// Fill out your copyright notice in the Description page of Project Settings.


#include "MovesetGenerator.h"

#define stringify(s) #s

UMovesetGenerator::UMovesetGenerator()
{
	// Terms for combined elements. Pairs in each key are sorted alphabetically.
	elementalCombinationStringMappings = {
		{"FireWater", "Steam"},
		{"AridFire", "Sand"},
		{"AlpineFire", "Volcano"},
		{"FireLowland", "Geothermal"},
		{"IceWater", "Rain"},
		{"AridIce", "Wind"},
		{"AlpineIce", "Snowcap"},
		{"IceLowland", "Tundra"},
		{"AlpineWater", "Cloudtop"},
		{"LowlandWater", "Swamp"},
		{"AlpineArid", "Mesa"},
		{"AridLowland", "Plains"}
	};

	elementalColorMappings = {
	{"Fire", FLinearColor(FColor::FromHex("#FAA0A0"))},       // Pastel red
	{"Ice", FLinearColor(FColor::FromHex("#B0E0E6"))},        // Pastel blue
	{"Water", FLinearColor(FColor::FromHex("#ADD8E6"))},      // Pastel light blue
	{"Arid", FLinearColor(FColor::FromHex("#FAC898"))},       // Pastel orange
	{"Alpine", FLinearColor(FColor::FromHex("#ADC3D1"))},     // Pastel grey-blue
	{"Lowland", FLinearColor(FColor::FromHex("#FFD700"))},    // Pastel gold
	{"Steam", FLinearColor(FColor::FromHex("#F5F5F5"))},      // Pastel white
	{"Sand", FLinearColor(FColor::FromHex("#FFE4B5"))},       // Pastel sandy brown
	{"Volcano", FLinearColor(FColor::FromHex("#FF6347"))},    // Pastel tomato red
	{"Geothermal", FLinearColor(FColor::FromHex("#FFB6C1"))}, // Pastel light pink
	{"Rain", FLinearColor(FColor::FromHex("#00CED1"))},       // Pastel dark turquoise
	{"Wind", FLinearColor(FColor::FromHex("#87CEEB"))},       // Pastel sky blue
	{"Snowcap", FLinearColor(FColor::FromHex("#FFFACD"))},    // Pastel lemon chiffon
	{"Tundra", FLinearColor(FColor::FromHex("#D8BFD8"))},     // Pastel thistle
	{"Cloudtop", FLinearColor(FColor::FromHex("#B0C4DE"))},   // Pastel light steel blue
	{"Swamp", FLinearColor(FColor::FromHex("#C3B1E1"))},      // Pastel purple
	{"Mesa", FLinearColor(FColor::FromHex("#FFA07A"))},       // Pastel light salmon
	{"Plains", FLinearColor(FColor::FromHex("#98FB98"))},     // Pastel peach
	{"Lockdown", FLinearColor(FColor::FromHex("#949494"))},	  // Pastel grey
	{"Freezing", FLinearColor(FColor::FromHex("#B0E0E6"))},   // Pastel blue (same as ice)
	};
}

UMovesetGenerator::~UMovesetGenerator()
{
}

// Helper functions for move generation
FString EffectToString(FGeneratedEffect effect) {
	switch (effect.type) {
	case EGeneratedMoveEffectTypes::ChangeTemp:
		if (effect.power > 0)
			return "Fire";
		else
			return "Ice";
	case EGeneratedMoveEffectTypes::ChangeHum:
		if (effect.power > 0)
			return "Water";
		else
			return "Arid";
	case EGeneratedMoveEffectTypes::ChangeElev:
		if (effect.power > 0)
			return "Alpine";
		else
			return "Lowland";
	case EGeneratedMoveEffectTypes::ChangeAtk:
		if (effect.power > 0)
			return "Empowering";
		else
			return "Enfeebling";
	case EGeneratedMoveEffectTypes::ChangeDef:
		if (effect.power > 0)
			return "Reinforcing";
		else
			return "Penetrating";
	case EGeneratedMoveEffectTypes::ChangeSpd:
		if (effect.power > 0)
			return "Hastening";
		else
			return "Slowing";
	case EGeneratedMoveEffectTypes::Lockdown:
		return "Lockdown";
	case EGeneratedMoveEffectTypes::Freeze:
		return "Freezing";
	case EGeneratedMoveEffectTypes::MoveTo:
		return "Dynamic";
	default:
		return "Power";
	}
}

FString SelectorToString(EGeneratedMoveTargetSelectorTypes selector) {
	switch (selector) {
	case EGeneratedMoveTargetSelectorTypes::Any:
		return "Strike";
	case EGeneratedMoveTargetSelectorTypes::Adjacent:
		return "Punch";
	case EGeneratedMoveTargetSelectorTypes::Occupied:
		return "Mark";
	case EGeneratedMoveTargetSelectorTypes::Own:
		return "Slam";
	case EGeneratedMoveTargetSelectorTypes::Opponent:
		return "Snipe";
	case EGeneratedMoveTargetSelectorTypes::Line2:
		return "Charge";
	case EGeneratedMoveTargetSelectorTypes::Line3:
		return "Beam";
	case EGeneratedMoveTargetSelectorTypes::AllAdjacent:
		return "Blast";
	case EGeneratedMoveTargetSelectorTypes::RandomAny:
		return "Gambit";
	case EGeneratedMoveTargetSelectorTypes::RandomAdjacent:
		return "Flail";
	case EGeneratedMoveTargetSelectorTypes::RandomOccupied:
		return "Coinflip";
	default:
		return "Attack";
	}
}

FLinearColor UMovesetGenerator::GenerateMoveColor(FString name) {
	int i = name.Find(" ");
	FString key = name;
	if (i > 0) key = name.Left(i);

	if (elementalColorMappings.Contains(key)) {
		return elementalColorMappings[key];
	}
	return FLinearColor(FColor::FromHex("#FCE182"));
}

FString UMovesetGenerator::GenerateMoveName(FGeneratedMove move) {
	EGeneratedMoveTargetSelectorTypes selector = move.selectors[0];

	TArray<FString> elementTextList = {};

	for (auto effect : move.effectLists[0].effects)
	{
		elementTextList += {EffectToString(effect)};
	}

	elementTextList.Sort();

	FString elementText = FString::Join(elementTextList, TEXT(""));
	
	// Combine combo elements to a name based on the mapping
	if (elementalCombinationStringMappings.Contains(elementText)) {
		elementText = elementalCombinationStringMappings[elementText];
	}

	FString selectorText;
	selectorText = SelectorToString(selector);

	return FString::Printf(TEXT("%s %s"), *elementText, *selectorText);
}

FString MergeMoveDescriptions(FString a, FString b) {
	TArray<FString> aList;
	a.ParseIntoArray(aList, TEXT(" "), true);

	TArray<FString> bList;
	b.ParseIntoArray(bList, TEXT(" "), true);

	// Output word list
	TArray<FString> cList = {};
	UE_LOG(LogTemp, Display, TEXT("Preparing to make cList. a = %s, b = %s"), *a, *b);

	// Count matching words for the prefix, starting from the front inwards. Also add them to the list
	int prefixLen = 0;
	int searchMax = FMath::Min(aList.Num(), bList.Num());
	for (int i = 0; i < searchMax; i++) {
		if (aList[i] == bList[i]) {
			cList.Add(aList[i]);
			UE_LOG(LogTemp, Display, TEXT("Added to cList (prefix): %s"), *aList[i]);
			prefixLen = i;
		}
		else break;
	}

	// Count matching words for the suffix, counting from the back inwards
	int suffixLen = 0;
	for (int i = 1; i <= searchMax - prefixLen; i++) {
		if (aList[aList.Num() - i] == bList[bList.Num() - i]) {
			suffixLen += 1;
			UE_LOG(LogTemp, Display, TEXT("Counted for cList (suffix): %s"), *aList[aList.Num() - i]);
		}
		else break;
	}

	// Add the a-specific words to the list
	for (int i = prefixLen; i < aList.Num() - suffixLen; i++) {
		cList.Add(aList[i]);
		UE_LOG(LogTemp, Display, TEXT("Added to cList (a-specific): %s"), *aList[i]);
	}

	cList.Add("and");

	// Add the b-specific words to the list
	for (int i = prefixLen; i < bList.Num() - suffixLen; i++) {
		cList.Add(bList[i]);
		UE_LOG(LogTemp, Display, TEXT("Added to cList (b-specific): %s"), *bList[i]);
	}

	// Add the suffix to the list
	for (int i = aList.Num() - suffixLen; i < aList.Num();  i++) {
		cList.Add(aList[i]);
		UE_LOG(LogTemp, Display, TEXT("Added to cList (suffix): %s"), *aList[i]);
	}

	return FString::Join(cList, TEXT(" "));
}

FString GenerateMoveDescription(FGeneratedMove move) {
	FString description = "";

	for (int selectorIndex = 0; selectorIndex < move.selectors.Num(); selectorIndex++) {
		FString selectorText;

		switch (move.selectors[selectorIndex]) {
			case EGeneratedMoveTargetSelectorTypes::Adjacent:
				selectorText = "an adjacent platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::Any:
				selectorText = "a platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::Line2:
				selectorText = "a line of 2 platforms";
				break;
			case EGeneratedMoveTargetSelectorTypes::Line3:
				selectorText = "a line of 3 platforms";
				break;
			case EGeneratedMoveTargetSelectorTypes::Occupied:
				selectorText = "an occupied platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::Own:
				selectorText = "your own platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::RandomAdjacent:
				selectorText = "a random adjacent platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::RandomAny:
				selectorText = "a random platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::RandomOccupied:
				selectorText = "a random occupied platform";
				break;
			case EGeneratedMoveTargetSelectorTypes::AllAdjacent:
				selectorText = "all adjacent platforms";
				break;
			case EGeneratedMoveTargetSelectorTypes::Opponent:
				selectorText = "the opponent's platform";
				break;
		}

		for (int effectListIndex = 0; effectListIndex < move.effectLists[selectorIndex].effects.Num(); effectListIndex++) {
			FString powerText;
			auto effect = move.effectLists[selectorIndex].effects[effectListIndex];
			FString polarityString;
			FString powerString;
			FString effectDescriptionString;
			switch (effect.type) {
				case EGeneratedMoveEffectTypes::ChangeAtk:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the attack of monsters on %s by %s"), *polarityString, *selectorText, *powerString);
					break;
				case EGeneratedMoveEffectTypes::ChangeDef:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the defense of monsters on %s by %s"), *polarityString, *selectorText, *powerString);
					break;
				case EGeneratedMoveEffectTypes::ChangeSpd:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the speed of monsters on %s by %s"), *polarityString, *selectorText, *powerString);
					break;
				case EGeneratedMoveEffectTypes::ChangeTemp:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the temperature by |%s| degrees on %s"), *polarityString, *powerString, *selectorText);
					break;
				case EGeneratedMoveEffectTypes::ChangeHum:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the humidity by |%s|%% on %s"), *polarityString, *powerString, *selectorText);
					break;
				case EGeneratedMoveEffectTypes::ChangeElev:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("%s the elevation by |%s|' on %s"), *polarityString, *powerString, *selectorText);
					break;
				case EGeneratedMoveEffectTypes::Lockdown:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("lockdown %s for %s turns"), *selectorText, *powerString);
					break;
				case EGeneratedMoveEffectTypes::Freeze:
					polarityString = effect.power > 0 ? "raise" : "lower";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("freeze %s for %s turns"), *selectorText, *powerString);
					break;
				case EGeneratedMoveEffectTypes::MoveTo:
					polarityString = effect.power > 0 ? "towards" : "away";
					powerString = FString::FromInt(FMath::RoundToInt(effect.power));
					effectDescriptionString = FString::Printf(TEXT("move %s spaces %s %s"), *powerString, *polarityString, *selectorText);
					break;
			}

			// if there are more effects to come...
			if (effectListIndex > 0 || selectorIndex > 0) {
				description = MergeMoveDescriptions(description, effectDescriptionString);
			}
			else {
				description = effectDescriptionString;
			}
		}
		
	}

	description += ".";
	return description;
	
}

//TODO: rate these based on synergy with category.
TPair<int, EGeneratedMoveTargetSelectorTypes> RandomSelectorFromCategory(EGeneratedMoveTargetSelectorTypeCategories category)
{
	TArray< TPair<int, EGeneratedMoveTargetSelectorTypes>> options = {};
	switch (category)
	{
		case EGeneratedMoveTargetSelectorTypeCategories::TargetSelf:
			options += {
				{ 10, EGeneratedMoveTargetSelectorTypes::Own},
				{ 2, EGeneratedMoveTargetSelectorTypes::RandomAny },
				{ 5, EGeneratedMoveTargetSelectorTypes::RandomOccupied },
				{ 10, EGeneratedMoveTargetSelectorTypes::Any }};
		break;
		case EGeneratedMoveTargetSelectorTypeCategories::TargetNotSelf:
			options += {
				{5, EGeneratedMoveTargetSelectorTypes::Adjacent },
				{10, EGeneratedMoveTargetSelectorTypes::AllAdjacent },
				{10, EGeneratedMoveTargetSelectorTypes::Any },
				{10, EGeneratedMoveTargetSelectorTypes::Line2 },
				{1, EGeneratedMoveTargetSelectorTypes::Line3 },
				{1, EGeneratedMoveTargetSelectorTypes::Occupied },
				{1, EGeneratedMoveTargetSelectorTypes::Opponent },
				{1, EGeneratedMoveTargetSelectorTypes::RandomAdjacent } };
		break;
		case EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent:
			options += {
				{10, EGeneratedMoveTargetSelectorTypes::Opponent },
				{5, EGeneratedMoveTargetSelectorTypes::Adjacent },
				{8, EGeneratedMoveTargetSelectorTypes::AllAdjacent },
				{2, EGeneratedMoveTargetSelectorTypes::Own },
				{7, EGeneratedMoveTargetSelectorTypes::Line2 },
				{10, EGeneratedMoveTargetSelectorTypes::Line3 },
				{3, EGeneratedMoveTargetSelectorTypes::RandomAdjacent },
				{4, EGeneratedMoveTargetSelectorTypes::RandomOccupied },
				{1, EGeneratedMoveTargetSelectorTypes::RandomAny } };
		break;
		case EGeneratedMoveTargetSelectorTypeCategories::TargetNear:
			options += { 
				{8, EGeneratedMoveTargetSelectorTypes::Adjacent },
				{10, EGeneratedMoveTargetSelectorTypes::AllAdjacent },
				{10, EGeneratedMoveTargetSelectorTypes::Own }, 
				{10, EGeneratedMoveTargetSelectorTypes::Line2 },
				{10, EGeneratedMoveTargetSelectorTypes::Line3 },
				{3, EGeneratedMoveTargetSelectorTypes::RandomAdjacent } };
		break;
		case EGeneratedMoveTargetSelectorTypeCategories::TargetFar:
			options += {
				{1, EGeneratedMoveTargetSelectorTypes::RandomAny },
				{10, EGeneratedMoveTargetSelectorTypes::Any },
				{8, EGeneratedMoveTargetSelectorTypes::Line2 },
				{7, EGeneratedMoveTargetSelectorTypes::Line3 },
				{10, EGeneratedMoveTargetSelectorTypes::Occupied },
				{8, EGeneratedMoveTargetSelectorTypes::Opponent }};
		break;
		default:
			options += {
				{3, EGeneratedMoveTargetSelectorTypes::Adjacent },
				{10, EGeneratedMoveTargetSelectorTypes::AllAdjacent },
				{10, EGeneratedMoveTargetSelectorTypes::Any },
				{6, EGeneratedMoveTargetSelectorTypes::Line2 },
				{10, EGeneratedMoveTargetSelectorTypes::Line3 },
				{8, EGeneratedMoveTargetSelectorTypes::Occupied },
				{10, EGeneratedMoveTargetSelectorTypes::Opponent },
				{2, EGeneratedMoveTargetSelectorTypes::RandomAdjacent },
				{10, EGeneratedMoveTargetSelectorTypes::Own },
				{3, EGeneratedMoveTargetSelectorTypes::RandomOccupied },
				{1, EGeneratedMoveTargetSelectorTypes::RandomAny }};
		break;
	}

	return options[FMath::RandRange(0, options.Num() - 1)];
}

void UMovesetGenerator::GenerateMoveset(FMonsterMoveGenerationInputs MonsterData, TArray<FGeneratedMove>& GeneratedMoveset, int numberOfMovesToGenerate)
{
	// Find this monster's top 3 elements

	float tempExtremity = fminf(1.0f - MonsterData.temp, MonsterData.temp);
	float humExtremity = fminf(1.0f - MonsterData.hum, MonsterData.hum);
	float elevExtremity = fminf(1.0f - MonsterData.elev, MonsterData.elev);

	TArray<int> polaritiesList = { MonsterData.temp < 0.5f, MonsterData.hum < 0.5f, MonsterData.elev < 0.5f };
	TArray<float> extremeties = {tempExtremity, humExtremity, elevExtremity};

	float smallestDistance = 1.0f;
	int mostExtremeIndex = 0;
	int polarity1 = 0;
	for (auto i = 0; i < extremeties.Num(); i++) {
		if (extremeties[i] < smallestDistance) {
			smallestDistance = extremeties[i];
			mostExtremeIndex = i;
			polarity1 = polaritiesList[i];
		}
	}

	smallestDistance = 1.0f;
	int secondMostExtremeIndex = 0;
	int polarity2 = 0;
	for (auto i = 0; i < extremeties.Num(); i++) {
		if (extremeties[i] < smallestDistance && i != mostExtremeIndex) {
			smallestDistance = extremeties[i];
			secondMostExtremeIndex = i;
			polarity2 = polaritiesList[i];
		}
	}

	int thirdMostExtremeIndex = 0;
	int polarity3 = 0;
	for (auto i = 0; i < extremeties.Num(); i++) {
		if (i != mostExtremeIndex && i != secondMostExtremeIndex) {
			thirdMostExtremeIndex = i;
			polarity3 = polaritiesList[i];
		}
	}

	TArray<FString> elementNames = { FString(TEXT("Fire")),FString(TEXT("Ice")),FString(TEXT("Water")),FString(TEXT("Arid")),FString(TEXT("Alpine")),FString(TEXT("Lowland")) };
	TArray<EGeneratedMoveEffectTypes> effectTypeList = { EGeneratedMoveEffectTypes::ChangeTemp, EGeneratedMoveEffectTypes::ChangeHum, EGeneratedMoveEffectTypes::ChangeElev };
	
	FString numberOneElementName = elementNames[mostExtremeIndex * 2 + polarity1];
	FString numberTwoElementName = elementNames[secondMostExtremeIndex * 2 + polarity2];
	FString numberThreeElementName = elementNames[thirdMostExtremeIndex * 2 + polarity3];
	EGeneratedMoveEffectTypes numberOneEffect = effectTypeList[mostExtremeIndex];
	EGeneratedMoveEffectTypes numberTwoEffect = effectTypeList[secondMostExtremeIndex];
	EGeneratedMoveEffectTypes numberThreeEffect = effectTypeList[thirdMostExtremeIndex];

	UE_LOG(LogTemp, Display, TEXT("Mostextremeindex: %d, secondMostExtremeIndex: %d"), mostExtremeIndex, secondMostExtremeIndex);

	UE_LOG(LogTemp, Display, TEXT("Number one element: %s, Number two element: %s, Number three element: %s"), *numberOneElementName, *numberTwoElementName, *numberThreeElementName);

	// Determine this monster's strategy

	TArray<EMonsterStrategyClasses> strategyOptions = {};

	int THRESHOLD_HIGH_ATK = 50;
	int THRESHOLD_LOW_ATK = 10;
	int THRESHOLD_HIGH_DEF = 50;
	int THRESHOLD_LOW_DEF = 10;
	int THRESHOLD_HIGH_SPD = 50;
	int THRESHOLD_LOW_SPD = 10;

	// low attack strategies 
	if (MonsterData.atk < THRESHOLD_LOW_ATK) {
		strategyOptions += { EMonsterStrategyClasses::BTanker, EMonsterStrategyClasses::CDodger};
	}

	// low def strategies 
	else if (MonsterData.atk < THRESHOLD_LOW_DEF) {
		strategyOptions += { EMonsterStrategyClasses::DBombardierPoke};
		if (numberOneEffect == EGeneratedMoveEffectTypes::ChangeElev || numberTwoEffect == EGeneratedMoveEffectTypes::ChangeElev) {
			strategyOptions += {EMonsterStrategyClasses::EDisrupter};
		}
	}

	// low spd strategies 
	else if (MonsterData.atk < THRESHOLD_LOW_SPD) {
		strategyOptions += { EMonsterStrategyClasses::ASlammer, EMonsterStrategyClasses::DBombardierNuke };
	}

	// high attack strategies 
	else if (MonsterData.atk < THRESHOLD_HIGH_ATK) {
		strategyOptions += { EMonsterStrategyClasses::BTanker, EMonsterStrategyClasses::CDodger };
	}

	// high def strategies 
	else if (MonsterData.atk < THRESHOLD_HIGH_DEF) {
		strategyOptions += { EMonsterStrategyClasses::DBombardierPoke };
		if (numberOneEffect == EGeneratedMoveEffectTypes::ChangeElev || numberTwoEffect == EGeneratedMoveEffectTypes::ChangeElev) {
			strategyOptions += {EMonsterStrategyClasses::EDisrupter};
		}
	}

	// high spd strategies 
	else if (MonsterData.atk < THRESHOLD_HIGH_SPD) {
		strategyOptions += { EMonsterStrategyClasses::ASlammer, EMonsterStrategyClasses::DBombardierNuke };
	}

	//no notable stats? 
	else {
		strategyOptions += { EMonsterStrategyClasses::ASlammer, EMonsterStrategyClasses::BTanker, EMonsterStrategyClasses::CDodger, EMonsterStrategyClasses::DBombardierNuke, EMonsterStrategyClasses::DBombardierPoke, EMonsterStrategyClasses::EDisrupter };
	}

	FString strategyListString = "";
	for (auto strat : strategyOptions)
	{
		strategyListString.Append(TEXT(stringify(strat)));
	}

	UE_LOG(LogTemp, Display, TEXT("Potential strategies: %s"), *strategyListString);

	EMonsterStrategyClasses chosen_strategy = strategyOptions[FMath::RandRange(0, strategyOptions.Num()-1)];

	UE_LOG(LogTemp, Display, TEXT("Chosen strategy: %s"), stringify(chosen_strategy));

	// List effects and targeters that'd be synergistic w this strategy. Aka "Goals". Assign a weight to each.
	TArray<FMonsterStrategyGoal> goals = {};

	// Remap polarities used above from 0,1 to 1,-1
	EMonsterEffectPolarity numberOnePolarity = polarity1 == 0 ? EMonsterEffectPolarity::Raise : EMonsterEffectPolarity::Lower;
	EMonsterEffectPolarity numberTwoPolarity = polarity2 == 0 ? EMonsterEffectPolarity::Raise : EMonsterEffectPolarity::Lower;
	EMonsterEffectPolarity numberThreePolarity = polarity3 == 0 ? EMonsterEffectPolarity::Raise : EMonsterEffectPolarity::Lower;

	// Define a set of goals for each strategy, along with priorities for rating power.
	switch (chosen_strategy) {
		case EMonsterStrategyClasses::ASlammer:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeAtk,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		1));
			break;

		case EMonsterStrategyClasses::BTanker:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeDef,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetNear,		1));
			break;

		case EMonsterStrategyClasses::CDodger:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeDef,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		1));
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::MoveTo,		EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		3));
			break;

		case EMonsterStrategyClasses::DBombardierNuke:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeAtk,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeDef,	EMonsterEffectPolarity::Lower,			EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	1));
			break;

		case EMonsterStrategyClasses::DBombardierPoke:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeAtk,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeSpd,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetOpponent,	1));
			break;
		case EMonsterStrategyClasses::EDisrupter:
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeAtk,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::Lockdown,		EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetFar,		5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetFar,		3));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetFar,		1));
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("Strategy not supported."));
			goals.Add(FMonsterStrategyGoal(EGeneratedMoveEffectTypes::ChangeAtk,	EMonsterEffectPolarity::Raise,			EGeneratedMoveTargetSelectorTypeCategories::TargetSelf,		5));
			goals.Add(FMonsterStrategyGoal(numberOneEffect,							numberOnePolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		5));
			goals.Add(FMonsterStrategyGoal(numberTwoEffect,							numberTwoPolarity,						EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		5));
			goals.Add(FMonsterStrategyGoal(numberThreeEffect,						numberThreePolarity,					EGeneratedMoveTargetSelectorTypeCategories::TargetAny,		1));
			break;
	}

	// Randomly construct moves.
	for (int i = 0; i < numberOfMovesToGenerate; i++) {

		FGeneratedMove move = FGeneratedMove();

		int chosenGoalCost = FMath::RandRange(0, 4) == 4 ? 2 : 1;
		int chosenGoalCount = FMath::RandRange(1,2);
		int chosenGoalIndex = FMath::RandRange(0, goals.Num() - 1);

		auto chosenGoal = goals[chosenGoalIndex];

		TPair<int, EGeneratedMoveTargetSelectorTypes> chosenSelector = RandomSelectorFromCategory(chosenGoal.selectorCategory);

		float penaltyScale = (chosenGoal.priority * 2 + chosenSelector.Key) / (2 * chosenGoalCost);
		float penaltyMagnitude = 10.0f * chosenGoalCost;

		float powerPenalty = penaltyMagnitude - ((2 * penaltyMagnitude * penaltyScale) / 10.0f);
		
		float chosenGoalPower = 30 + powerPenalty;

		// Use multiples of 5
		chosenGoalPower = (FMath::CeilToFloat(chosenGoalPower / 5.0f)+1) * 5;

		UE_LOG(LogTemp, Display, TEXT("Potential strategies: %s"), *strategyListString);

		FGeneratedEffect effect = FGeneratedEffect(chosenGoal.effectType, chosenGoalPower * ((int)chosenGoal.effectPolarity - 1));

		move.selectors.Add(chosenSelector.Value);
		move.effectLists.Add(FGeneratedEffectList({ effect }));

		//sometimes turn a one-element effect into a two-element
		if (chosenGoalCount > 0 && chosenGoal.effectType == numberOneEffect || chosenGoal.effectType == numberTwoEffect || chosenGoal.effectType == numberThreeEffect) {
			TArray<EGeneratedMoveEffectTypes> effectOptionsOne = { numberOneEffect , numberTwoEffect , numberThreeEffect };
			TArray<EGeneratedMoveEffectTypes> effectOptionsTwo = { numberOneEffect , numberTwoEffect , numberThreeEffect };
			TArray<EMonsterEffectPolarity> polarityOptionsOne = { numberOnePolarity, numberTwoPolarity, numberThreePolarity };
			TArray<EMonsterEffectPolarity> polarityOptionsTwo = { numberOnePolarity, numberTwoPolarity, numberThreePolarity };
			int k = FMath::RandRange(0, 2);
			effectOptionsTwo.RemoveAt(k);
			polarityOptionsTwo.RemoveAt(k);
			int j = FMath::RandRange(0, 1);

			move.effectLists[0].effects.Empty();

			float firstEffectRatio = FMath::RandRange(0.4f, 0.6f);

			float firstEffectPower = firstEffectRatio * chosenGoalPower * ((int)polarityOptionsOne[k] - 1);
			firstEffectPower = (FMath::CeilToFloat(firstEffectPower / 5.0f)+1) * 5;
			
			float secondEffectPower = (1.0f - firstEffectRatio) * chosenGoalPower * ((int)polarityOptionsTwo[j] - 1);
			secondEffectPower = (FMath::CeilToFloat(secondEffectPower / 5.0f)+1) * 5;

			move.effectLists[0].effects.Add(FGeneratedEffect(effectOptionsOne[k], firstEffectPower));
			move.effectLists[0].effects.Add(FGeneratedEffect(effectOptionsTwo[j], secondEffectPower));
		}

		move.name = GenerateMoveName(move);
		move.description = GenerateMoveDescription(move);
		move.cost = chosenGoalCost;
		move.color = GenerateMoveColor(move.name);

		GeneratedMoveset.Add(move);
	}

	GeneratedMoveset;
}

FLinearColor UMovesetGenerator::GenerateMonsterBaseColor(float temp, float hum, float elev) {
	// Find top element
	float tempExtremity = fminf(1.0f - temp, temp);
	float humExtremity = fminf(1.0f - hum, hum);
	float elevExtremity = fminf(1.0f - elev, elev);

	TArray<int> polaritiesList = { temp < 0.5f, hum < 0.5f, elev < 0.5f };
	TArray<float> extremeties = { tempExtremity, humExtremity, elevExtremity };

	float smallestDistance = 1.0f;
	int mostExtremeIndex = 0;
	int polarity1 = 0;
	for (auto i = 0; i < extremeties.Num(); i++) {
		if (extremeties[i] < smallestDistance) {
			smallestDistance = extremeties[i];
			mostExtremeIndex = i;
			polarity1 = polaritiesList[i];
		}
	}

	smallestDistance = 1.0f;
	int secondMostExtremeIndex = 0;
	int polarity2 = 0;
	for (auto i = 0; i < extremeties.Num(); i++) {
		if (extremeties[i] < smallestDistance && i != mostExtremeIndex) {
			smallestDistance = extremeties[i];
			secondMostExtremeIndex = i;
			polarity2 = polaritiesList[i];
		}
	}

	TArray<EGeneratedMoveEffectTypes> effectTypeList = { EGeneratedMoveEffectTypes::ChangeTemp, EGeneratedMoveEffectTypes::ChangeHum, EGeneratedMoveEffectTypes::ChangeElev };

	EGeneratedMoveEffectTypes numberOneEffect = effectTypeList[mostExtremeIndex];
	EGeneratedMoveEffectTypes numberTwoEffect = effectTypeList[secondMostExtremeIndex];

	TArray<FString> effectsList = { EffectToString(FGeneratedEffect(numberOneEffect, (polarity1 * -2) + 1 )), EffectToString(FGeneratedEffect(numberTwoEffect, (polarity2 * -2) + 1)) };

	effectsList.Sort();

	FString key = FString::Join(effectsList, TEXT(""));

	// Combine dual elements into one name
	if (elementalCombinationStringMappings.Contains(key)) {
		key = elementalCombinationStringMappings[key];
	}

	// Set color based on element
	return GenerateMoveColor(key);
}