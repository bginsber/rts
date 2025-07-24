#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "SurvivalBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Alpine      UMETA(DisplayName = "Alpine"),
    Forest      UMETA(DisplayName = "Forest"), 
    River       UMETA(DisplayName = "River"),
    Transition  UMETA(DisplayName = "Transition")
};

USTRUCT(BlueprintType)
struct FBiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CalorieBurnMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeedMultiplier;

    FBiomeZone()
    {
        BiomeType = EBiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        CalorieBurnMultiplier = 1.0f;
        MovementSpeedMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class RTS_API ASurvivalBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalBiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    class AActor* RaceLandscape;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeZone> BiomeZones;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Route")
    TArray<FVector> RaceCheckpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float LandscapeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 HeightmapResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxElevation;

public:
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void GenerateRaceLandscape();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes") 
    FBiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Race Route")
    void CreateRaceSpline();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(Category = "Terrain")
    void ApplyHeightmapFromArray(const TArray<uint16>& HeightData);

private:
    void InitializeBiomeZones();
    void CreateAlpineBiome();
    void CreateForestBiome(); 
    void CreateRiverBiome();
    void CreateTransitionZones();
    
    TArray<uint16> GenerateHeightmapData();
    float GetBiomeInfluenceAtLocation(const FVector& Location, const FBiomeZone& Biome) const;
};