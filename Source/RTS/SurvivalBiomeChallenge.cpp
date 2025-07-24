#include "SurvivalBiomeChallenge.h"
#include "SurvivalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ASurvivalBiomeChallenge::ASurvivalBiomeChallenge()
{
    PrimaryActorTick.bCanEverTick = true;
    
    ChallengeCheckInterval = 2.0f; // Check for challenges every 2 seconds
    ChallengeCheckTimer = 0.0f;
    
    // Environmental effect intensities based on design document
    ExposureIntensity = 1.6f;      // +60% calorie burn in alpine conditions
    NavigationDifficulty = 1.2f;   // +20% calorie burn in dense forest
    
    BiomeManager = nullptr;
}

void ASurvivalBiomeChallenge::BeginPlay()
{
    Super::BeginPlay();
    
    // Find BiomeManager if not manually assigned
    if (!BiomeManager)
    {
        BiomeManager = Cast<ASurvivalBiomeManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ASurvivalBiomeManager::StaticClass())
        );
    }
    
    // Initialize all biome challenges
    InitializeBiomeChallenges();
}

void ASurvivalBiomeChallenge::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodically check for characters entering/exiting challenge zones
    ChallengeCheckTimer += DeltaTime;
    if (ChallengeCheckTimer >= ChallengeCheckInterval)
    {
        ChallengeCheckTimer = 0.0f;
        
        // Find all survival characters in the world
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivalCharacter::StaticClass(), FoundCharacters);
        
        for (AActor* Actor : FoundCharacters)
        {
            if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(Actor))
            {
                FVector CharacterLocation = Character->GetActorLocation();
                TArray<FBiomeChallenge> ActiveChallenges = GetActiveChallengesAtLocation(CharacterLocation);
                
                // Apply effects from active challenges
                for (const FBiomeChallenge& Challenge : ActiveChallenges)
                {
                    ApplyChallengeEffects(Character, Challenge);
                }
            }
        }
    }
}

void ASurvivalBiomeChallenge::InitializeBiomeChallenges()
{
    CreateAlpineChallenges();
    CreateForestChallenges();
    CreateRiverChallenges();
    
    int32 TotalChallenges = AlpineChallenges.Num() + ForestChallenges.Num() + RiverChallenges.Num();
    UE_LOG(LogTemp, Log, TEXT("Initialized %d biome challenges"), TotalChallenges);
}

void ASurvivalBiomeChallenge::CreateAlpineChallenges()
{
    AlpineChallenges.Empty();
    
    // Vertical climbing challenge near alpine peak
    FBiomeChallenge ClimbingChallenge;
    ClimbingChallenge.ChallengeType = EChallengeType::VerticalClimb;
    ClimbingChallenge.ChallengeLocation = FVector(2500, 1250, 1200); // Alpine peak area
    ClimbingChallenge.ActivationRadius = 800.0f;
    ClimbingChallenge.IntensityMultiplier = 2.0f; // Double stamina drain during climbing
    ClimbingChallenge.bRequiresTeamCoordination = true;
    ClimbingChallenge.TimeToComplete = 45.0f;
    AlpineChallenges.Add(ClimbingChallenge);
    
    // Altitude exposure across the entire alpine biome
    FBiomeChallenge ExposureChallenge;
    ExposureChallenge.ChallengeType = EChallengeType::Exposure;
    ExposureChallenge.ChallengeLocation = FVector(2500, 1250, 1000); // Alpine center
    ExposureChallenge.ActivationRadius = 1500.0f; // Entire alpine biome
    ExposureChallenge.IntensityMultiplier = ExposureIntensity;
    ExposureChallenge.bRequiresTeamCoordination = false;
    ExposureChallenge.TimeToComplete = 0.0f; // Constant effect
    AlpineChallenges.Add(ExposureChallenge);
    
    // Unstable terrain challenge on steep slopes
    FBiomeChallenge InstabilityChallenge;
    InstabilityChallenge.ChallengeType = EChallengeType::TerrainInstability;
    InstabilityChallenge.ChallengeLocation = FVector(3000, 1500, 1000); // Steep alpine descent
    InstabilityChallenge.ActivationRadius = 600.0f;
    InstabilityChallenge.IntensityMultiplier = 1.5f;
    InstabilityChallenge.bRequiresTeamCoordination = true;
    InstabilityChallenge.TimeToComplete = 30.0f;
    AlpineChallenges.Add(InstabilityChallenge);
}

void ASurvivalBiomeChallenge::CreateForestChallenges()
{
    ForestChallenges.Empty();
    
    // Dense vegetation navigation challenge
    FBiomeChallenge NavigationChallenge;
    NavigationChallenge.ChallengeType = EChallengeType::Navigation;
    NavigationChallenge.ChallengeLocation = FVector(1500, 750, 400); // Forest center
    NavigationChallenge.ActivationRadius = 1200.0f; // Entire forest biome
    NavigationChallenge.IntensityMultiplier = NavigationDifficulty;
    NavigationChallenge.bRequiresTeamCoordination = false;
    NavigationChallenge.TimeToComplete = 0.0f; // Constant effect
    ForestChallenges.Add(NavigationChallenge);
    
    // Resource gathering opportunity zone
    FBiomeChallenge ResourceChallenge;
    ResourceChallenge.ChallengeType = EChallengeType::ResourceGather;
    ResourceChallenge.ChallengeLocation = FVector(1200, 600, 350); // Forest resource area
    ResourceChallenge.ActivationRadius = 400.0f;
    ResourceChallenge.IntensityMultiplier = 0.8f; // Reduced stamina drain when gathering
    ResourceChallenge.bRequiresTeamCoordination = true;
    ResourceChallenge.TimeToComplete = 60.0f; // Time to gather resources
    ForestChallenges.Add(ResourceChallenge);
}

void ASurvivalBiomeChallenge::CreateRiverChallenges()
{
    RiverChallenges.Empty();
    
    // Major water crossing challenge
    FBiomeChallenge WaterCrossing;
    WaterCrossing.ChallengeType = EChallengeType::WaterCrossing;
    WaterCrossing.ChallengeLocation = FVector(4200, 1000, 200); // River crossing point
    WaterCrossing.ActivationRadius = 300.0f;
    WaterCrossing.IntensityMultiplier = 2.5f; // Significant stamina drain in water
    WaterCrossing.bRequiresTeamCoordination = true;
    WaterCrossing.TimeToComplete = 90.0f; // Crossing time
    RiverChallenges.Add(WaterCrossing);
    
    // Hypothermia risk zone throughout river biome
    FBiomeChallenge HypothermiaRisk;
    HypothermiaRisk.ChallengeType = EChallengeType::Hypothermia;
    HypothermiaRisk.ChallengeLocation = FVector(4000, 1000, 300); // River center
    HypothermiaRisk.ActivationRadius = 800.0f; // Entire river biome
    HypothermiaRisk.IntensityMultiplier = 1.8f; // +80% stamina drain from cold/wet
    HypothermiaRisk.bRequiresTeamCoordination = false;
    HypothermiaRisk.TimeToComplete = 0.0f; // Constant effect
    RiverChallenges.Add(HypothermiaRisk);
}

TArray<FBiomeChallenge> ASurvivalBiomeChallenge::GetActiveChallengesAtLocation(const FVector& WorldLocation) const
{
    TArray<FBiomeChallenge> ActiveChallenges;
    
    // Check Alpine challenges
    for (const FBiomeChallenge& Challenge : AlpineChallenges)
    {
        if (IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            ActiveChallenges.Add(Challenge);
        }
    }
    
    // Check Forest challenges
    for (const FBiomeChallenge& Challenge : ForestChallenges)
    {
        if (IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            ActiveChallenges.Add(Challenge);
        }
    }
    
    // Check River challenges
    for (const FBiomeChallenge& Challenge : RiverChallenges)
    {
        if (IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            ActiveChallenges.Add(Challenge);
        }
    }
    
    return ActiveChallenges;
}

bool ASurvivalBiomeChallenge::IsLocationWithinChallenge(const FVector& WorldLocation, const FBiomeChallenge& Challenge) const
{
    float Distance = FVector::Dist(WorldLocation, Challenge.ChallengeLocation);
    return Distance <= Challenge.ActivationRadius;
}

float ASurvivalBiomeChallenge::CalculateAltitudeExposure(const FVector& WorldLocation) const
{
    // Exposure increases with altitude and distance from shelter
    float Elevation = WorldLocation.Z;
    float ExposureMultiplier = 1.0f + (Elevation / 1000.0f) * 0.3f; // +30% per 1000m elevation
    
    return FMath::Clamp(ExposureMultiplier, 1.0f, 2.0f);
}

bool ASurvivalBiomeChallenge::RequiresVerticalClimbing(const FVector& WorldLocation) const
{
    // Check if location is in a vertical climbing zone
    for (const FBiomeChallenge& Challenge : AlpineChallenges)
    {
        if (Challenge.ChallengeType == EChallengeType::VerticalClimb &&
            IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            return true;
        }
    }
    return false;
}

float ASurvivalBiomeChallenge::CalculateNavigationDifficulty(const FVector& WorldLocation) const
{
    // Navigation difficulty based on forest density and terrain complexity
    for (const FBiomeChallenge& Challenge : ForestChallenges)
    {
        if (Challenge.ChallengeType == EChallengeType::Navigation &&
            IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            return Challenge.IntensityMultiplier;
        }
    }
    return 1.0f;
}

float ASurvivalBiomeChallenge::CalculateHypothermiaRisk(const FVector& WorldLocation) const
{
    // Hypothermia risk in river/water areas
    for (const FBiomeChallenge& Challenge : RiverChallenges)
    {
        if (Challenge.ChallengeType == EChallengeType::Hypothermia &&
            IsLocationWithinChallenge(WorldLocation, Challenge))
        {
            float Distance = FVector::Dist(WorldLocation, Challenge.ChallengeLocation);
            float RiskIntensity = 1.0f + ((Challenge.ActivationRadius - Distance) / Challenge.ActivationRadius) * 0.8f;
            return FMath::Clamp(RiskIntensity, 1.0f, Challenge.IntensityMultiplier);
        }
    }
    return 1.0f;
}

void ASurvivalBiomeChallenge::ApplyChallengeEffects(ASurvivalCharacter* Character, const FBiomeChallenge& Challenge)
{
    if (!Character)
        return;
    
    FVector CharacterLocation = Character->GetActorLocation();
    
    switch (Challenge.ChallengeType)
    {
        case EChallengeType::Exposure:
            {
                // Apply altitude exposure effects
                float ExposureMultiplier = CalculateAltitudeExposure(CharacterLocation);
                // Effects would be applied through stamina system
                UE_LOG(LogTemp, VeryVerbose, TEXT("Character at altitude %.0fm - Exposure: %.2fx"), 
                       CharacterLocation.Z, ExposureMultiplier);
            }
            break;
            
        case EChallengeType::VerticalClimb:
            {
                // Increase stamina drain during vertical climbing
                UE_LOG(LogTemp, VeryVerbose, TEXT("Character in vertical climbing zone - Stamina drain: %.2fx"), 
                       Challenge.IntensityMultiplier);
            }
            break;
            
        case EChallengeType::Navigation:
            {
                // Apply forest navigation difficulty
                float NavDifficulty = CalculateNavigationDifficulty(CharacterLocation);
                UE_LOG(LogTemp, VeryVerbose, TEXT("Character in forest - Navigation difficulty: %.2fx"), 
                       NavDifficulty);
            }
            break;
            
        case EChallengeType::Hypothermia:
            {
                // Apply hypothermia risk effects
                float HypothermiaRisk = CalculateHypothermiaRisk(CharacterLocation);
                UE_LOG(LogTemp, VeryVerbose, TEXT("Character in cold zone - Hypothermia risk: %.2fx"), 
                       HypothermiaRisk);
            }
            break;
            
        case EChallengeType::WaterCrossing:
            {
                // Increase stamina drain significantly while crossing water
                UE_LOG(LogTemp, VeryVerbose, TEXT("Character crossing water - Stamina drain: %.2fx"), 
                       Challenge.IntensityMultiplier);
            }
            break;
            
        default:
            break;
    }
}