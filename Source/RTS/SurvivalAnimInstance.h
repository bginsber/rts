#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SurvivalCharacter.h"
#include "SurvivalAnimInstance.generated.h"

UCLASS()
class RTS_API USurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USurvivalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    ESurvivalMovementMode MovementMode;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float TetherTension;

private:
    UPROPERTY()
    ASurvivalCharacter* SurvivalCharacter;
};