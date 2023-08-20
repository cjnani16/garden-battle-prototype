// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetArrayLibrary.h"
#include "MovesetGenerator.generated.h"

// Input data about the monster, used to guide strategy deduction.

USTRUCT(BlueprintType)
struct PROTOGARDENBATTLE_API FMonsterMoveGenerationInputs {
	GENERATED_BODY()

	FMonsterMoveGenerationInputs() : atk(50), def(50), spd(50), temp(0.5f), hum(0.5f), elev(0.5f), wgt(50) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int atk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int def;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int spd;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float temp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float hum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float elev;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int wgt;
};

// Enums + Structs for defining various strategies and declaring goals for the move generation.

UENUM(BlueprintType)
enum class EGeneratedMoveEffectTypes : uint8 {
	ChangeTemp,
	ChangeHum,
	ChangeElev,
	StoreTemp,
	StoreHum,
	StoreElev,
	Lockdown,
	Freeze,
	Sandtrap,
	GateTemp,
	GateHum,
	GateElev,
	ChangeAtk,
	ChangeDef,
	ChangeSpd,
	PullPush,
	MoveTo
};

UENUM(BlueprintType)
enum class EGeneratedMoveTargetSelectorTypeCategories : uint8 {
	TargetSelf,
	TargetNotSelf,
	TargetOpponent,
	TargetAny,
	TargetNear,
	TargetFar
};

UENUM(BlueprintType)
enum class EMonsterStrategyClasses : uint8 {
	ASlammer,
	BTanker,
	CDodger,
	DBombardierPoke,
	DBombardierNuke,
	EDisrupter
};

UENUM(BlueprintType)
enum class EMonsterEffectPolarity : uint8 {
	Raise = 2,
	Lower = 0
};

USTRUCT(BlueprintType)
struct PROTOGARDENBATTLE_API FMonsterStrategyGoal {
	GENERATED_BODY()

	FMonsterStrategyGoal() :
		effectType(EGeneratedMoveEffectTypes::ChangeAtk),
		effectPolarity(EMonsterEffectPolarity::Raise),
		selectorCategory(EGeneratedMoveTargetSelectorTypeCategories::TargetSelf),
		priority(1) {}

	FMonsterStrategyGoal(
		EGeneratedMoveEffectTypes _effectType,
		EMonsterEffectPolarity _effectPolarity,
		EGeneratedMoveTargetSelectorTypeCategories _selectorCategory,
		int _priority
	) :
		effectType(_effectType),
		effectPolarity(_effectPolarity),
		selectorCategory(_selectorCategory),
		priority(_priority) {}

	UPROPERTY()
		EGeneratedMoveEffectTypes effectType;
	UPROPERTY()
		EMonsterEffectPolarity effectPolarity;
	UPROPERTY()
		EGeneratedMoveTargetSelectorTypeCategories selectorCategory;
	UPROPERTY()
		int priority;
};

// Enums + Structs for defining moves (should match those used in Blueprints... Ideally Blueprints can be updated to use these instead but will probably just rely on conversions.)

UENUM(BlueprintType)
enum class EGeneratedMoveTargetSelectorTypes : uint8 {
	Any,
	Adjacent,
	RandomAny,
	RandomOccupied,
	RandomAdjacent,
	Occupied,
	Opponent,
	Own,
	Line2,
	Line3,
	AllAdjacent
};

USTRUCT(BlueprintType)
struct PROTOGARDENBATTLE_API FGeneratedEffect {
	GENERATED_BODY()

	FGeneratedEffect() : type(EGeneratedMoveEffectTypes::ChangeTemp), power(0.2f) {}
	FGeneratedEffect(EGeneratedMoveEffectTypes t, float p) : type(t), power(p) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EGeneratedMoveEffectTypes type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float power;

};

USTRUCT(BlueprintType)
struct PROTOGARDENBATTLE_API FGeneratedEffectList {
	GENERATED_BODY()

	FGeneratedEffectList() : effects({}) {}
	FGeneratedEffectList(TArray<FGeneratedEffect> e) : effects(e) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FGeneratedEffect> effects;
};

USTRUCT(BlueprintType)
struct PROTOGARDENBATTLE_API FGeneratedMove {
	GENERATED_BODY()

	FGeneratedMove() : name(TEXT("[GMove]")), description(TEXT("[GMoveDesc]")), color(FLinearColor(0.5f, 0.5f,0.5f)), selectors({}), effectLists({}), cost(1), style(TEXT("")) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<EGeneratedMoveTargetSelectorTypes> selectors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FGeneratedEffectList> effectLists;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int cost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString style;
};

// Moveset generator class with Blueprint callable function to request a moveset

UCLASS(BlueprintType)
class PROTOGARDENBATTLE_API UMovesetGenerator : public UObject
{
	GENERATED_BODY()
public:
	UMovesetGenerator();
	~UMovesetGenerator();
	UFUNCTION(BlueprintCallable)
		void GenerateMoveset(FMonsterMoveGenerationInputs MonsterData, TArray<FGeneratedMove>& GeneratedMoveset, int numberOfMovesToGenerate);
	UFUNCTION(BlueprintCallable)
		FLinearColor GenerateMonsterBaseColor(float temp, float hum, float elev);
private:
	TMap<FString, FString> elementalCombinationStringMappings;
	TMap<FString, FLinearColor> elementalColorMappings;
	FString GenerateMoveName(FGeneratedMove move);
	FLinearColor GenerateMoveColor(FString name);
};
