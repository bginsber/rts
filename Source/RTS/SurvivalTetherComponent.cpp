#include "SurvivalTetherComponent.h"
#include "SurvivalCharacter.h"
#include "Engine/Engine.h"

USurvivalTetherComponent::USurvivalTetherComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Set maximum tether distance from design document (50 meters)
    MaxTetherDistance = 5000.0f; // 50 meters in UE units (cm)
    TetheredPartner = nullptr;
    TetherTension = 0.0f;
}

void USurvivalTetherComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USurvivalTetherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (TetheredPartner)
    {
        UpdateTetherConstraints();
    }
}

void USurvivalTetherComponent::SetTetheredPartner(ASurvivalCharacter* Partner)
{
    TetheredPartner = Partner;
}

float USurvivalTetherComponent::GetDistanceToPartner() const
{
    if (!TetheredPartner || !GetOwner())
    {
        return 0.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), TetheredPartner->GetActorLocation());
}

bool USurvivalTetherComponent::IsWithinTetherRange() const
{
    return GetDistanceToPartner() <= MaxTetherDistance;
}

void USurvivalTetherComponent::UpdateTetherConstraints()
{
    float Distance = GetDistanceToPartner();
    
    if (Distance > MaxTetherDistance)
    {
        // Calculate tension based on overstreach
        TetherTension = (Distance - MaxTetherDistance) / MaxTetherDistance;
        
        // Apply constraint force (this is a simplified version)
        // In a full implementation, this would use physics constraints
        if (ASurvivalCharacter* OwnerCharacter = Cast<ASurvivalCharacter>(GetOwner()))
        {
            FVector Direction = (TetheredPartner->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            FVector TargetLocation = TetheredPartner->GetActorLocation() - (Direction * MaxTetherDistance);
            
            // Gradually pull the character back within range
            FVector CurrentLocation = OwnerCharacter->GetActorLocation();
            FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 2.0f);
            OwnerCharacter->SetActorLocation(NewLocation);
        }
    }
    else
    {
        TetherTension = 0.0f;
    }
}