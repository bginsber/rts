#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalRaceGameMode.generated.h"

class ASurvivalRaceGameState;
class ASurvivalPlayerState;
class ASurvivalCharacter;

UCLASS()
class RTS_API ASurvivalRaceGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASurvivalRaceGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
    float RaceTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
    float MaxTetherDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
    int32 MinPlayersToStart;

    UPROPERTY(BlueprintReadOnly, Category = "Race State")
    ASurvivalRaceGameState* SurvivalGameState;

public:
    UFUNCTION(BlueprintCallable, Category = "Race Management")
    void StartRaceCountdown();

    UFUNCTION(BlueprintCallable, Category = "Race Management")
    void CheckRaceConditions();

    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void AssignPlayerToTeam(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void ValidateTeamIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Team Management")
    void HandlePlayerDisconnection(ASurvivalPlayerState* DisconnectedPlayer);

    UFUNCTION(BlueprintCallable, Category = "Anti-Cheat")
    bool ValidatePlayerMovement(ASurvivalCharacter* Character, FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Anti-Cheat")
    bool ValidateStaminaConsumption(ASurvivalPlayerState* PlayerState, float ConsumedAmount, float DeltaTime);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
    void OnRaceStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
    void OnTeamEliminated(int32 TeamID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
    void OnRaceCompleted(int32 WinningTeamID);

private:
    void CheckTetherConstraints();
    void CheckEliminationConditions();
    void UpdateTeamCohesionScores();
    
    float LastTetherCheck;
    float TetherCheckInterval;
};