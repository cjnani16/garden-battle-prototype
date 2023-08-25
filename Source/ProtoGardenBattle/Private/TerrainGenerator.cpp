// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator.h"

UTerrainGenerator::UTerrainGenerator()
{
}

UTerrainGenerator::~UTerrainGenerator()
{
}

FIntVector CalculateBiomeKey(FLinearColor condition)
{
	// find the 2 most extreme conditions
	TArray<float> conditionExtremetiesSorted = { FMath::Abs(condition.R - 0.5f), FMath::Abs(condition.G - 0.5f), FMath::Abs(condition.B - 0.5f) };
	conditionExtremetiesSorted.StableSort();
	TArray<int> conditionSigns = { (int)(condition.R > 0.5f) * 2 - 1, (int)(condition.G > 0.5f) * 2 - 1, (int)(condition.B > 0.5f) * 2 - 1 };
	TArray<float> conditionValues = { FMath::Abs(condition.R - 0.5f), FMath::Abs(condition.G - 0.5f), FMath::Abs(condition.B - 0.5f) };
	int mostExtremeConditionIndex = conditionValues.Find(conditionExtremetiesSorted[2]);
	int secondMostExtremeConditionIndex = conditionValues.Find(conditionExtremetiesSorted[1]);

	FIntVector biomeKey = FIntVector(
		(int)(mostExtremeConditionIndex == 0 || secondMostExtremeConditionIndex == 0) * conditionSigns[0],
		(int)(mostExtremeConditionIndex == 1 || secondMostExtremeConditionIndex == 1) * conditionSigns[1],
		(int)(mostExtremeConditionIndex == 2 || secondMostExtremeConditionIndex == 2) * conditionSigns[2]
	);

	return biomeKey;
}

float CalculateHeight(const FVector position, const int MapSize, const FTerrainGenerationNoiseSettings GenerationSettings, FLinearColor condition)
{
	float CoastalHeightModifier = FMath::Max(0.0f, 1.0f - (FMath::Max(0.0f, FVector::Dist2D(position, FVector(0.5f, 0.5f, 0.0f)) - GenerationSettings.IslandUVRadius) / (GenerationSettings.IslandUVCoastalRadius)));

	float BiomeHeightModifier = (((FVector(CalculateBiomeKey(condition))+FVector::One())/2.0f).Z * GenerationSettings.ElevationConditionHeightContributionRatio) + (1.0f - GenerationSettings.ElevationConditionHeightContributionRatio);

	float ElevationFrequencyModifier = (condition.B * GenerationSettings.ElevationConditionFreqContributionRatio) + (1.0f - GenerationSettings.ElevationConditionFreqContributionRatio);

	float heightRaw = 0;
	float p = 0.5;
	float l = 2.0f;
	for (int o = 0; o < 3; o++) {
		heightRaw += FMath::Pow(p,o) * GenerationSettings.HeightFactor * CoastalHeightModifier * ((BiomeHeightModifier) * (/*(1.0f - GenerationSettings.ElevationConditionHeightContributionRatio) * */ (FMath::Pow((TerrainGenPerlinHelpers::PerlinNoise2D(GenerationSettings.XYScale * ElevationFrequencyModifier * (float)MapSize * FMath::Pow(2.0f, o) * FVector2D(position.X, position.Y)) / 2.0f) + 0.5f, GenerationSettings.PerlinPower))));
	}
	
	float heightTerraced = FMath::Floor(heightRaw / GenerationSettings.HeightTerracing) * GenerationSettings.HeightTerracing;
	return heightTerraced;
}

FLinearColor CalculateCondition(const FVector position, const int MapSize, const FTerrainGenerationNoiseSettings GenerationSettings, FVector offsetsPerCondition)
{
	float CoastalHeightModifier = FMath::Max(0.0f, 1.0f - (FMath::Max(0.0f, FVector::Dist2D(position, FVector(0.5f, 0.5f, 0.0f)) - GenerationSettings.IslandUVRadius) / (GenerationSettings.IslandUVCoastalRadius)));

	float temp = (FMath::Pow((TerrainGenPerlinHelpers::PerlinNoise2D(GenerationSettings.XYScale * GenerationSettings.IslandConditionChangeFreq * (float)MapSize * FVector2D(position.X + offsetsPerCondition.X, position.Y + offsetsPerCondition.X)) / 2.0f) + 0.5f, GenerationSettings.IslandConditionChangePower));
	float hum = (FMath::Pow((TerrainGenPerlinHelpers::PerlinNoise2D(GenerationSettings.XYScale * GenerationSettings.IslandConditionChangeFreq * (float)MapSize * FVector2D(position.X + offsetsPerCondition.Y, position.Y + offsetsPerCondition.Y)) / 2.0f) + 0.5f, GenerationSettings.IslandConditionChangePower));
	float elev = CoastalHeightModifier * (FMath::Pow((TerrainGenPerlinHelpers::PerlinNoise2D(GenerationSettings.XYScale * GenerationSettings.IslandConditionChangeFreq * (float)MapSize * FVector2D(position.X + offsetsPerCondition.Z, position.Y + offsetsPerCondition.Z)) / 2.0f) + 0.5f, GenerationSettings.IslandConditionChangePower));
	return FLinearColor(temp, hum, elev);
}


FIntVector UTerrainGenerator::GetBiomeFromWorldPosition(FVector location)
{

	FVector vertexUVPosition = FVector((location.X - BBoxMin.X) / (BBoxMax.X - BBoxMin.X), (location.Y - BBoxMin.Y) / (BBoxMax.Y - BBoxMin.Y), 0);

	return CalculateBiomeKey(CalculateCondition(vertexUVPosition, MapSize, GenerationSettings, OffsetsPerCondition));
}

void TryPlaceMaterial(FVector position, FVector currentVertexPosition, FLinearColor currentVertexCondition, FVector perConditionOffsets, const int MapSize, const FTerrainGenerationNoiseSettings GenerationSettings, FIslandMaterialDataCollection& IslandMaterialDataCollection)
{

	// Identify the biome for this location
	FIntVector biomeKey = CalculateBiomeKey(currentVertexCondition);

	// get offsets for perlin frmo the biome
	float biomePerlinOffset = perConditionOffsets[biomeKey.X + 1]*7.0f + perConditionOffsets[biomeKey.Y + 1]*11.0f + perConditionOffsets[biomeKey.Z + 1];

	//UE_LOG(LogTemp, Display, TEXT("  ...biome is : (%d,%d,%d)"), biomeKey.X, biomeKey.Y, biomeKey.Z);

	// Check the materials list for that biome
	int biomeIndex = IslandMaterialDataCollection.SupportedBiomes.Find(biomeKey);
	if (biomeIndex > 0) {
		FIslandMaterialList* materialList = &IslandMaterialDataCollection.PerBiomeMaterials[biomeIndex];
		
		int32 seed = FMath::RoundToInt32(currentVertexPosition.X * 0.1f + currentVertexPosition.Y * 0.7f);
		FRandomStream random = FRandomStream(seed);

		// Shuffle the list
		const int32 NumShuffles = materialList->List.Num() - 1;
		for (int32 i = 0; i < NumShuffles; ++i)
		{
			int32 SwapIdx = random.RandRange(i, NumShuffles);
			materialList->List.Swap(i, SwapIdx);
		}

		for (int materialListIndex = 0; materialListIndex < materialList->List.Num(); materialListIndex++) {
			
			FIslandMaterialData& mat = materialList->List[materialListIndex];

			//Calculate a probability for this material
			
			// increase frequency with lacunarity
			float spawnProbability = 0;
			
			
			spawnProbability += (FMath::Pow((
				TerrainGenPerlinHelpers::PerlinNoise2D(
					GenerationSettings.XYScale * 2.0f *
					FMath::Pow(IslandMaterialDataCollection.PerOctaveLacunarity, mat.octave) *
					(float)MapSize *
					FVector2D(position.X /* + biomePerlinOffset*/, position.Y /* + biomePerlinOffset*/)
				) / 2.0f) + 0.5f,
				GenerationSettings.PerlinPower*2.0f));
			

			// lower amplitude with persistence
			// spawnProbability *= FMath::Pow(IslandMaterialDataCollection.PerOctavePersistence, mat.octave);

			// shift phase with centrality
			//spawnProbability = (1.0f - FMath::Abs(spawnProbability - mat.centrality));
			spawnProbability *= IslandMaterialDataCollection.OverallMaterialDensity * mat.rarity;

			// threshold with rarity
			float Number = random.FRand();
			bool spawn = (spawnProbability > 0) && (spawnProbability  >= Number);

			//UE_LOG(LogTemp, Display, TEXT("Spawn chance for %s: %f, roll = %f"), *mat.meshAsset->GetFName().ToString(), spawnProbability, Number);

			if (spawn) {
				float widthScale = random.FRandRange(0.8f, 1.2f) * mat.meshScalingFactor.X;
				float heightScale = random.FRandRange(0.8f, 1.2f) * mat.meshScalingFactor.Z;

				//float moveLimit = (1.0f/(float)MapSize);

				//FVector shiftedPosition = FVector(position.X + random.FRandRange(-moveLimit, moveLimit), position.Y + random.FRandRange(-moveLimit, moveLimit), 0);

				//shiftedPosition.Z = CalculateHeight(shiftedPosition, MapSize, GenerationSettings, CalculateCondition(shiftedPosition, MapSize, GenerationSettings, perConditionOffsets));
				//shiftedPosition.X = shiftedPosition.X * 

				materialList->List[materialListIndex].instanceLocations.Add(
					FTransform(
						FRotator(0, random.FRandRange(0.0f, 360.0f), 0),
						currentVertexPosition + FVector(random.FRandRange(-15, 15), random.FRandRange(-15, 15), 0),
						FVector(widthScale, widthScale, heightScale)
					)
				);
				materialList->List[materialListIndex].name = mat.meshAsset ? mat.meshAsset->GetName() : FString::Printf(TEXT("POT_%d_%d"), biomeIndex, materialListIndex);
				break;
			}
		}
	}

}

void ResetSeed(int32 seed) {
	memcpy(TerrainGenPerlinHelpers::Permutation, TerrainGenPerlinHelpers::PermutationBase, sizeof(int32)*512);
	Algo::Rotate(TerrainGenPerlinHelpers::Permutation, seed);
	TerrainGenPerlinHelpers::Stream = FRandomStream(seed);
}

void UTerrainGenerator::GenerateNewIslandMesh(FTerrainGenCompletedDelegate Out, UDynamicMesh* IslandMesh, const FTransform IslandTransform, const int IslandTileSize, const FTerrainGenerationNoiseSettings BaseTerrainGenerationSettings, const FIslandMaterialDataCollection& InputIslandMaterialDataCollection)
{
	ResetSeed(BaseTerrainGenerationSettings.Seed);

	UE_LOG(LogTemp, Display, TEXT("(0/4) Terrain Generation Seed: %d"), BaseTerrainGenerationSettings.Seed);

	FIslandMaterialDataCollection IslandMaterialDataCollection = InputIslandMaterialDataCollection;
	MapSize = IslandTileSize;
	GenerationSettings = BaseTerrainGenerationSettings;
	OffsetsPerCondition = FVector(TerrainGenPerlinHelpers::Stream.FRandRange(-1000.2, 1000.3), TerrainGenPerlinHelpers::Stream.FRandRange(-1000.5, 1000.4), TerrainGenPerlinHelpers::Stream.FRandRange(-1000.1, 1000.0));
	FVector offsetsPerCondition = OffsetsPerCondition;

	// Get coordinate information
	FBox bbox = UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(IslandMesh);
	FVector min = bbox.Min * IslandTransform.GetScale3D();
	FVector max = bbox.Max * IslandTransform.GetScale3D();
	BBoxMin = min;
	BBoxMax = max;
	UE_LOG(LogTemp, Warning, TEXT("Terrain Generation Progress: BBox Min = (%f/%f), Max = (%f/%f)."), min.X, min.Y, max.X, max.Y);

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [Out, IslandMesh, IslandTransform, IslandTileSize, BaseTerrainGenerationSettings, IslandMaterialDataCollection, offsetsPerCondition, min, max]() mutable
		{
			UE_LOG(LogTemp, Display, TEXT("(1/4) Terrain Generation Progress: Tesselating..."));
			UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyUniformTessellation(IslandMesh, IslandTileSize - 1, nullptr);

			// Get mesh vertex and color lists to set
			FGeometryScriptVectorList VertexPositionList;
			bool bHasVertexIDGaps;
			UGeometryScriptLibrary_MeshQueryFunctions::GetAllVertexPositions(IslandMesh, VertexPositionList, false, bHasVertexIDGaps);

			FGeometryScriptColorList VertexColorList;
			bool bIsValidColorSet;
			UGeometryScriptLibrary_MeshVertexColorFunctions::GetMeshPerVertexColors(IslandMesh, VertexColorList, bIsValidColorSet, bHasVertexIDGaps, true);

			UE_LOG(LogTemp, Display, TEXT("(2/4) Terrain Generation Progress: Assigning Positions..."));
			auto maxVertexIndex = VertexPositionList.List->Num();
			for (auto vertexIndex = 0; vertexIndex < maxVertexIndex; vertexIndex++) {
				FVector currentVertexPosition = (*VertexPositionList.List)[vertexIndex];

				// Transform to local space.
				currentVertexPosition = UKismetMathLibrary::TransformLocation(IslandTransform, currentVertexPosition);

				FVector vertexUVPosition = FVector((currentVertexPosition.X - min.X)/(max.X - min.X), (currentVertexPosition.Y - min.Y)/(max.Y - min.Y), 0);

				FLinearColor currentVertexCondition = CalculateCondition(vertexUVPosition, IslandTileSize, BaseTerrainGenerationSettings, offsetsPerCondition);
				currentVertexPosition = FVector(currentVertexPosition.X, currentVertexPosition.Y, CalculateHeight(vertexUVPosition, IslandTileSize, BaseTerrainGenerationSettings, currentVertexCondition));

				TryPlaceMaterial(vertexUVPosition, currentVertexPosition, currentVertexCondition, offsetsPerCondition, IslandTileSize, BaseTerrainGenerationSettings, IslandMaterialDataCollection);

				// Transform back to world space
				currentVertexPosition = UKismetMathLibrary::InverseTransformLocation(IslandTransform, currentVertexPosition);

				// See if any materials should be placed here
				
				(*VertexPositionList.List)[vertexIndex] = currentVertexPosition;

				(*VertexColorList.List)[vertexIndex] = FLinearColor((FVector(CalculateBiomeKey(currentVertexCondition)) + FVector::One())/2.0f);
			
			}

			UGeometryScriptLibrary_MeshBasicEditFunctions::SetAllMeshVertexPositions(IslandMesh, VertexPositionList, nullptr);

			UGeometryScriptLibrary_MeshVertexColorFunctions::SetMeshPerVertexColors(IslandMesh, VertexColorList, nullptr);

			// UE_LOG(LogTemp, Display, TEXT("(4/4) Terrain Generation Progress: Tesselate and Smooth..."));
			//UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyUniformTessellation(IslandMesh, 3, nullptr);
			//FGeometryScriptIterativeMeshSmoothingOptions smoothingOptions = FGeometryScriptIterativeMeshSmoothingOptions();
			//UGeometryScriptLibrary_MeshDeformFunctions::ApplyIterativeSmoothingToMesh(IslandMesh, FGeometryScriptMeshSelection(), smoothingOptions, nullptr);

			UE_LOG(LogTemp, Display, TEXT("(4/4) Terrain Generation Progress: Recalculating Normals..."));
			UGeometryScriptLibrary_MeshNormalsFunctions::SetPerFaceNormals(IslandMesh, nullptr);

			UE_LOG(LogTemp, Display, TEXT("(4/4) Terrain Generation Progress: Done!"));

			// Choose a coastal position to start
			TArray<FVector> startPositions = {};
			for (float angle = 0; angle < 8; angle++) {

				FVector startPosition = 0.85f * FVector(max.X * FMath::Cos((PI/4.0f) * angle), max.Y * FMath::Sin((PI/4.0f) * angle), 0);

				FVector vertexUVPosition = FVector((startPosition.X - min.X) / (max.X - min.X), (startPosition.Y - min.Y) / (max.Y - min.Y), 0);

				FLinearColor currentVertexCondition = CalculateCondition(vertexUVPosition, IslandTileSize, BaseTerrainGenerationSettings, offsetsPerCondition);
				startPosition = FVector(startPosition.X, startPosition.Y, CalculateHeight(vertexUVPosition, IslandTileSize, BaseTerrainGenerationSettings, currentVertexCondition));//GeneratedHeightMap[heightMapIndex]);

				startPositions.Add(startPosition);
			}


			AsyncTask(ENamedThreads::GameThread, [Out, startPositions, IslandMaterialDataCollection]()
				{
					Out.ExecuteIfBound(startPositions, IslandMaterialDataCollection);
				}
			);

		}
	);

	return;
}