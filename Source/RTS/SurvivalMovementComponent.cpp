#include "SurvivalMovementComponent.h"
#include "Engine/Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

USurvivalMovementComponent::USurvivalMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    TerrainResistanceMultiplier = 1.0f;
    StaminaDrainMultiplier = 1.0f;
    bApplyTerrainEffects = true;
    BaseMaxSpeed = 600.0f;
    
    CurrentTerrainType = ETerrainType::Unknown;
    LastTerrainType = ETerrainType::Unknown;
    CurrentBiome = EBiomeType::Forest;
    LastBiome = EBiomeType::Forest;
    FootstepTimer = 0.0f;
    FootstepInterval = 0.5f; // Default footstep interval
    
    BiomeSpeedMultiplier = 1.0f;
    BiomeStaminaMultiplier = 1.0f;
    BiomeManager = nullptr;
    
    // Initialize sound assets to nullptr
    GrassFootstepSound = nullptr;
    MudFootstepSound = nullptr;
    WaterFootstepSound = nullptr;
    RockFootstepSound = nullptr;
}

void USurvivalMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    BaseMaxSpeed = MaxWalkSpeed;
    
    // Find BiomeManager in the world if not manually assigned
    if (!BiomeManager)
    {
        BiomeManager = Cast<ASurvivalBiomeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASurvivalBiomeManager::StaticClass()));
    }
}

void USurvivalMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bApplyTerrainEffects)
    {
        DetectTerrainType();
        DetectBiomeEffects();
    }

    // Handle footstep audio timing
    if (GetCurrentVelocity().Size() > 50.0f) // Only when moving
    {
        FootstepTimer += DeltaTime;
        float AdjustedInterval = FootstepInterval / FMath::Max(GetCurrentVelocity().Size() / 300.0f, 0.5f);
        
        if (FootstepTimer >= AdjustedInterval)
        {
            PlayFootstepSound();
            FootstepTimer = 0.0f;
        }
    }
    else
    {
        FootstepTimer = 0.0f;
    }
}

void USurvivalMovementComponent::SetTerrainResistance(float Multiplier)
{
    TerrainResistanceMultiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
}

void USurvivalMovementComponent::ApplyStaminaModifiedSpeed(float StaminaPercentage)
{
    // Reduce speed as stamina decreases
    float StaminaModifier = FMath::Lerp(0.3f, 1.0f, StaminaPercentage);
    StaminaDrainMultiplier = StaminaModifier;
}

float USurvivalMovementComponent::GetMaxSpeed() const
{
    float ModifiedSpeed = BaseMaxSpeed * TerrainResistanceMultiplier * StaminaDrainMultiplier * BiomeSpeedMultiplier;
    return FMath::Max(ModifiedSpeed, 50.0f); // Minimum movement speed
}

void USurvivalMovementComponent::DetectTerrainType()
{
    if (!GetOwner())
        return;

    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200); // Trace downward 2 meters

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    ETerrainType NewTerrainType = ETerrainType::Unknown;

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams))
    {
        if (Hit.PhysMaterial.IsValid())
        {
            UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get();
            
            // Map physical material to terrain resistance based on design document
            if (PhysMat->GetName().Contains("Grass"))
            {
                SetTerrainResistance(1.0f); // Base speed
                NewTerrainType = ETerrainType::Grass;
            }
            else if (PhysMat->GetName().Contains("Mud"))
            {
                SetTerrainResistance(0.714f); // 1.4x resistance = 1/1.4 speed
                NewTerrainType = ETerrainType::Mud;
            }
            else if (PhysMat->GetName().Contains("Water"))
            {
                SetTerrainResistance(0.625f); // 1.6x resistance = 1/1.6 speed
                NewTerrainType = ETerrainType::Water;
            }
            else if (PhysMat->GetName().Contains("Rock"))
            {
                SetTerrainResistance(0.833f); // 1.2x resistance = 1/1.2 speed
                NewTerrainType = ETerrainType::Rock;
            }
        }
    }

    // Check for terrain type changes and broadcast events
    if (NewTerrainType != CurrentTerrainType)
    {
        LastTerrainType = CurrentTerrainType;
        CurrentTerrainType = NewTerrainType;
        
        // Broadcast terrain change event for UI/effects
        OnTerrainChanged.Broadcast(CurrentTerrainType);
    }
}

void USurvivalMovementComponent::PlayFootstepSound()
{
    USoundBase* FootstepSound = nullptr;
    
    switch (CurrentTerrainType)
    {
    case ETerrainType::Grass:
        FootstepSound = GrassFootstepSound;
        break;
    case ETerrainType::Mud:
        FootstepSound = MudFootstepSound;
        break;
    case ETerrainType::Water:
        FootstepSound = WaterFootstepSound;
        break;
    case ETerrainType::Rock:
        FootstepSound = RockFootstepSound;
        break;
    default:
        FootstepSound = GrassFootstepSound; // Default fallback
        break;
    }

    if (FootstepSound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator,
            1.0f, // Volume
            1.0f  // Pitch
        );
    }
}

void USurvivalMovementComponent::DetectBiomeEffects()
{
    if (!BiomeManager || !GetOwner())
        return;

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    EBiomeType NewBiome = BiomeManager->GetBiomeAtLocation(CurrentLocation);
    
    if (NewBiome != CurrentBiome)
    {
        LastBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        // Get biome zone data for multipliers
        FBiomeZone BiomeData = BiomeManager->GetBiomeZoneAtLocation(CurrentLocation);
        BiomeSpeedMultiplier = BiomeData.MovementSpeedMultiplier;
        BiomeStaminaMultiplier = BiomeData.CalorieBurnMultiplier;
        
        UE_LOG(LogTemp, Log, TEXT("Entered %s biome - Speed: %.2fx, Stamina: %.2fx"), 
               *UEnum::GetValueAsString(CurrentBiome), BiomeSpeedMultiplier, BiomeStaminaMultiplier);
    }
}

EBiomeType USurvivalMovementComponent::GetCurrentBiome() const
{
    return CurrentBiome;
}