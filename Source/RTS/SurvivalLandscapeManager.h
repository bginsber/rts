#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "SurvivalBiomeManager.h"
#include "SurvivalLandscapeManager.generated.h"

USTRUCT(BlueprintType)
struct FHeightmapData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<uint16> HeightValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxElevation;

    FHeightmapData()
    {
        Width = 1009;
        Height = 1009;
        MaxElevation = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class RTS_API ASurvivalLandscapeManager : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalLandscapeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
    class ASurvivalBiomeManager* BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Generation")
    class ALandscape* TargetLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    int32 HeightmapResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    float RaceRouteWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    float RaceRouteLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
    float MaxElevationVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Generation")
    TArray<FVector> RouteCheckpoints;

public:
    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    FHeightmapData GenerateHeightmapData();

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    void ApplyHeightmapToLandscape(const FHeightmapData& HeightmapData);

    UFUNCTION(BlueprintCallable, Category = "Route Generation")
    void GenerateRaceRoute();

    UFUNCTION(BlueprintCallable, Category = "Elevation")
    float GetElevationAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Elevation")
    float SampleHeightmapAtUV(const FHeightmapData& HeightmapData, float U, float V) const;

private:
    void InitializeRouteCheckpoints();
    float CalculateBiomeElevation(const FVector& WorldLocation, const FBiomeZone& Biome) const;
    float ApplyPerlinNoise(float X, float Y, float Scale, float Amplitude) const;
    uint16 WorldHeightToHeightmapValue(float WorldHeight) const;
};