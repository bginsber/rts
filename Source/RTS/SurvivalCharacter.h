#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "SurvivalCharacter.generated.h"

UENUM(BlueprintType)
enum class EMovementMode : uint8
{
    Walk    UMETA(DisplayName = "Walk"),
    Jog     UMETA(DisplayName = "Jog"),
    Sprint  UMETA(DisplayName = "Sprint")
};

class USurvivalStaminaComponent;
class USurvivalTetherComponent;
class USurvivalMovementComponent;

UCLASS()
class RTS_API ASurvivalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASurvivalCharacter();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    USurvivalStaminaComponent* StaminaComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    USurvivalTetherComponent* TetherComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    USurvivalMovementComponent* SurvivalMovementComponent;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EMovementMode CurrentMovementMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JogSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float InputBufferTime;

private:
    float TargetSpeed;
    float CurrentTransitionTime;
    
    TArray<EMovementMode> InputBuffer;
    float LastInputTime;

public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementMode(EMovementMode NewMode);

    // Server RPCs for networked movement
    UFUNCTION(Server, Reliable, WithValidation, Category = "Movement")
    void ServerSetMovementMode(EMovementMode NewMode);

    UFUNCTION(NetMulticast, Reliable, Category = "Movement")
    void MulticastSetMovementMode(EMovementMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    EMovementMode GetCurrentMovementMode() const { return CurrentMovementMode; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    USurvivalStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    USurvivalTetherComponent* GetTetherComponent() const { return TetherComponent; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    USurvivalMovementComponent* GetSurvivalMovementComponent() const { return SurvivalMovementComponent; }

protected:
    void SwitchToWalk();
    void SwitchToJog();
    void SwitchToSprint();

    void UpdateMovementSpeed();
    void ProcessInputBuffer();
    void AddToInputBuffer(EMovementMode Mode);
};