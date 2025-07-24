#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalBiomeManager.h"
#include "SurvivalBiomeChallenge.generated.h"

UENUM(BlueprintType)
enum class EChallengeType : uint8
{
    None            UMETA(DisplayName = "None"),
    VerticalClimb   UMETA(DisplayName = "Vertical Climb"),
    Exposure        UMETA(DisplayName = "Altitude Exposure"),
    Navigation      UMETA(DisplayName = "Dense Vegetation"),
    ResourceGather  UMETA(DisplayName = "Resource Gathering"),
    WaterCrossing   UMETA(DisplayName = "Water Crossing"),
    Hypothermia     UMETA(DisplayName = "Hypothermia Risk"),
    TerrainInstability UMETA(DisplayName = "Unstable Terrain")
};

USTRUCT(BlueprintType)
struct FBiomeChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EChallengeType ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ChallengeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresTeamCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeToComplete;

    FBiomeChallenge()
    {
        ChallengeType = EChallengeType::None;
        IntensityMultiplier = 1.0f;
        ActivationRadius = 500.0f;
        ChallengeLocation = FVector::ZeroVector;
        bRequiresTeamCoordination = false;
        TimeToComplete = 30.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class RTS_API ASurvivalBiomeChallenge : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalBiomeChallenge();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge System")
    class ASurvivalBiomeManager* BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alpine Challenges")
    TArray<FBiomeChallenge> AlpineChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Challenges")
    TArray<FBiomeChallenge> ForestChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Challenges")
    TArray<FBiomeChallenge> RiverChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge Detection")
    float ChallengeCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float ExposureIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float NavigationDifficulty;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Challenge System")
    void InitializeBiomeChallenges();

    UFUNCTION(BlueprintCallable, Category = "Challenge System")
    TArray<FBiomeChallenge> GetActiveChallengesAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Challenge System")
    bool IsLocationWithinChallenge(const FVector& WorldLocation, const FBiomeChallenge& Challenge) const;

    UFUNCTION(BlueprintCallable, Category = "Alpine Challenges")
    float CalculateAltitudeExposure(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Alpine Challenges")
    bool RequiresVerticalClimbing(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Forest Challenges") 
    float CalculateNavigationDifficulty(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "River Challenges")
    float CalculateHypothermiaRisk(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Challenge System")
    void ApplyChallengeEffects(class ASurvivalCharacter* Character, const FBiomeChallenge& Challenge);

private:
    void CreateAlpineChallenges();
    void CreateForestChallenges();
    void CreateRiverChallenges();
    
    float ChallengeCheckTimer;
    TArray<class ASurvivalCharacter*> TrackedCharacters;
};