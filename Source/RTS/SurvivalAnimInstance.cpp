#include "SurvivalAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SurvivalStaminaComponent.h"
#include "SurvivalTetherComponent.h"
#include "Kismet/KismetMathLibrary.h"

USurvivalAnimInstance::USurvivalAnimInstance()
{
    Speed = 0.0f;
    bIsInAir = false;
    MovementMode = ESurvivalMovementMode::Walk;
    Direction = 0.0f;
    StaminaPercentage = 1.0f;
    TetherTension = 0.0f;
    SurvivalCharacter = nullptr;
}

void USurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    SurvivalCharacter = Cast<ASurvivalCharacter>(TryGetPawnOwner());
}

void USurvivalAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!SurvivalCharacter)
    {
        SurvivalCharacter = Cast<ASurvivalCharacter>(TryGetPawnOwner());
        return;
    }

    // Update basic movement properties
    if (UCharacterMovementComponent* Movement = SurvivalCharacter->GetCharacterMovement())
    {
        FVector Velocity = Movement->Velocity;
        Speed = Velocity.Size();
        bIsInAir = Movement->IsFalling();
        
        // Calculate movement direction relative to character rotation
        if (Speed > 0.0f)
        {
            FVector ForwardVector = SurvivalCharacter->GetActorForwardVector();
            Direction = UKismetMathLibrary::DegAcos(FVector::DotProduct(ForwardVector, Velocity.GetSafeNormal()));
            
            // Determine if moving backwards
            FVector RightVector = SurvivalCharacter->GetActorRightVector();
            if (FVector::DotProduct(RightVector, Velocity) < 0)
            {
                Direction *= -1.0f;
            }
        }
        else
        {
            Direction = 0.0f;
        }
    }

    // Update survival-specific properties
    MovementMode = SurvivalCharacter->GetCurrentMovementMode();
    
    if (USurvivalStaminaComponent* StaminaComp = SurvivalCharacter->GetStaminaComponent())
    {
        StaminaPercentage = StaminaComp->GetCaloriePercentage();
    }
    
    if (USurvivalTetherComponent* TetherComp = SurvivalCharacter->GetTetherComponent())
    {
        TetherTension = TetherComp->GetTetherTension();
    }
}