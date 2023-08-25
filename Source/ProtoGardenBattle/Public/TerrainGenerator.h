// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UDynamicMesh.h"
#include "Engine/DataTable.h"
#include "GeometryScript/GeometryScriptTypes.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshVertexColorFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Async/AsyncWork.h"
#include "Math/UnrealMath.h"
#include "Algo/Rotate.h"
#include "MathUtil.h"
#include "MovesetGenerator.h"
#include "TerrainGenerator.generated.h"

// Stat bonuses (apply to pots and materials)
USTRUCT(BlueprintType)
struct FIslandMaterialStatData : public FTableRowBase {
    GENERATED_BODY()

        FIslandMaterialStatData() : atk(0), def(0), spd(0), temp(0), hum(0), elev(0), moves({}) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float atk;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float def;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float spd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float temp;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float hum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float elev;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FGeneratedMove> moves;
};

//  Island materials (plants and whatnot) -- can also contain Pot data!
USTRUCT(BlueprintType)
struct FIslandMaterialData {
    GENERATED_BODY()

    FIslandMaterialData() : octave(0), centrality(1.0f), rarity(0.5f), pickupable(true), meshAsset(nullptr), meshScalingFactor(FVector::One()), stats(FIslandMaterialStatData()), instanceLocations({}) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int octave;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float centrality;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float rarity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool pickupable;

    // If this is blank, assume the material is a Pot!
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UStaticMesh* meshAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector meshScalingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FIslandMaterialStatData stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FTransform> instanceLocations;
};

USTRUCT(BlueprintType)
struct FIslandMaterialList : public FTableRowBase {
    GENERATED_BODY()

    FIslandMaterialList() : ConditionKey(FIntVector()), List({}) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FIntVector ConditionKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FIslandMaterialData> List;
};

USTRUCT(BlueprintType)
struct FIslandMaterialDataCollection : public FTableRowBase {
    GENERATED_BODY()

    FIslandMaterialDataCollection() : OverallMaterialDensity(0.01f), NumOctaves(3), PerOctavePersistence(0.5f), PerOctaveLacunarity(2.0f), SupportedBiomes({}), PerBiomeMaterials({})  {}

    // map biome to the list of materials found there.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float OverallMaterialDensity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int NumOctaves;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float PerOctavePersistence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float PerOctaveLacunarity;

    //biome keys are 3 ints: 1/-1 if top 2 element, 0 otherwise, for each of THE.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FIntVector> SupportedBiomes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FIslandMaterialList> PerBiomeMaterials;
};


// Settings for noise map
USTRUCT(BlueprintType)
struct FTerrainGenerationNoiseSettings {
    GENERATED_BODY()

    FTerrainGenerationNoiseSettings() : XYScale(0), HeightFactor(10), HeightTerracing(1), PerlinBias(1), PerlinPower(2), IslandUVRadius(0.2f), IslandUVCoastalRadius(0.29f), IslandConditionChangeFreq(0.15), IslandConditionChangePower(2), ElevationConditionHeightContributionRatio(0.7f), Seed(0) {}

    UPROPERTY(BlueprintReadWrite)
        float XYScale;
    UPROPERTY(BlueprintReadWrite)
        float HeightFactor;
    UPROPERTY(BlueprintReadWrite)
        float HeightTerracing;
    UPROPERTY(BlueprintReadWrite)
        float PerlinBias;
    UPROPERTY(BlueprintReadWrite)
        float PerlinPower;
    UPROPERTY(BlueprintReadWrite)
        float IslandUVRadius;
    UPROPERTY(BlueprintReadWrite)
        float IslandUVCoastalRadius;
    UPROPERTY(BlueprintReadWrite)
        float IslandConditionChangeFreq;
    UPROPERTY(BlueprintReadWrite)
        float IslandConditionChangePower;
    UPROPERTY(BlueprintReadWrite)
        float ElevationConditionHeightContributionRatio;
    UPROPERTY(BlueprintReadWrite)
        float ElevationConditionFreqContributionRatio;
    UPROPERTY(BlueprintReadWrite)
        int32 Seed;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FTerrainGenCompletedDelegate, const TArray<FVector>&, StartingPositions, FIslandMaterialDataCollection, PopulatedIslandMaterialDatacollection);

/**
 * 
 */
UCLASS(BlueprintType)
class PROTOGARDENBATTLE_API UTerrainGenerator : public UObject
{
    GENERATED_BODY()

public:
    UTerrainGenerator();
	~UTerrainGenerator();

    UFUNCTION(BlueprintCallable)
        void GenerateNewIslandMesh(FTerrainGenCompletedDelegate Out, UDynamicMesh* IslandMesh, const FTransform IslandTransform, const int IslandTileSize, const FTerrainGenerationNoiseSettings BaseTerrainGenerationSettings, const FIslandMaterialDataCollection& InputIslandMaterialDataCollection);
    UFUNCTION(BlueprintCallable)
        FIntVector GetBiomeFromWorldPosition(FVector location);

private:
    UPROPERTY(BlueprintType)
        int MapSize;
    UPROPERTY(BlueprintType)
        FTerrainGenerationNoiseSettings GenerationSettings;
    UPROPERTY(BlueprintType)
        FVector OffsetsPerCondition;
    FVector BBoxMin;
    FVector BBoxMax;
};


// Implementation of 1D, 2D and 3D Perlin noise based on Ken Perlin's improved version https://mrl.nyu.edu/~perlin/noise/
// (See Random3.tps for additional third party software info.)
namespace TerrainGenPerlinHelpers
{
    // random permutation of 256 numbers, repeated 2x
    static int32 Permutation[512] = {
        63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
        78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
        57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
        124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
        104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
        155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235,

        63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
        78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
        57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
        124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
        104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
        155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235
    };

    static int32 PermutationBase[512] = {
        63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
        78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
        57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
        124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
        104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
        155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235,

        63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
        78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
        57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
        124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
        104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
        155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235
    };

    // Gradient functions for 1D, 2D and 3D Perlin noise

    FORCEINLINE float Grad1(int32 Hash, float X)
    {
        // Slicing Perlin's 3D improved noise would give us only scales of -1, 0 and 1; this looks pretty bad so let's use a different sampling
        static const float Grad1Scales[16] = { -8 / 8, -7 / 8., -6 / 8., -5 / 8., -4 / 8., -3 / 8., -2 / 8., -1 / 8., 1 / 8., 2 / 8., 3 / 8., 4 / 8., 5 / 8., 6 / 8., 7 / 8., 8 / 8 };
        return Grad1Scales[Hash & 15] * X;
    }

    // Note: If you change the Grad2 or Grad3 functions, check that you don't change the range of the resulting noise as well; it should be (within floating point error) in the range of (-1, 1)
    FORCEINLINE float Grad2(int32 Hash, float X, float Y)
    {
        // corners and major axes (similar to the z=0 projection of the cube-edge-midpoint sampling from improved Perlin noise)
        switch (Hash & 7)
        {
        case 0: return X;
        case 1: return X + Y;
        case 2: return Y;
        case 3: return -X + Y;
        case 4: return -X;
        case 5: return -X - Y;
        case 6: return -Y;
        case 7: return X - Y;
            // can't happen
        default: return 0;
        }
    }

    FORCEINLINE float Grad3(int32 Hash, float X, float Y, float Z)
    {
        switch (Hash & 15)
        {
            // 12 cube midpoints
        case 0: return X + Z;
        case 1: return X + Y;
        case 2: return Y + Z;
        case 3: return -X + Y;
        case 4: return -X + Z;
        case 5: return -X - Y;
        case 6: return -Y + Z;
        case 7: return X - Y;
        case 8: return X - Z;
        case 9: return Y - Z;
        case 10: return -X - Z;
        case 11: return -Y - Z;
            // 4 vertices of regular tetrahedron
        case 12: return X + Y;
        case 13: return -X + Y;
        case 14: return -Y + Z;
        case 15: return -Y - Z;
            // can't happen
        default: return 0;
        }
    }

    // Curve w/ second derivative vanishing at 0 and 1, from Perlin's improved noise paper
    FORCEINLINE float SmoothCurve(float X)
    {
        return X * X * X * (X * (X * 6.0f - 15.0f) + 10.0f);
    }

    static FRandomStream Stream;

    static float PerlinNoise2D(const FVector2D& Location)
    {
        float Xfl = FMath::FloorToFloat((float)Location.X);
        float Yfl = FMath::FloorToFloat((float)Location.Y);
        int32 Xi = (int32)(Xfl) & 255;
        int32 Yi = (int32)(Yfl) & 255;
        float X = (float)Location.X - Xfl;
        float Y = (float)Location.Y - Yfl;
        float Xm1 = X - 1.0f;
        float Ym1 = Y - 1.0f;

        const int32* P = Permutation;
        int32 AA = P[Xi] + Yi;
        int32 AB = AA + 1;
        int32 BA = P[Xi + 1] + Yi;
        int32 BB = BA + 1;

        float U = SmoothCurve(X);
        float V = SmoothCurve(Y);

        // Note: Due to the choice of Grad2, this will be in the (-1,1) range with no additional scaling
        return FMath::Lerp(
            FMath::Lerp(Grad2(P[AA], X, Y), Grad2(P[BA], Xm1, Y), U),
            FMath::Lerp(Grad2(P[AB], X, Ym1), Grad2(P[BB], Xm1, Ym1), U),
            V);
    }
};