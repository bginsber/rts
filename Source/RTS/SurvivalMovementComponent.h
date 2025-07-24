#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "SurvivalMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Mud         UMETA(DisplayName = "Mud"), 
    Water       UMETA(DisplayName = "Water"),
    Rock        UMETA(DisplayName = "Rock"),
    Unknown     UMETA(DisplayName = "Unknown")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainChanged, ETerrainType, NewTerrainType);

UCLASS()
class RTS_API USurvivalMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    USurvivalMovementComponent();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Movement")
    float TerrainResistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Movement")
    float StaminaDrainMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Movement")
    bool bApplyTerrainEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Audio")
    class USoundBase* GrassFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Audio")
    class USoundBase* MudFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Audio")
    class USoundBase* WaterFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Audio")
    class USoundBase* RockFootstepSound;

public:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintAssignable, Category = "Terrain Events")
    FOnTerrainChanged OnTerrainChanged;

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void SetTerrainResistance(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    float GetTerrainResistance() const { return TerrainResistanceMultiplier; }

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void ApplyStaminaModifiedSpeed(float StaminaPercentage);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    ETerrainType GetCurrentTerrainType() const { return CurrentTerrainType; }

protected:
    virtual float GetMaxSpeed() const override;
    void DetectTerrainType();
    void PlayFootstepSound();

private:
    float BaseMaxSpeed;
    ETerrainType CurrentTerrainType;
    ETerrainType LastTerrainType;
    float FootstepTimer;
    float FootstepInterval;
};