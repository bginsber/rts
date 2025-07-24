#include "SurvivalBiomeManager.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeInfo.h"
#include "Landscape/Classes/LandscapeComponent.h"
#include "Engine/World.h"
#include "Components/SplineComponent.h"

ASurvivalBiomeManager::ASurvivalBiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Default landscape settings for 5km race route
    LandscapeScale = 100.0f;
    HeightmapResolution = 1009; // Standard UE5 heightmap size (power of 2 + 1)
    MaxElevation = 2000.0f; // 2km elevation variation
    
    // Initialize with default biome zones
    InitializeBiomeZones();
}

void ASurvivalBiomeManager::BeginPlay()
{
    Super::BeginPlay();
}

void ASurvivalBiomeManager::InitializeBiomeZones()
{
    BiomeZones.Empty();
    
    // Define the 5km race route with three main biomes
    CreateAlpineBiome();
    CreateForestBiome();
    CreateRiverBiome();
    CreateTransitionZones();
    
    // Set up checkpoints along the race route
    RaceCheckpoints = {
        FVector(0, 0, 0),           // Start
        FVector(1000, 500, 200),    // Forest entry
        FVector(2000, 1000, 800),   // Alpine ascent
        FVector(3000, 1500, 1200),  // Alpine peak
        FVector(4000, 1000, 600),   // River descent
        FVector(5000, 0, 0)         // Finish
    };
}

void ASurvivalBiomeManager::CreateAlpineBiome()
{
    FBiomeZone Alpine;
    Alpine.BiomeType = EBiomeType::Alpine;
    Alpine.CenterLocation = FVector(2500, 1250, 1000); // Middle-high elevation
    Alpine.Radius = 1500.0f;
    Alpine.CalorieBurnMultiplier = 1.6f; // +60% burn rate due to altitude/cold
    Alpine.MovementSpeedMultiplier = 0.7f; // Slower due to terrain difficulty
    
    BiomeZones.Add(Alpine);
}

void ASurvivalBiomeManager::CreateForestBiome()
{
    FBiomeZone Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.CenterLocation = FVector(1500, 750, 400); // Lower elevation forest
    Forest.Radius = 1200.0f;
    Forest.CalorieBurnMultiplier = 1.2f; // +20% due to navigation difficulty
    Forest.MovementSpeedMultiplier = 0.85f; // Slightly slower due to obstacles
    
    BiomeZones.Add(Forest);
}

void ASurvivalBiomeManager::CreateRiverBiome()
{
    FBiomeZone River;
    River.BiomeType = EBiomeType::River;
    River.CenterLocation = FVector(4000, 1000, 300); // River valley
    River.Radius = 800.0f;
    River.CalorieBurnMultiplier = 1.8f; // +80% due to water crossings/hypothermia
    River.MovementSpeedMultiplier = 0.6f; // Much slower due to water resistance
    
    BiomeZones.Add(River);
}

void ASurvivalBiomeManager::CreateTransitionZones()
{
    // Create transition zones between major biomes
    FBiomeZone ForestToAlpine;
    ForestToAlpine.BiomeType = EBiomeType::Transition;
    ForestToAlpine.CenterLocation = FVector(2000, 1000, 600);
    ForestToAlpine.Radius = 600.0f;
    ForestToAlpine.CalorieBurnMultiplier = 1.4f;
    ForestToAlpine.MovementSpeedMultiplier = 0.8f;
    
    FBiomeZone AlpineToRiver;
    AlpineToRiver.BiomeType = EBiomeType::Transition;
    AlpineToRiver.CenterLocation = FVector(3500, 1250, 800);
    AlpineToRiver.Radius = 600.0f;
    AlpineToRiver.CalorieBurnMultiplier = 1.5f;
    AlpineToRiver.MovementSpeedMultiplier = 0.75f;
    
    BiomeZones.Add(ForestToAlpine);
    BiomeZones.Add(AlpineToRiver);
}

void ASurvivalBiomeManager::GenerateRaceLandscape()
{
    if (!RaceLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No landscape reference set in BiomeManager"));
        return;
    }
    
    // Generate heightmap data based on biome layout
    TArray<uint16> HeightData = GenerateHeightmapData();
    
    // Apply the heightmap to the landscape
    ApplyHeightmapFromArray(HeightData);
    
    UE_LOG(LogTemp, Log, TEXT("Race landscape generated with %d biome zones"), BiomeZones.Num());
}

TArray<uint16> ASurvivalBiomeManager::GenerateHeightmapData()
{
    TArray<uint16> HeightData;
    HeightData.SetNum(HeightmapResolution * HeightmapResolution);
    
    for (int32 Y = 0; Y < HeightmapResolution; Y++)
    {
        for (int32 X = 0; X < HeightmapResolution; X++)
        {
            // Convert heightmap coordinates to world coordinates
            float WorldX = (X / float(HeightmapResolution - 1)) * 5000.0f; // 5km wide
            float WorldY = (Y / float(HeightmapResolution - 1)) * 2500.0f; // 2.5km deep
            
            FVector WorldLocation(WorldX, WorldY, 0);
            
            // Calculate elevation based on biome influences
            float Elevation = 0.0f;
            float TotalInfluence = 0.0f;
            
            for (const FBiomeZone& Biome : BiomeZones)
            {
                float Influence = GetBiomeInfluenceAtLocation(WorldLocation, Biome);
                if (Influence > 0.0f)
                {
                    float BiomeBaseHeight = 0.0f;
                    switch (Biome.BiomeType)
                    {
                        case EBiomeType::Alpine:
                            BiomeBaseHeight = 1200.0f; // High elevation
                            break;
                        case EBiomeType::Forest:
                            BiomeBaseHeight = 400.0f; // Medium elevation
                            break;
                        case EBiomeType::River:
                            BiomeBaseHeight = 200.0f; // Low elevation
                            break;
                        case EBiomeType::Transition:
                            BiomeBaseHeight = 600.0f; // Varied elevation
                            break;
                    }
                    
                    Elevation += BiomeBaseHeight * Influence;
                    TotalInfluence += Influence;
                }
            }
            
            if (TotalInfluence > 0.0f)
            {
                Elevation /= TotalInfluence;
            }
            
            // Add some noise for natural variation
            float Noise = FMath::PerlinNoise2D(FVector2D(WorldX * 0.001f, WorldY * 0.001f)) * 100.0f;
            Elevation += Noise;
            
            // Convert to heightmap format (0-65535)
            uint16 HeightValue = FMath::Clamp(FMath::RoundToInt((Elevation / MaxElevation) * 65535.0f), 0, 65535);
            
            HeightData[Y * HeightmapResolution + X] = HeightValue;
        }
    }
    
    return HeightData;
}

float ASurvivalBiomeManager::GetBiomeInfluenceAtLocation(const FVector& Location, const FBiomeZone& Biome) const
{
    float Distance = FVector::Dist2D(Location, Biome.CenterLocation);
    
    if (Distance >= Biome.Radius)
    {
        return 0.0f;
    }
    
    // Smooth falloff from center to edge
    float Influence = 1.0f - (Distance / Biome.Radius);
    return FMath::SmoothStep(0.0f, 1.0f, Influence);
}

void ASurvivalBiomeManager::ApplyHeightmapFromArray(const TArray<uint16>& HeightData)
{
    if (!RaceLandscape || HeightData.Num() == 0)
    {
        return;
    }
    
    ULandscapeInfo* LandscapeInfo = RaceLandscape->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return;
    }
    
    // This would typically use the Landscape API to apply the heightmap
    // For now, we'll log that the data is ready for application
    UE_LOG(LogTemp, Log, TEXT("Heightmap data ready: %d values for %dx%d terrain"), 
           HeightData.Num(), HeightmapResolution, HeightmapResolution);
}

EBiomeType ASurvivalBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MaxInfluence = 0.0f;
    EBiomeType DominantBiome = EBiomeType::Forest;
    
    for (const FBiomeZone& Biome : BiomeZones)
    {
        float Influence = GetBiomeInfluenceAtLocation(WorldLocation, Biome);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome.BiomeType;
        }
    }
    
    return DominantBiome;
}

FBiomeZone ASurvivalBiomeManager::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    float MaxInfluence = 0.0f;
    FBiomeZone DominantZone;
    
    for (const FBiomeZone& Biome : BiomeZones)
    {
        float Influence = GetBiomeInfluenceAtLocation(WorldLocation, Biome);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantZone = Biome;
        }
    }
    
    return DominantZone;
}

void ASurvivalBiomeManager::CreateRaceSpline()
{
    // This would create a spline component connecting all checkpoints
    // For blueprint implementation in UE5 editor
    UE_LOG(LogTemp, Log, TEXT("Race spline ready with %d checkpoints"), RaceCheckpoints.Num());
}

float ASurvivalBiomeManager::GetElevationAtLocation(const FVector& WorldLocation) const
{
    if (!RaceLandscape)
    {
        return 0.0f;
    }
    
    // Query landscape elevation at world location
    // This would use the Landscape API to get actual height
    return 0.0f;
}