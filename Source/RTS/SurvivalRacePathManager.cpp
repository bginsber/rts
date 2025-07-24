#include "SurvivalRacePathManager.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASurvivalRacePathManager::ASurvivalRacePathManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create spline component for the race path
    RaceSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RaceSpline"));
    RootComponent = RaceSpline;
    
    // Initialize path settings
    DefaultPathWidth = 200.0f;  // 200m wide path
    PathSmoothness = 1.0f;      // Smooth spline interpolation
    TotalRaceDistance = 5000.0f; // 5km total race distance
    
    bShowDebugPath = true;
    CheckpointMarkerMesh = nullptr;
    BiomeManager = nullptr;
}

void ASurvivalRacePathManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find BiomeManager
    if (!BiomeManager)
    {
        BiomeManager = Cast<ASurvivalBiomeManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ASurvivalBiomeManager::StaticClass())
        );
    }
    
    // Initialize and generate the race path
    InitializeRaceCheckpoints();
    GenerateRacePath();
}

void ASurvivalRacePathManager::InitializeRaceCheckpoints()
{
    RaceCheckpoints.Empty();
    
    // Define the race route through all biome zones based on design document
    FRaceCheckpoint StartCheckpoint;
    StartCheckpoint.Location = FVector(0, 0, 100);
    StartCheckpoint.CheckpointName = TEXT("Race Start");
    StartCheckpoint.AssociatedBiome = EBiomeType::Forest;
    StartCheckpoint.Radius = 150.0f;
    StartCheckpoint.SpeedLimit = 400.0f;
    RaceCheckpoints.Add(StartCheckpoint);
    
    FRaceCheckpoint ForestEntry;
    ForestEntry.Location = FVector(800, 400, 250);
    ForestEntry.CheckpointName = TEXT("Forest Entry");
    ForestEntry.AssociatedBiome = EBiomeType::Forest;
    ForestEntry.Radius = 120.0f;
    ForestEntry.SpeedLimit = 350.0f;
    RaceCheckpoints.Add(ForestEntry);
    
    FRaceCheckpoint ForestDeep;
    ForestDeep.Location = FVector(1500, 750, 400);
    ForestDeep.CheckpointName = TEXT("Deep Forest");
    ForestDeep.AssociatedBiome = EBiomeType::Forest;
    ForestDeep.Radius = 100.0f;
    ForestDeep.SpeedLimit = 300.0f; // Slower due to navigation difficulty
    RaceCheckpoints.Add(ForestDeep);
    
    FRaceCheckpoint AlpineBase;
    AlpineBase.Location = FVector(2000, 1000, 700);
    AlpineBase.CheckpointName = TEXT("Alpine Base");
    AlpineBase.AssociatedBiome = EBiomeType::Transition;
    AlpineBase.Radius = 130.0f;
    AlpineBase.SpeedLimit = 250.0f; // Slower for climbing preparation
    RaceCheckpoints.Add(AlpineBase);
    
    FRaceCheckpoint AlpinePeak;
    AlpinePeak.Location = FVector(2500, 1250, 1400);
    AlpinePeak.CheckpointName = TEXT("Alpine Peak");
    AlpinePeak.AssociatedBiome = EBiomeType::Alpine;
    AlpinePeak.Radius = 80.0f;
    AlpinePeak.SpeedLimit = 150.0f; // Very slow at peak elevation
    RaceCheckpoints.Add(AlpinePeak);
    
    FRaceCheckpoint AlpineDescent;
    AlpineDescent.Location = FVector(3000, 1500, 1000);
    AlpineDescent.CheckpointName = TEXT("Alpine Descent");
    AlpineDescent.AssociatedBiome = EBiomeType::Alpine;
    AlpineDescent.Radius = 100.0f;
    AlpineDescent.SpeedLimit = 200.0f; // Careful descent speed
    RaceCheckpoints.Add(AlpineDescent);
    
    FRaceCheckpoint RiverApproach;
    RiverApproach.Location = FVector(3500, 1250, 600);
    RiverApproach.CheckpointName = TEXT("River Approach");
    RiverApproach.AssociatedBiome = EBiomeType::Transition;
    RiverApproach.Radius = 120.0f;
    RiverApproach.SpeedLimit = 300.0f;
    RaceCheckpoints.Add(RiverApproach);
    
    FRaceCheckpoint RiverCrossing;
    RiverCrossing.Location = FVector(4000, 1000, 300);
    RiverCrossing.CheckpointName = TEXT("River Crossing");
    RiverCrossing.AssociatedBiome = EBiomeType::River;
    RiverCrossing.Radius = 90.0f;
    RiverCrossing.SpeedLimit = 100.0f; // Very slow for water crossing
    RaceCheckpoints.Add(RiverCrossing);
    
    FRaceCheckpoint RiverValley;
    RiverValley.Location = FVector(4500, 750, 200);
    RiverValley.CheckpointName = TEXT("River Valley");
    RiverValley.AssociatedBiome = EBiomeType::River;
    RiverValley.Radius = 110.0f;
    RiverValley.SpeedLimit = 250.0f;
    RaceCheckpoints.Add(RiverValley);
    
    FRaceCheckpoint FinishLine;
    FinishLine.Location = FVector(5000, 0, 50);
    FinishLine.CheckpointName = TEXT("Finish Line");
    FinishLine.AssociatedBiome = EBiomeType::River;
    FinishLine.Radius = 200.0f; // Large finish area
    FinishLine.SpeedLimit = 600.0f; // Full speed to finish
    RaceCheckpoints.Add(FinishLine);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized race path with %d checkpoints"), RaceCheckpoints.Num());
}

void ASurvivalRacePathManager::GenerateRacePath()
{
    if (RaceCheckpoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Need at least 2 checkpoints to generate race path"));
        return;
    }
    
    // Update spline with checkpoint locations
    UpdateSplineFromCheckpoints();
    
    // Generate path segments between checkpoints
    GeneratePathSegments();
    
    // Calculate difficulty multipliers for each segment
    CalculateSegmentDifficulties();
    
    // Create visual checkpoint markers
    CreateCheckpointMarkers();
    
    float TotalLength = CalculatePathTotalLength();
    UE_LOG(LogTemp, Log, TEXT("Generated race path: %.0fm total length with %d segments"), 
           TotalLength, PathSegments.Num());
}

void ASurvivalRacePathManager::UpdateSplineFromCheckpoints()
{
    if (!RaceSpline)
        return;
    
    // Clear existing spline points
    RaceSpline->ClearSplinePoints();
    
    // Add checkpoint locations as spline points
    for (int32 i = 0; i < RaceCheckpoints.Num(); i++)
    {
        const FRaceCheckpoint& Checkpoint = RaceCheckpoints[i];
        
        // Interpolate elevation if we have landscape data
        FVector SplinePoint = InterpolatePathElevation(Checkpoint.Location);
        
        RaceSpline->AddSplinePoint(SplinePoint, ESplineCoordinateSpace::World);
        RaceSpline->SetSplinePointType(i, ESplinePointType::CurveClamped);
    }
    
    // Update spline with smooth tangents
    RaceSpline->UpdateSpline();
    
    // Apply smoothness factor to tangents
    for (int32 i = 0; i < RaceSpline->GetNumberOfSplinePoints(); i++)
    {
        FVector Tangent = RaceSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
        RaceSpline->SetTangentAtSplinePoint(i, Tangent * PathSmoothness, ESplineCoordinateSpace::World);
    }
    
    RaceSpline->UpdateSpline();
}

FVector ASurvivalRacePathManager::InterpolatePathElevation(const FVector& BaseLocation) const
{
    FVector Result = BaseLocation;
    
    // If we have a BiomeManager, get more accurate elevation
    if (BiomeManager)
    {
        // This would query the landscape or biome system for actual elevation
        // For now, use the Z value from the checkpoint
    }
    
    return Result;
}

void ASurvivalRacePathManager::GeneratePathSegments()
{
    PathSegments.Empty();
    
    for (int32 i = 0; i < RaceCheckpoints.Num() - 1; i++)
    {
        FPathSegment Segment;
        Segment.StartPoint = RaceCheckpoints[i].Location;
        Segment.EndPoint = RaceCheckpoints[i + 1].Location;
        Segment.PathWidth = DefaultPathWidth;
        
        // Determine segment biome (use the destination checkpoint's biome)
        Segment.SegmentBiome = RaceCheckpoints[i + 1].AssociatedBiome;
        
        // Set recommended speed based on terrain difficulty
        Segment.RecommendedSpeed = (RaceCheckpoints[i].SpeedLimit + RaceCheckpoints[i + 1].SpeedLimit) * 0.5f;
        
        PathSegments.Add(Segment);
    }
}

void ASurvivalRacePathManager::CalculateSegmentDifficulties()
{
    for (FPathSegment& Segment : PathSegments)
    {
        // Calculate difficulty based on elevation change and biome type
        float ElevationChange = FMath::Abs(Segment.EndPoint.Z - Segment.StartPoint.Z);
        float Distance = FVector::Dist(Segment.StartPoint, Segment.EndPoint);
        float Gradient = Distance > 0 ? ElevationChange / Distance : 0.0f;
        
        // Base difficulty from gradient
        float GradientDifficulty = 1.0f + (Gradient * 2.0f); // +2x difficulty per unit gradient
        
        // Biome-specific difficulty multipliers
        float BiomeDifficulty = 1.0f;
        switch (Segment.SegmentBiome)
        {
            case EBiomeType::Alpine:
                BiomeDifficulty = 1.6f; // High altitude difficulty
                break;
            case EBiomeType::Forest:
                BiomeDifficulty = 1.2f; // Navigation difficulty
                break;
            case EBiomeType::River:
                BiomeDifficulty = 1.8f; // Water crossing difficulty
                break;
            case EBiomeType::Transition:
                BiomeDifficulty = 1.3f; // Moderate transition difficulty
                break;
        }
        
        Segment.DifficultyMultiplier = GradientDifficulty * BiomeDifficulty;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Segment difficulty: %.2f (gradient: %.3f, biome: %.2f)"), 
               Segment.DifficultyMultiplier, GradientDifficulty, BiomeDifficulty);
    }
}

void ASurvivalRacePathManager::CreateCheckpointMarkers()
{
    // In a full implementation, this would create visual marker actors at each checkpoint
    for (int32 i = 0; i < RaceCheckpoints.Num(); i++)
    {
        const FRaceCheckpoint& Checkpoint = RaceCheckpoints[i];
        
        if (bShowDebugPath)
        {
            // Draw debug spheres for checkpoints in development
            FColor CheckpointColor = FColor::Green;
            if (i == 0)
                CheckpointColor = FColor::Blue; // Start
            else if (i == RaceCheckpoints.Num() - 1)
                CheckpointColor = FColor::Red; // Finish
            
            // This would be replaced with actual mesh spawning in a full implementation
            UE_LOG(LogTemp, VeryVerbose, TEXT("Checkpoint %d: %s at (%.0f, %.0f, %.0f)"), 
                   i, *Checkpoint.CheckpointName, 
                   Checkpoint.Location.X, Checkpoint.Location.Y, Checkpoint.Location.Z);
        }
    }
}

float ASurvivalRacePathManager::GetDistanceAlongPath(const FVector& WorldLocation) const
{
    if (!RaceSpline)
        return 0.0f;
    
    // Find the closest point on the spline and return its distance along the path
    FVector ClosestLocation = RaceSpline->FindLocationClosestToWorldLocation(WorldLocation, ESplineCoordinateSpace::World);
    float SplineKey = RaceSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
    
    return RaceSpline->GetDistanceAlongSplineAtSplineInputKey(SplineKey);
}

FVector ASurvivalRacePathManager::GetPathLocationAtDistance(float Distance) const
{
    if (!RaceSpline)
        return FVector::ZeroVector;
    
    return RaceSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRaceCheckpoint ASurvivalRacePathManager::GetNearestCheckpoint(const FVector& WorldLocation) const
{
    FRaceCheckpoint NearestCheckpoint;
    float NearestDistance = FLT_MAX;
    
    for (const FRaceCheckpoint& Checkpoint : RaceCheckpoints)
    {
        float Distance = FVector::Dist(WorldLocation, Checkpoint.Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestCheckpoint = Checkpoint;
        }
    }
    
    return NearestCheckpoint;
}

bool ASurvivalRacePathManager::IsLocationOnPath(const FVector& WorldLocation, float Tolerance) const
{
    if (!RaceSpline)
        return false;
    
    FVector ClosestPoint = RaceSpline->FindLocationClosestToWorldLocation(WorldLocation, ESplineCoordinateSpace::World);
    float Distance = FVector::Dist(WorldLocation, ClosestPoint);
    
    return Distance <= Tolerance;
}

float ASurvivalRacePathManager::GetRecommendedSpeedAtLocation(const FVector& WorldLocation) const
{
    // Find the path segment containing this location
    float PathDistance = GetDistanceAlongPath(WorldLocation);
    float AccumulatedDistance = 0.0f;
    
    for (const FPathSegment& Segment : PathSegments)
    {
        float SegmentLength = FVector::Dist(Segment.StartPoint, Segment.EndPoint);
        
        if (PathDistance <= AccumulatedDistance + SegmentLength)
        {
            // Adjust recommended speed based on difficulty
            return Segment.RecommendedSpeed / Segment.DifficultyMultiplier;
        }
        
        AccumulatedDistance += SegmentLength;
    }
    
    // Default speed if not found
    return 400.0f;
}

EBiomeType ASurvivalRacePathManager::GetPathBiomeAtLocation(const FVector& WorldLocation) const
{
    FRaceCheckpoint NearestCheckpoint = GetNearestCheckpoint(WorldLocation);
    return NearestCheckpoint.AssociatedBiome;
}

float ASurvivalRacePathManager::CalculatePathTotalLength() const
{
    if (!RaceSpline)
        return 0.0f;
    
    return RaceSpline->GetSplineLength();
}

TArray<float> ASurvivalRacePathManager::GetElevationProfile(int32 SampleCount) const
{
    TArray<float> ElevationProfile;
    
    if (!RaceSpline || SampleCount <= 0)
        return ElevationProfile;
    
    float TotalLength = CalculatePathTotalLength();
    float SampleDistance = TotalLength / SampleCount;
    
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Distance = i * SampleDistance;
        FVector Location = GetPathLocationAtDistance(Distance);
        ElevationProfile.Add(Location.Z);
    }
    
    return ElevationProfile;
}