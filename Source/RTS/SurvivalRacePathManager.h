#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "SurvivalBiomeManager.h"
#include "SurvivalRacePathManager.generated.h"

USTRUCT(BlueprintType)
struct FRaceCheckpoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CheckpointName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedLimit; // Maximum recommended speed through this checkpoint

    FRaceCheckpoint()
    {
        Location = FVector::ZeroVector;
        Radius = 100.0f;
        AssociatedBiome = EBiomeType::Forest;
        CheckpointName = TEXT("Checkpoint");
        bIsMandatory = true;
        SpeedLimit = 600.0f;
    }
};

USTRUCT(BlueprintType)
struct FPathSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType SegmentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecommendedSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DifficultyMultiplier;

    FPathSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        PathWidth = 200.0f;
        SegmentBiome = EBiomeType::Forest;
        RecommendedSpeed = 400.0f;
        DifficultyMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class RTS_API ASurvivalRacePathManager : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalRacePathManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    class USplineComponent* RaceSpline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Path")
    TArray<FRaceCheckpoint> RaceCheckpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Path")
    TArray<FPathSegment> PathSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Generation")
    class ASurvivalBiomeManager* BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    float DefaultPathWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    float PathSmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    float TotalRaceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    bool bShowDebugPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    class UStaticMesh* CheckpointMarkerMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Race Path")
    void GenerateRacePath();

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    void CreateCheckpointMarkers();

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    float GetDistanceAlongPath(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    FVector GetPathLocationAtDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    FRaceCheckpoint GetNearestCheckpoint(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    bool IsLocationOnPath(const FVector& WorldLocation, float Tolerance = 200.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    float GetRecommendedSpeedAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Race Path")
    EBiomeType GetPathBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Path Analysis")
    float CalculatePathTotalLength() const;

    UFUNCTION(BlueprintCallable, Category = "Path Analysis")
    TArray<float> GetElevationProfile(int32 SampleCount = 100) const;

private:
    void InitializeRaceCheckpoints();
    void GeneratePathSegments();
    void UpdateSplineFromCheckpoints();
    void CalculateSegmentDifficulties();
    
    FVector InterpolatePathElevation(const FVector& BaseLocation) const;
};