#include "SurvivalStaminaComponent.h"

USurvivalStaminaComponent::USurvivalStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize based on design document specifications
    MaxCalories = 2000.0f;
    CurrentCalories = MaxCalories;
    BaseMetabolicRate = 2000.0f; // 2000 cal/day baseline

    // Movement burn multipliers from design document
    WalkBurnMultiplier = 2.5f;
    JogBurnMultiplier = 5.0f;
    SprintBurnMultiplier = 9.0f;

    // Visual feedback thresholds
    CriticalStaminaThreshold = 0.15f; // 15% - critical state
    LowStaminaThreshold = 0.30f; // 30% - low stamina warning

    // Initialize tracking variables
    LastStaminaPercentage = 1.0f;
    bWasCritical = false;
}

void USurvivalStaminaComponent::BeginPlay()
{
    Super::BeginPlay();
    LastStaminaPercentage = GetCaloriePercentage();
}

void USurvivalStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Apply base metabolic rate consumption
    float BaseConsumption = (BaseMetabolicRate / 86400.0f) * DeltaTime; // Convert per day to per second
    ConsumeCalories(BaseConsumption);

    // Check for threshold changes and broadcast events
    CheckStaminaThresholds();
}

void USurvivalStaminaComponent::ConsumeCalories(float Amount)
{
    CurrentCalories = FMath::Max(0.0f, CurrentCalories - Amount);
    CheckStaminaThresholds();
}

void USurvivalStaminaComponent::AddCalories(float Amount)
{
    CurrentCalories = FMath::Min(MaxCalories, CurrentCalories + Amount);
    CheckStaminaThresholds();
}

void USurvivalStaminaComponent::UpdateCalorieBurn(float DeltaTime, int32 MovementMode, float TerrainMultiplier, float BiomeMultiplier)
{
    float BurnMultiplier = 1.0f;
    
    switch (MovementMode)
    {
    case 0: // Walk
        BurnMultiplier = WalkBurnMultiplier;
        break;
    case 1: // Jog
        BurnMultiplier = JogBurnMultiplier;
        break;
    case 2: // Sprint
        BurnMultiplier = SprintBurnMultiplier;
        break;
    }

    float MovementBurn = (BaseMetabolicRate / 86400.0f) * BurnMultiplier * TerrainMultiplier * BiomeMultiplier * DeltaTime;
    ConsumeCalories(MovementBurn);
}

void USurvivalStaminaComponent::CheckStaminaThresholds()
{
    float CurrentStaminaPercentage = GetCaloriePercentage();
    
    // Broadcast stamina changes for UI updates
    if (FMath::Abs(CurrentStaminaPercentage - LastStaminaPercentage) > 0.001f)
    {
        OnStaminaChanged.Broadcast(CurrentStaminaPercentage);
        LastStaminaPercentage = CurrentStaminaPercentage;
    }

    // Check for critical stamina state changes
    bool bIsCriticalNow = IsStaminaCritical();
    if (bIsCriticalNow && !bWasCritical)
    {
        OnStaminaCritical.Broadcast();
        bWasCritical = true;
    }
    else if (!bIsCriticalNow && bWasCritical)
    {
        bWasCritical = false;
    }

    // Broadcast calorie deficit for negative calorie states
    float Deficit = GetCalorieDeficit();
    if (Deficit > 0.0f)
    {
        OnCalorieDeficit.Broadcast(Deficit);
    }
}