#include "SurvivalRaceGameState.h"
#include "SurvivalPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ASurvivalRaceGameState::ASurvivalRaceGameState()
{
    bReplicates = true;
    
    // Initialize race configuration
    MaxTeams = 4;
    PlayersPerTeam = 2;
    TeamsRemaining = 0;
    bRaceInProgress = false;
    RaceStartTime = 0.0f;
    
    // Initialize empty teams array
    ActiveTeams.SetNum(MaxTeams);
    for (int32 i = 0; i < MaxTeams; i++)
    {
        ActiveTeams[i].TeamID = i;
    }
}

void ASurvivalRaceGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASurvivalRaceGameState, ActiveTeams);
    DOREPLIFETIME(ASurvivalRaceGameState, RaceStartTime);
    DOREPLIFETIME(ASurvivalRaceGameState, bRaceInProgress);
    DOREPLIFETIME(ASurvivalRaceGameState, TeamsRemaining);
}

int32 ASurvivalRaceGameState::AssignPlayerToTeam(ASurvivalPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return -1;
    }

    // Find available team
    int32 TeamID = FindAvailableTeam();
    if (TeamID == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available teams for player assignment"));
        return -1;
    }

    // Add player to team
    ActiveTeams[TeamID].TeamMembers.Add(PlayerState);
    
    // Update teams remaining count
    if (ActiveTeams[TeamID].TeamMembers.Num() == PlayersPerTeam)
    {
        TeamsRemaining++;
    }

    UE_LOG(LogTemp, Log, TEXT("Player assigned to Team %d. Team now has %d/%d players"), 
           TeamID, ActiveTeams[TeamID].TeamMembers.Num(), PlayersPerTeam);

    return TeamID;
}

bool ASurvivalRaceGameState::IsTeamFull(int32 TeamID) const
{
    if (TeamID < 0 || TeamID >= ActiveTeams.Num())
    {
        return true; // Invalid team is considered "full"
    }

    return ActiveTeams[TeamID].TeamMembers.Num() >= PlayersPerTeam;
}

FTeamInfo ASurvivalRaceGameState::GetTeamInfo(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < ActiveTeams.Num())
    {
        return ActiveTeams[TeamID];
    }
    
    return FTeamInfo(); // Return default/empty team info
}

void ASurvivalRaceGameState::StartRace()
{
    if (CanStartRace() && HasAuthority())
    {
        bRaceInProgress = true;
        RaceStartTime = GetServerWorldTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Race started with %d teams competing"), TeamsRemaining);
    }
}

void ASurvivalRaceGameState::EliminateTeam(int32 TeamID)
{
    if (TeamID >= 0 && TeamID < ActiveTeams.Num() && HasAuthority())
    {
        if (!ActiveTeams[TeamID].bIsEliminated)
        {
            ActiveTeams[TeamID].bIsEliminated = true;
            TeamsRemaining--;
            
            UE_LOG(LogTemp, Log, TEXT("Team %d eliminated. %d teams remaining"), TeamID, TeamsRemaining);
            
            // Check if race should end
            if (TeamsRemaining <= 1)
            {
                bRaceInProgress = false;
                UE_LOG(LogTemp, Log, TEXT("Race ended - winner determined"));
            }
        }
    }
}

bool ASurvivalRaceGameState::CanStartRace() const
{
    return TeamsRemaining >= 2 && !bRaceInProgress;
}

TArray<ASurvivalPlayerState*> ASurvivalRaceGameState::GetTeamMembers(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < ActiveTeams.Num())
    {
        return ActiveTeams[TeamID].TeamMembers;
    }
    
    return TArray<ASurvivalPlayerState*>();
}

void ASurvivalRaceGameState::UpdateTeamCohesion(int32 TeamID, float CohesionScore)
{
    if (TeamID >= 0 && TeamID < ActiveTeams.Num() && HasAuthority())
    {
        ActiveTeams[TeamID].TeamCohesionScore = FMath::Clamp(CohesionScore, 0.0f, 1.0f);
    }
}

void ASurvivalRaceGameState::UpdateTeamDistance(int32 TeamID, float DistanceTraveled)
{
    if (TeamID >= 0 && TeamID < ActiveTeams.Num() && HasAuthority())
    {
        ActiveTeams[TeamID].TotalDistanceTraveled = FMath::Max(ActiveTeams[TeamID].TotalDistanceTraveled, DistanceTraveled);
    }
}

int32 ASurvivalRaceGameState::FindAvailableTeam() const
{
    for (int32 i = 0; i < ActiveTeams.Num(); i++)
    {
        if (!IsTeamFull(i) && !ActiveTeams[i].bIsEliminated)
        {
            return i;
        }
    }
    
    return -1; // No available team
}