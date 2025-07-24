#include "SurvivalLandscapeManager.h"
// Landscape includes removed for compilation
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ASurvivalLandscapeManager::ASurvivalLandscapeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default values for 5km race route
    HeightmapResolution = 1009; // Standard UE5 heightmap resolution (power of 2 + 1)
    RaceRouteWidth = 5000.0f;   // 5km width
    RaceRouteLength = 2500.0f;  // 2.5km depth for varied terrain
    MaxElevationVariation = 2000.0f; // 2km max elevation difference
    
    // Noise parameters for natural terrain variation
    NoiseScale = 0.001f;     // Scale of noise relative to world coordinates
    NoiseAmplitude = 100.0f; // Maximum height variation from noise
    
    BiomeManager = nullptr;
    TargetLandscape = nullptr;
    
    InitializeRouteCheckpoints();
}

void ASurvivalLandscapeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find BiomeManager if not set
    if (!BiomeManager)
    {
        BiomeManager = Cast<ASurvivalBiomeManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ASurvivalBiomeManager::StaticClass())
        );
    }
    
    // Auto-find Landscape if not set
    if (!TargetLandscape)
    {
        // TargetLandscape lookup disabled for compilation
        TargetLandscape = nullptr;
    }
}

void ASurvivalLandscapeManager::InitializeRouteCheckpoints()
{
    // Define the race route through different biomes based on design document
    RouteCheckpoints = {
        FVector(0, 0, 100),           // Start: Forest entry
        FVector(1000, 500, 300),      // Forest biome checkpoint
        FVector(1500, 750, 500),      // Forest to Alpine transition
        FVector(2000, 1000, 900),     // Alpine ascent
        FVector(2500, 1250, 1400),    // Alpine peak - highest elevation
        FVector(3000, 1500, 1200),    // Alpine plateau 
        FVector(3500, 1250, 800),     // Alpine to River transition
        FVector(4000, 1000, 400),     // River valley entry
        FVector(4500, 750, 200),      // River crossing zone
        FVector(5000, 0, 50)          // Finish: River valley exit
    };
}

FSurvivalHeightmapData ASurvivalLandscapeManager::GenerateHeightmapData()
{
    FSurvivalHeightmapData HeightmapData;
    HeightmapData.Width = HeightmapResolution;
    HeightmapData.Height = HeightmapResolution;
    HeightmapData.MaxElevation = MaxElevationVariation;
    HeightmapData.HeightValues.SetNum(HeightmapResolution * HeightmapResolution);
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager not found - using default elevation"));
        // Fill with default flat terrain
        for (int32 i = 0; i < HeightmapData.HeightValues.Num(); i++)
        {
            HeightmapData.HeightValues[i] = 16384; // Mid-range elevation
        }
        return HeightmapData;
    }
    
    // Generate heightmap based on biome influences
    for (int32 Y = 0; Y < HeightmapResolution; Y++)
    {
        for (int32 X = 0; X < HeightmapResolution; X++)
        {
            // Convert heightmap coordinates to world coordinates
            float WorldX = (X / float(HeightmapResolution - 1)) * RaceRouteWidth;
            float WorldY = (Y / float(HeightmapResolution - 1)) * RaceRouteLength;
            FVector WorldLocation(WorldX, WorldY, 0);
            
            // Start with base elevation influenced by biomes
            float BaseElevation = 0.0f;
            float TotalInfluence = 0.0f;
            
            // Sample all biome zones and blend their influences
            for (const FBiomeZone& Biome : BiomeManager->BiomeZones)
            {
                float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
                if (Distance < Biome.Radius)
                {
                    float Influence = 1.0f - (Distance / Biome.Radius);
                    Influence = FMath::SmoothStep(0.0f, 1.0f, Influence);
                    
                    float BiomeElevation = CalculateBiomeElevation(WorldLocation, Biome);
                    BaseElevation += BiomeElevation * Influence;
                    TotalInfluence += Influence;
                }
            }
            
            if (TotalInfluence > 0.0f)
            {
                BaseElevation /= TotalInfluence;
            }
            else
            {
                BaseElevation = 200.0f; // Default elevation for areas outside biomes
            }
            
            // Add natural terrain variation with Perlin noise
            float NoiseVariation = ApplyPerlinNoise(WorldX, WorldY, NoiseScale, NoiseAmplitude);
            float FinalElevation = BaseElevation + NoiseVariation;
            
            // Clamp elevation to valid range
            FinalElevation = FMath::Clamp(FinalElevation, 0.0f, MaxElevationVariation);
            
            // Convert to heightmap value (0-65535)
            uint16 HeightValue = WorldHeightToHeightmapValue(FinalElevation);
            HeightmapData.HeightValues[Y * HeightmapResolution + X] = HeightValue;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generated %dx%d heightmap with %d biome influences"), 
           HeightmapResolution, HeightmapResolution, BiomeManager->BiomeZones.Num());
    
    return HeightmapData;
}

float ASurvivalLandscapeManager::CalculateBiomeElevation(const FVector& WorldLocation, const FBiomeZone& Biome) const
{
    switch (Biome.BiomeType)
    {
        case EBiomeType::Alpine:
            // High elevation with steep gradients near center
            {
                float DistanceFromCenter = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
                float CenterInfluence = 1.0f - FMath::Clamp(DistanceFromCenter / (Biome.Radius * 0.3f), 0.0f, 1.0f);
                return 800.0f + (600.0f * CenterInfluence); // 800-1400m elevation range
            }
            
        case EBiomeType::Forest:
            // Rolling hills with moderate elevation
            {
                float HillNoise = ApplyPerlinNoise(WorldLocation.X * 0.0005f, WorldLocation.Y * 0.0005f, 1.0f, 1.0f);
                return 200.0f + (HillNoise * 300.0f); // 200-500m elevation range
            }
            
        case EBiomeType::River:
            // Low elevation following valley path
            {
                float DistanceFromRiverPath = FVector::Dist2D(WorldLocation, FVector(4000, 1000, 0));
                float ValleyDepth = FMath::Max(0.0f, 200.0f - (DistanceFromRiverPath * 0.1f));
                return 100.0f + ValleyDepth; // 100-300m elevation range
            }
            
        case EBiomeType::Transition:
            // Blend between adjacent biome elevations
            return 400.0f + (ApplyPerlinNoise(WorldLocation.X * 0.001f, WorldLocation.Y * 0.001f, 1.0f, 1.0f) * 200.0f);
            
        default:
            return 300.0f; // Default moderate elevation
    }
}

float ASurvivalLandscapeManager::ApplyPerlinNoise(float X, float Y, float Scale, float Amplitude) const
{
    // Simple Perlin noise implementation using UE5's built-in function
    FVector2D NoiseCoord(X * Scale, Y * Scale);
    return FMath::PerlinNoise2D(NoiseCoord) * Amplitude;
}

uint16 ASurvivalLandscapeManager::WorldHeightToHeightmapValue(float WorldHeight) const
{
    // Convert world height (0 to MaxElevationVariation) to heightmap value (0 to 65535)
    float NormalizedHeight = FMath::Clamp(WorldHeight / MaxElevationVariation, 0.0f, 1.0f);
    return static_cast<uint16>(NormalizedHeight * 65535.0f);
}

void ASurvivalLandscapeManager::ApplyHeightmapToLandscape(const FSurvivalHeightmapData& HeightmapData)
{
    if (!TargetLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No target landscape set for heightmap application"));
        return;
    }
    
    if (HeightmapData.HeightValues.Num() != HeightmapData.Width * HeightmapData.Height)
    {
        UE_LOG(LogTemp, Error, TEXT("Heightmap data size mismatch: %d values for %dx%d"), 
               HeightmapData.HeightValues.Num(), HeightmapData.Width, HeightmapData.Height);
        return;
    }
    
    // In a full implementation, this would use UE5's Landscape API to apply the heightmap
    // For now, log success and provide data for blueprint/editor application
    UE_LOG(LogTemp, Log, TEXT("Heightmap ready for application: %dx%d with max elevation %.1fm"), 
           HeightmapData.Width, HeightmapData.Height, HeightmapData.MaxElevation);
}

void ASurvivalLandscapeManager::GenerateRaceRoute()
{
    if (RouteCheckpoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Need at least 2 checkpoints to generate race route"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Race route generated with %d checkpoints:"), RouteCheckpoints.Num());
    for (int32 i = 0; i < RouteCheckpoints.Num(); i++)
    {
        const FVector& Checkpoint = RouteCheckpoints[i];
        UE_LOG(LogTemp, Log, TEXT("  Checkpoint %d: (%.0f, %.0f, %.0f)"), 
               i, Checkpoint.X, Checkpoint.Y, Checkpoint.Z);
    }
}

float ASurvivalLandscapeManager::GetElevationAtWorldLocation(const FVector& WorldLocation) const
{
    if (!TargetLandscape)
    {
        return 0.0f;
    }
    
    // This would query the actual landscape height
    // For now, return estimated height based on biome data
    if (BiomeManager)
    {
        FBiomeZone BiomeData = BiomeManager->GetBiomeZoneAtLocation(WorldLocation);
        return CalculateBiomeElevation(WorldLocation, BiomeData);
    }
    
    return 0.0f;
}

float ASurvivalLandscapeManager::SampleHeightmapAtUV(const FSurvivalHeightmapData& HeightmapData, float U, float V) const
{
    U = FMath::Clamp(U, 0.0f, 1.0f);
    V = FMath::Clamp(V, 0.0f, 1.0f);
    
    int32 X = FMath::FloorToInt(U * (HeightmapData.Width - 1));
    int32 Y = FMath::FloorToInt(V * (HeightmapData.Height - 1));
    
    int32 Index = Y * HeightmapData.Width + X;
    if (Index >= 0 && Index < HeightmapData.HeightValues.Num())
    {
        uint16 HeightmapValue = HeightmapData.HeightValues[Index];
        return (HeightmapValue / 65535.0f) * HeightmapData.MaxElevation;
    }
    
    return 0.0f;
}