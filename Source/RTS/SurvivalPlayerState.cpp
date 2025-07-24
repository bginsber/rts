#include "SurvivalPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ASurvivalPlayerState::ASurvivalPlayerState()
{
    bReplicates = true;
    
    // Initialize survival metrics based on design document
    MaxCalories = 2000.0f;
    CurrentCalories = MaxCalories;
    DistanceTraveled = 0.0f;
    TotalCaloriesBurned = 0.0f;
    
    // Initialize team information
    TeamID = -1;
    TeamPartner = nullptr;
    TetherDistance = 0.0f;
    
    // Initialize specialization
    PlayerSpecialization = ESurvivalSpecialization::None;
    
    // Initialize race status
    bIsEliminated = false;
    bIsIncapacitated = false;
    RaceStartTime = 0.0f;
}

void ASurvivalPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Survival metrics
    DOREPLIFETIME(ASurvivalPlayerState, CurrentCalories);
    DOREPLIFETIME(ASurvivalPlayerState, MaxCalories);
    DOREPLIFETIME(ASurvivalPlayerState, DistanceTraveled);
    DOREPLIFETIME(ASurvivalPlayerState, TotalCaloriesBurned);
    
    // Team information
    DOREPLIFETIME(ASurvivalPlayerState, TeamID);
    DOREPLIFETIME(ASurvivalPlayerState, TeamPartner);
    DOREPLIFETIME(ASurvivalPlayerState, TetherDistance);
    
    // Specialization
    DOREPLIFETIME(ASurvivalPlayerState, PlayerSpecialization);
    
    // Race status
    DOREPLIFETIME(ASurvivalPlayerState, bIsEliminated);
    DOREPLIFETIME(ASurvivalPlayerState, bIsIncapacitated);
    DOREPLIFETIME(ASurvivalPlayerState, RaceStartTime);
}

void ASurvivalPlayerState::UpdateCalories(float NewCalories)
{
    if (HasAuthority())
    {
        if (IsValidCalorieUpdate(NewCalories))
        {
            CurrentCalories = FMath::Clamp(NewCalories, -10000.0f, MaxCalories); // Allow negative calories for deficit
        }
    }
    else
    {
        // Client requests server validation
        ServerUpdateCalories(NewCalories);
    }
}

void ASurvivalPlayerState::ConsumeCalories(float Amount)
{
    if (HasAuthority())
    {
        float NewCalories = CurrentCalories - Amount;
        if (IsValidCalorieUpdate(NewCalories))
        {
            CurrentCalories = NewCalories;
            TotalCaloriesBurned += Amount;
        }
    }
}

void ASurvivalPlayerState::AddCalories(float Amount)
{
    if (HasAuthority())
    {
        float NewCalories = FMath::Min(CurrentCalories + Amount, MaxCalories);
        if (IsValidCalorieUpdate(NewCalories))
        {
            CurrentCalories = NewCalories;
        }
    }
}

void ASurvivalPlayerState::UpdateDistanceTraveled(float NewDistance)
{
    if (HasAuthority())
    {
        if (IsValidDistanceUpdate(NewDistance))
        {
            DistanceTraveled = NewDistance;
        }
    }
    else
    {
        // Client requests server validation
        ServerUpdateDistance(NewDistance);
    }
}

void ASurvivalPlayerState::SetTeamPartner(ASurvivalPlayerState* Partner)
{
    if (HasAuthority())
    {
        TeamPartner = Partner;
        if (Partner && Partner->TeamPartner != this)
        {
            Partner->TeamPartner = this;
        }
    }
}

void ASurvivalPlayerState::UpdateTetherDistance(float Distance)
{
    if (HasAuthority())
    {
        TetherDistance = Distance;
    }
}

void ASurvivalPlayerState::SetSpecialization(ESurvivalSpecialization Specialization)
{
    if (HasAuthority())
    {
        PlayerSpecialization = Specialization;
    }
}

void ASurvivalPlayerState::EliminatePlayer()
{
    if (HasAuthority())
    {
        bIsEliminated = true;
        UE_LOG(LogTemp, Log, TEXT("Player %s eliminated"), *GetPlayerName());
    }
}

void ASurvivalPlayerState::SetIncapacitated(bool bIncapacitated)
{
    if (HasAuthority())
    {
        bIsIncapacitated = bIncapacitated;
    }
}

void ASurvivalPlayerState::ServerUpdateCalories_Implementation(float NewCalories)
{
    UpdateCalories(NewCalories);
}

bool ASurvivalPlayerState::ServerUpdateCalories_Validate(float NewCalories)
{
    return IsValidCalorieUpdate(NewCalories);
}

void ASurvivalPlayerState::ServerUpdateDistance_Implementation(float NewDistance)
{
    UpdateDistanceTraveled(NewDistance);
}

bool ASurvivalPlayerState::ServerUpdateDistance_Validate(float NewDistance)
{
    return IsValidDistanceUpdate(NewDistance);
}

bool ASurvivalPlayerState::IsValidCalorieUpdate(float NewCalories) const
{
    // Allow reasonable calorie ranges (negative for deficit, up to max for positive)
    return NewCalories >= -10000.0f && NewCalories <= MaxCalories;
}

bool ASurvivalPlayerState::IsValidDistanceUpdate(float NewDistance) const
{
    // Distance should only increase and be within reasonable bounds
    return NewDistance >= DistanceTraveled && NewDistance <= DistanceTraveled + 1000.0f; // Max 1km jump per update
}