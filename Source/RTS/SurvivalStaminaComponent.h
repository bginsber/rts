#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, StaminaPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalorieDeficit, float, DeficitAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaCritical);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTS_API USurvivalStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalStaminaComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float CurrentCalories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float MaxCalories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float BaseMetabolicRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float WalkBurnMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float JogBurnMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float SprintBurnMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float CriticalStaminaThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float LowStaminaThreshold;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
    FOnStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
    FOnCalorieDeficit OnCalorieDeficit;

    UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
    FOnStaminaCritical OnStaminaCritical;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetCurrentCalories() const { return CurrentCalories; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetCaloriePercentage() const { return CurrentCalories / MaxCalories; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeCalories(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void AddCalories(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsStaminaCritical() const { return GetCaloriePercentage() <= CriticalStaminaThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsStaminaLow() const { return GetCaloriePercentage() <= LowStaminaThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetCalorieDeficit() const { return FMath::Max(0.0f, -CurrentCalories); }

    void UpdateCalorieBurn(float DeltaTime, int32 MovementMode, float TerrainMultiplier = 1.0f, float BiomeMultiplier = 1.0f);

private:
    void CheckStaminaThresholds();
    float LastStaminaPercentage;
    bool bWasCritical;
};