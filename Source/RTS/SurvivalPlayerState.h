#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SurvivalPlayerState.generated.h"

UENUM(BlueprintType)
enum class ESurvivalSpecialization : uint8
{
    None        UMETA(DisplayName = "None"),
    Climber     UMETA(DisplayName = "Climber"),
    Navigator   UMETA(DisplayName = "Navigator"),
    Forager     UMETA(DisplayName = "Forager"),
    Medic       UMETA(DisplayName = "Medic")
};

UCLASS()
class RTS_API ASurvivalPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASurvivalPlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // Survival Metrics
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Survival")
    float CurrentCalories;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Survival")
    float MaxCalories;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Survival")
    float DistanceTraveled;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Survival")
    float TotalCaloriesBurned;

    // Team Information
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    int32 TeamID;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    ASurvivalPlayerState* TeamPartner;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    float TetherDistance;

    // Player Specialization
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Specialization")
    ESurvivalSpecialization PlayerSpecialization;

    // Race Status
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    bool bIsEliminated;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    bool bIsIncapacitated;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    float RaceStartTime;

public:
    // Calorie Management
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetCaloriePercentage() const { return MaxCalories > 0 ? CurrentCalories / MaxCalories : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateCalories(float NewCalories);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeCalories(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddCalories(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStaminaCritical() const { return GetCaloriePercentage() <= 0.15f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStaminaLow() const { return GetCaloriePercentage() <= 0.30f; }

    // Distance Tracking
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateDistanceTraveled(float NewDistance);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetDistanceTraveled() const { return DistanceTraveled; }

    // Team Management
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamID(int32 NewTeamID) { TeamID = NewTeamID; }

    UFUNCTION(BlueprintCallable, Category = "Team")
    int32 GetTeamID() const { return TeamID; }

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamPartner(ASurvivalPlayerState* Partner);

    UFUNCTION(BlueprintCallable, Category = "Team")
    ASurvivalPlayerState* GetTeamPartner() const { return TeamPartner; }

    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTetherDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Team")
    bool IsWithinTetherRange() const { return TetherDistance <= 50.0f; }

    // Specialization
    UFUNCTION(BlueprintCallable, Category = "Specialization")
    void SetSpecialization(ESurvivalSpecialization Specialization);

    UFUNCTION(BlueprintCallable, Category = "Specialization")
    ESurvivalSpecialization GetSpecialization() const { return PlayerSpecialization; }

    // Race Status
    UFUNCTION(BlueprintCallable, Category = "Race")
    void EliminatePlayer();

    UFUNCTION(BlueprintCallable, Category = "Race")
    void SetIncapacitated(bool bIncapacitated);

    UFUNCTION(BlueprintCallable, Category = "Race")
    bool IsEliminated() const { return bIsEliminated; }

    UFUNCTION(BlueprintCallable, Category = "Race")
    bool IsIncapacitated() const { return bIsIncapacitated; }

    // Server RPCs for validation
    UFUNCTION(Server, Reliable, WithValidation, Category = "Network")
    void ServerUpdateCalories(float NewCalories);

    UFUNCTION(Server, Reliable, WithValidation, Category = "Network")
    void ServerUpdateDistance(float NewDistance);

private:
    // Validation helpers
    bool IsValidCalorieUpdate(float NewCalories) const;
    bool IsValidDistanceUpdate(float NewDistance) const;
};