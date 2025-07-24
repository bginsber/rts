#include "SurvivalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "SurvivalStaminaComponent.h"
#include "SurvivalTetherComponent.h"
#include "SurvivalMovementComponent.h"
#include "SurvivalPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ASurvivalCharacter::ASurvivalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Initialize movement speeds based on design document
    WalkSpeed = 180.0f;  // 1.8 speed multiplier from base
    JogSpeed = 320.0f;   // 3.2 speed multiplier from base  
    SprintSpeed = 500.0f; // 5.0 speed multiplier from base

    CurrentMovementMode = EMovementMode::Walk;
    
    // Initialize smooth transition parameters
    TransitionSpeed = 3.0f; // Speed of transitions between modes
    InputBufferTime = 0.2f; // 200ms input buffer window
    TargetSpeed = WalkSpeed;
    CurrentTransitionTime = 0.0f;
    LastInputTime = 0.0f;

    // Create survival components
    StaminaComponent = CreateDefaultSubobject<USurvivalStaminaComponent>(TEXT("StaminaComponent"));
    TetherComponent = CreateDefaultSubobject<USurvivalTetherComponent>(TEXT("TetherComponent"));
    
    // Replace default movement component with custom survival movement component
    SurvivalMovementComponent = CreateDefaultSubobject<USurvivalMovementComponent>(TEXT("SurvivalMovementComponent"));

    // Set default movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASurvivalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ASurvivalCharacter, CurrentMovementMode);
}

void ASurvivalCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateMovementSpeed();
}

void ASurvivalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Process input buffer
    ProcessInputBuffer();
    
    // Handle smooth transitions between movement speeds
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        float CurrentSpeed = MovementComp->MaxWalkSpeed;
        if (FMath::Abs(CurrentSpeed - TargetSpeed) > 1.0f)
        {
            float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, TransitionSpeed);
            MovementComp->MaxWalkSpeed = NewSpeed;
        }
    }
    
    // Update stamina based on current movement and apply stamina effects to movement
    if (StaminaComponent && SurvivalMovementComponent)
    {
        float TerrainMultiplier = 1.0f / SurvivalMovementComponent->GetTerrainResistance();
        StaminaComponent->UpdateCalorieBurn(DeltaTime, static_cast<int32>(CurrentMovementMode), TerrainMultiplier);
        
        // Apply stamina effects to movement speed
        float StaminaPercentage = StaminaComponent->GetCaloriePercentage();
        SurvivalMovementComponent->ApplyStaminaModifiedSpeed(StaminaPercentage);
        
        // Sync stamina with PlayerState on server
        if (HasAuthority())
        {
            if (ASurvivalPlayerState* SurvivalPS = GetPlayerState<ASurvivalPlayerState>())
            {
                SurvivalPS->UpdateCalories(StaminaComponent->GetCurrentCalories());
                
                // Update distance traveled
                if (GetVelocity().Size() > 10.0f) // Only count if actually moving
                {
                    float DistanceThisFrame = GetVelocity().Size() * DeltaTime;
                    float NewTotalDistance = SurvivalPS->GetDistanceTraveled() + DistanceThisFrame;
                    SurvivalPS->UpdateDistanceTraveled(NewTotalDistance);
                }
            }
        }
    }
}

void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind movement mode switching
    PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ASurvivalCharacter::SwitchToWalk);
    PlayerInputComponent->BindAction("Jog", IE_Pressed, this, &ASurvivalCharacter::SwitchToJog);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalCharacter::SwitchToSprint);
}

void ASurvivalCharacter::SetMovementMode(EMovementMode NewMode)
{
    if (HasAuthority())
    {
        // Server execution
        if (CurrentMovementMode != NewMode)
        {
            CurrentMovementMode = NewMode;
            UpdateMovementSpeed();
            MulticastSetMovementMode(NewMode);
        }
    }
    else
    {
        // Client requests server to change mode
        ServerSetMovementMode(NewMode);
    }
}

void ASurvivalCharacter::ServerSetMovementMode_Implementation(EMovementMode NewMode)
{
    SetMovementMode(NewMode);
}

bool ASurvivalCharacter::ServerSetMovementMode_Validate(EMovementMode NewMode)
{
    // Basic validation - mode should be valid enum value
    return NewMode >= EMovementMode::Walk && NewMode <= EMovementMode::Sprint;
}

void ASurvivalCharacter::MulticastSetMovementMode_Implementation(EMovementMode NewMode)
{
    if (!HasAuthority())
    {
        // Client receives server's authoritative movement mode
        CurrentMovementMode = NewMode;
        UpdateMovementSpeed();
    }
}

void ASurvivalCharacter::SwitchToWalk()
{
    AddToInputBuffer(EMovementMode::Walk);
}

void ASurvivalCharacter::SwitchToJog()
{
    AddToInputBuffer(EMovementMode::Jog);
}

void ASurvivalCharacter::SwitchToSprint()
{
    AddToInputBuffer(EMovementMode::Sprint);
}

void ASurvivalCharacter::UpdateMovementSpeed()
{
    switch (CurrentMovementMode)
    {
    case EMovementMode::Walk:
        TargetSpeed = WalkSpeed;
        break;
    case EMovementMode::Jog:
        TargetSpeed = JogSpeed;
        break;
    case EMovementMode::Sprint:
        TargetSpeed = SprintSpeed;
        break;
    }
}

void ASurvivalCharacter::ProcessInputBuffer()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clear old inputs from buffer
    InputBuffer.RemoveAll([&](const EMovementMode& Mode) {
        return (CurrentTime - LastInputTime) > InputBufferTime;
    });
    
    // Process the most recent input if any
    if (InputBuffer.Num() > 0)
    {
        EMovementMode NewMode = InputBuffer.Last();
        if (CurrentMovementMode != NewMode)
        {
            CurrentMovementMode = NewMode;
            UpdateMovementSpeed();
        }
        InputBuffer.Empty();
    }
}

void ASurvivalCharacter::AddToInputBuffer(EMovementMode Mode)
{
    LastInputTime = GetWorld()->GetTimeSeconds();
    InputBuffer.Add(Mode);
}