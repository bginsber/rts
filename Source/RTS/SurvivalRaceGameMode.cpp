#include "SurvivalRaceGameMode.h"
#include "SurvivalRaceGameState.h"
#include "SurvivalPlayerState.h"
#include "SurvivalCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASurvivalRaceGameMode::ASurvivalRaceGameMode()
{
    // Set default classes
    GameStateClass = ASurvivalRaceGameState::StaticClass();
    PlayerStateClass = ASurvivalPlayerState::StaticClass();
    DefaultPawnClass = ASurvivalCharacter::StaticClass();

    // Initialize race configuration
    RaceTimeLimit = 10800.0f; // 3 hours in seconds
    MaxTetherDistance = 50.0f; // 50 meter tether constraint
    MinPlayersToStart = 4; // Minimum 2 teams of 2 players

    // Initialize timing
    LastTetherCheck = 0.0f;
    TetherCheckInterval = 1.0f; // Check tether constraints every second

    PrimaryActorTick.bCanEverTick = true;
}

void ASurvivalRaceGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        UE_LOG(LogTemp, Log, TEXT("Player logged in: %s"), *NewPlayer->GetName());
        
        // Assign player to team
        AssignPlayerToTeam(NewPlayer);
        
        // Check if we can start the race
        CheckRaceConditions();
    }
}

void ASurvivalRaceGameMode::Logout(AController* Exiting)
{
    if (APlayerController* PlayerController = Cast<APlayerController>(Exiting))
    {
        if (ASurvivalPlayerState* PlayerState = PlayerController->GetPlayerState<ASurvivalPlayerState>())
        {
            HandlePlayerDisconnection(PlayerState);
        }
    }

    Super::Logout(Exiting);
}

void ASurvivalRaceGameMode::BeginPlay()
{
    Super::BeginPlay();

    SurvivalGameState = GetGameState<ASurvivalRaceGameState>();
    if (!SurvivalGameState)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalRaceGameMode: Failed to get SurvivalRaceGameState"));
    }
}

void ASurvivalRaceGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (SurvivalGameState && SurvivalGameState->bRaceInProgress)
    {
        // Periodic checks during race
        LastTetherCheck += DeltaTime;
        if (LastTetherCheck >= TetherCheckInterval)
        {
            CheckTetherConstraints();
            CheckEliminationConditions();
            UpdateTeamCohesionScores();
            LastTetherCheck = 0.0f;
        }
    }
}

void ASurvivalRaceGameMode::StartRaceCountdown()
{
    if (SurvivalGameState && SurvivalGameState->CanStartRace())
    {
        UE_LOG(LogTemp, Log, TEXT("Starting race countdown"));
        
        // Start the race through the game state
        SurvivalGameState->StartRace();
        
        // Trigger Blueprint event
        OnRaceStarted();
    }
}

void ASurvivalRaceGameMode::CheckRaceConditions()
{
    if (!SurvivalGameState)
        return;

    // Count active players
    int32 ActivePlayers = GetNumPlayers();
    
    if (ActivePlayers >= MinPlayersToStart && !SurvivalGameState->bRaceInProgress)
    {
        UE_LOG(LogTemp, Log, TEXT("Race conditions met. %d players connected"), ActivePlayers);
        StartRaceCountdown();
    }
}

void ASurvivalRaceGameMode::AssignPlayerToTeam(APlayerController* PlayerController)
{
    if (!PlayerController || !SurvivalGameState)
        return;

    ASurvivalPlayerState* PlayerState = PlayerController->GetPlayerState<ASurvivalPlayerState>();
    if (!PlayerState)
        return;

    int32 AssignedTeamID = SurvivalGameState->AssignPlayerToTeam(PlayerState);
    if (AssignedTeamID != -1)
    {
        PlayerState->SetTeamID(AssignedTeamID);
        
        // Set up team partnership if team is now full
        TArray<ASurvivalPlayerState*> TeamMembers = SurvivalGameState->GetTeamMembers(AssignedTeamID);
        if (TeamMembers.Num() == 2)
        {
            TeamMembers[0]->SetTeamPartner(TeamMembers[1]);
            TeamMembers[1]->SetTeamPartner(TeamMembers[0]);
            
            UE_LOG(LogTemp, Log, TEXT("Team %d is now complete with partners assigned"), AssignedTeamID);
        }
    }
}

void ASurvivalRaceGameMode::ValidateTeamIntegrity()
{
    if (!SurvivalGameState)
        return;

    // Check all teams for disconnected players
    for (int32 TeamID = 0; TeamID < SurvivalGameState->MaxTeams; TeamID++)
    {
        TArray<ASurvivalPlayerState*> TeamMembers = SurvivalGameState->GetTeamMembers(TeamID);
        
        // Remove null/invalid players
        TeamMembers.RemoveAll([](ASurvivalPlayerState* Player) {
            return !IsValid(Player) || !Player->GetPlayerController();
        });
        
        // If team is incomplete, eliminate it
        if (TeamMembers.Num() < SurvivalGameState->PlayersPerTeam && SurvivalGameState->bRaceInProgress)
        {
            SurvivalGameState->EliminateTeam(TeamID);
            OnTeamEliminated(TeamID);
        }
    }
}

void ASurvivalRaceGameMode::HandlePlayerDisconnection(ASurvivalPlayerState* DisconnectedPlayer)
{
    if (!DisconnectedPlayer || !SurvivalGameState)
        return;

    int32 TeamID = DisconnectedPlayer->GetTeamID();
    if (TeamID != -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player disconnected from Team %d"), TeamID);
        
        // Eliminate the entire team due to disconnection
        SurvivalGameState->EliminateTeam(TeamID);
        OnTeamEliminated(TeamID);
    }
}

bool ASurvivalRaceGameMode::ValidatePlayerMovement(ASurvivalCharacter* Character, FVector NewLocation)
{
    if (!Character)
        return false;

    FVector CurrentLocation = Character->GetActorLocation();
    float MovementDistance = FVector::Dist(CurrentLocation, NewLocation);
    
    // Basic sanity check - no teleporting
    const float MaxMovementPerFrame = 1000.0f; // 10 meters per frame at 60fps
    
    return MovementDistance <= MaxMovementPerFrame;
}

bool ASurvivalRaceGameMode::ValidateStaminaConsumption(ASurvivalPlayerState* PlayerState, float ConsumedAmount, float DeltaTime)
{
    if (!PlayerState)
        return false;

    // Calculate maximum possible consumption based on sprint mode
    const float MaxBurnRate = 9.0f; // Sprint multiplier
    const float BaseMetabolicRate = 2000.0f; // Calories per day
    const float MaxConsumptionPerSecond = (BaseMetabolicRate / 86400.0f) * MaxBurnRate;
    const float MaxConsumption = MaxConsumptionPerSecond * DeltaTime * 2.0f; // 2x buffer for lag

    return ConsumedAmount <= MaxConsumption;
}

void ASurvivalRaceGameMode::CheckTetherConstraints()
{
    if (!SurvivalGameState)
        return;

    for (int32 TeamID = 0; TeamID < SurvivalGameState->MaxTeams; TeamID++)
    {
        TArray<ASurvivalPlayerState*> TeamMembers = SurvivalGameState->GetTeamMembers(TeamID);
        
        if (TeamMembers.Num() == 2)
        {
            // Get character locations
            ASurvivalCharacter* Player1 = Cast<ASurvivalCharacter>(TeamMembers[0]->GetPawn());
            ASurvivalCharacter* Player2 = Cast<ASurvivalCharacter>(TeamMembers[1]->GetPawn());
            
            if (Player1 && Player2)
            {
                float Distance = FVector::Dist(Player1->GetActorLocation(), Player2->GetActorLocation());
                
                // Update tether distance in player states
                TeamMembers[0]->UpdateTetherDistance(Distance);
                TeamMembers[1]->UpdateTetherDistance(Distance);
                
                // Check for violations
                if (Distance > MaxTetherDistance * 1.1f) // 10% buffer
                {
                    UE_LOG(LogTemp, Warning, TEXT("Team %d tether constraint violated: %.2f meters"), TeamID, Distance);
                    // Could implement penalties here
                }
            }
        }
    }
}

void ASurvivalRaceGameMode::CheckEliminationConditions()
{
    if (!SurvivalGameState)
        return;

    for (int32 TeamID = 0; TeamID < SurvivalGameState->MaxTeams; TeamID++)
    {
        TArray<ASurvivalPlayerState*> TeamMembers = SurvivalGameState->GetTeamMembers(TeamID);
        FTeamInfo TeamInfo = SurvivalGameState->GetTeamInfo(TeamID);
        
        if (!TeamInfo.bIsEliminated && TeamMembers.Num() == 2)
        {
            bool bShouldEliminate = false;
            
            // Check if both players are critically low on stamina
            bool bBothCritical = TeamMembers[0]->IsStaminaCritical() && TeamMembers[1]->IsStaminaCritical();
            
            // Check if either player is incapacitated
            bool bEitherIncapacitated = TeamMembers[0]->IsIncapacitated() || TeamMembers[1]->IsIncapacitated();
            
            if (bBothCritical || bEitherIncapacitated)
            {
                bShouldEliminate = true;
            }
            
            if (bShouldEliminate)
            {
                SurvivalGameState->EliminateTeam(TeamID);
                OnTeamEliminated(TeamID);
            }
        }
    }
}

void ASurvivalRaceGameMode::UpdateTeamCohesionScores()
{
    if (!SurvivalGameState)
        return;

    for (int32 TeamID = 0; TeamID < SurvivalGameState->MaxTeams; TeamID++)
    {
        TArray<ASurvivalPlayerState*> TeamMembers = SurvivalGameState->GetTeamMembers(TeamID);
        
        if (TeamMembers.Num() == 2)
        {
            // Calculate cohesion based on distance and synchronized actions
            float TetherDistance = TeamMembers[0]->TetherDistance;
            float CohesionScore = 1.0f - (TetherDistance / MaxTetherDistance);
            CohesionScore = FMath::Clamp(CohesionScore, 0.0f, 1.0f);
            
            SurvivalGameState->UpdateTeamCohesion(TeamID, CohesionScore);
        }
    }
}