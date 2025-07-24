#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SurvivalRaceGameState.generated.h"

class ASurvivalPlayerState;

USTRUCT(BlueprintType)
struct FTeamInfo
{
    GENERATED_BODY()

    UPROPERTY(Replicated, BlueprintReadOnly)
    TArray<ASurvivalPlayerState*> TeamMembers;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float TeamCohesionScore;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float TotalDistanceTraveled;

    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsEliminated;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 TeamID;

    FTeamInfo()
    {
        TeamCohesionScore = 1.0f;
        TotalDistanceTraveled = 0.0f;
        bIsEliminated = false;
        TeamID = -1;
    }
};

UCLASS()
class RTS_API ASurvivalRaceGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ASurvivalRaceGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Teams")
    TArray<FTeamInfo> ActiveTeams;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    float RaceStartTime;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    bool bRaceInProgress;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
    int32 TeamsRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
    int32 MaxTeams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
    int32 PlayersPerTeam;

public:
    UFUNCTION(BlueprintCallable, Category = "Teams")
    int32 AssignPlayerToTeam(ASurvivalPlayerState* PlayerState);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    bool IsTeamFull(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    FTeamInfo GetTeamInfo(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, Category = "Race")
    void StartRace();

    UFUNCTION(BlueprintCallable, Category = "Race")
    void EliminateTeam(int32 TeamID);

    UFUNCTION(BlueprintCallable, Category = "Race")
    bool CanStartRace() const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    TArray<ASurvivalPlayerState*> GetTeamMembers(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UpdateTeamCohesion(int32 TeamID, float CohesionScore);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    void UpdateTeamDistance(int32 TeamID, float DistanceTraveled);

private:
    int32 FindAvailableTeam() const;
    void CreateNewTeam(int32 TeamID);
};