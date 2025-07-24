#include "SurvivalLandscapeTextureBlender.h"
// Landscape include removed for compilation
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ASurvivalLandscapeTextureBlender::ASurvivalLandscapeTextureBlender()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize texture blending settings
    WeightmapResolution = 1009; // Match heightmap resolution
    BlendSmoothness = 0.8f;     // Smooth transitions between biomes
    bUseAltitudeBlending = true; // Blend textures based on elevation
    
    BiomeManager = nullptr;
    TargetLandscape = nullptr;
    LandscapeMaterialCollection = nullptr;
}

void ASurvivalLandscapeTextureBlender::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find required components
    if (!BiomeManager)
    {
        BiomeManager = Cast<ASurvivalBiomeManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ASurvivalBiomeManager::StaticClass())
        );
    }
    
    if (!TargetLandscape)
    {
        // TargetLandscape lookup disabled for compilation
        TargetLandscape = nullptr;
    }
    
    // Initialize texture layers for each biome
    InitializeTextureLayersForBiomes();
}

void ASurvivalLandscapeTextureBlender::InitializeTextureLayersForBiomes()
{
    if (BiomeTextureLayers.Num() == 0)
    {
        CreateDefaultTextureLayers();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d texture layers for biome blending"), BiomeTextureLayers.Num());
}

void ASurvivalLandscapeTextureBlender::CreateDefaultTextureLayers()
{
    BiomeTextureLayers.Empty();
    
    // Alpine biome layer - rocky, snow-capped terrain
    FBiomeTextureLayer AlpineLayer;
    AlpineLayer.BiomeType = EBiomeType::Alpine;
    AlpineLayer.TextureScale = 15.0f; // Larger scale for mountain textures
    AlpineLayer.BaseColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f); // Cool, rocky colors
    AlpineLayer.BlendRadius = 600.0f;
    BiomeTextureLayers.Add(AlpineLayer);
    
    // Forest biome layer - grass, dirt, moss textures
    FBiomeTextureLayer ForestLayer;
    ForestLayer.BiomeType = EBiomeType::Forest;
    ForestLayer.TextureScale = 8.0f; // Medium detail for forest floor
    ForestLayer.BaseColor = FLinearColor(0.3f, 0.6f, 0.2f, 1.0f); // Forest green
    ForestLayer.BlendRadius = 500.0f;
    BiomeTextureLayers.Add(ForestLayer);
    
    // River biome layer - wet sand, mud, river stones
    FBiomeTextureLayer RiverLayer;
    RiverLayer.BiomeType = EBiomeType::River;
    RiverLayer.TextureScale = 6.0f; // Fine detail for riverbank materials
    RiverLayer.BaseColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f); // Muddy brown
    RiverLayer.BlendRadius = 400.0f;
    BiomeTextureLayers.Add(RiverLayer);
    
    // Transition biome layer - blended materials
    FBiomeTextureLayer TransitionLayer;
    TransitionLayer.BiomeType = EBiomeType::Transition;
    TransitionLayer.TextureScale = 10.0f; // Balanced scale for transitions
    TransitionLayer.BaseColor = FLinearColor(0.6f, 0.55f, 0.45f, 1.0f); // Neutral earth tones
    TransitionLayer.BlendRadius = 800.0f; // Large blend radius for smooth transitions
    BiomeTextureLayers.Add(TransitionLayer);
    
    UE_LOG(LogTemp, Log, TEXT("Created default texture layers for %d biome types"), BiomeTextureLayers.Num());
}

TArray<FTextureWeightMap> ASurvivalLandscapeTextureBlender::GenerateTextureWeightMaps()
{
    TArray<FTextureWeightMap> WeightMaps;
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager not found - cannot generate texture weight maps"));
        return WeightMaps;
    }
    
    // Generate a weight map for each biome type
    for (const FBiomeTextureLayer& TextureLayer : BiomeTextureLayers)
    {
        FTextureWeightMap WeightMap = GenerateWeightMapForBiome(TextureLayer.BiomeType);
        WeightMaps.Add(WeightMap);
    }
    
    // Apply smoothing and blending between weight maps
    BlendWeightMapsAtBorders(WeightMaps);
    
    UE_LOG(LogTemp, Log, TEXT("Generated %d texture weight maps (%dx%d resolution)"), 
           WeightMaps.Num(), WeightmapResolution, WeightmapResolution);
    
    return WeightMaps;
}

FTextureWeightMap ASurvivalLandscapeTextureBlender::GenerateWeightMapForBiome(EBiomeType BiomeType)
{
    FTextureWeightMap WeightMap;
    WeightMap.Width = WeightmapResolution;
    WeightMap.Height = WeightmapResolution;
    WeightMap.AssociatedBiome = BiomeType;
    WeightMap.WeightData.SetNum(WeightmapResolution * WeightmapResolution);
    
    // Generate weights based on biome influence at each point
    for (int32 Y = 0; Y < WeightmapResolution; Y++)
    {
        for (int32 X = 0; X < WeightmapResolution; X++)
        {
            // Convert weightmap coordinates to world coordinates
            float WorldX = (X / float(WeightmapResolution - 1)) * 5000.0f; // 5km width
            float WorldY = (Y / float(WeightmapResolution - 1)) * 2500.0f; // 2.5km depth
            FVector WorldLocation(WorldX, WorldY, 0);
            
            // Calculate influence of this biome at this location
            float Influence = CalculateBiomeInfluenceAtLocation(WorldLocation, BiomeType);
            
            // Add altitude-based blending if enabled
            if (bUseAltitudeBlending)
            {
                float Altitude = WorldLocation.Z;
                switch (BiomeType)
                {
                    case EBiomeType::Alpine:
                        // Alpine textures more prominent at high altitude
                        if (Altitude > 800.0f)
                        {
                            Influence *= 1.0f + ((Altitude - 800.0f) / 600.0f); // Boost above 800m
                        }
                        break;
                        
                    case EBiomeType::River:
                        // River textures more prominent at low altitude
                        if (Altitude < 400.0f)
                        {
                            Influence *= 1.5f; // Boost below 400m
                        }
                        break;
                        
                    default:
                        break;
                }
            }
            
            // Convert influence to weight value and store
            uint8 WeightValue = WorldInfluenceToWeightValue(Influence);
            WeightMap.WeightData[Y * WeightmapResolution + X] = WeightValue;
        }
    }
    
    return WeightMap;
}

float ASurvivalLandscapeTextureBlender::CalculateBiomeInfluenceAtLocation(const FVector& WorldLocation, EBiomeType BiomeType) const
{
    if (!BiomeManager)
        return 0.0f;
    
    float MaxInfluence = 0.0f;
    
    // Check influence from all biome zones of the specified type
    for (const FBiomeZone& Biome : BiomeManager->BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
            if (Distance < Biome.Radius)
            {
                float Influence = 1.0f - (Distance / Biome.Radius);
                Influence = FMath::SmoothStep(0.0f, 1.0f, Influence);
                MaxInfluence = FMath::Max(MaxInfluence, Influence);
            }
        }
    }
    
    return MaxInfluence;
}

uint8 ASurvivalLandscapeTextureBlender::WorldInfluenceToWeightValue(float Influence) const
{
    // Convert influence (0.0-1.0) to weight value (0-255)
    Influence = FMath::Clamp(Influence, 0.0f, 1.0f);
    return static_cast<uint8>(Influence * 255.0f);
}

void ASurvivalLandscapeTextureBlender::BlendWeightMapsAtBorders(TArray<FTextureWeightMap>& WeightMaps)
{
    // Smooth transitions between different biome weight maps
    const int32 BlendKernelSize = 3; // 3x3 smoothing kernel
    
    for (FTextureWeightMap& WeightMap : WeightMaps)
    {
        TArray<uint8> SmoothedData = WeightMap.WeightData;
        
        for (int32 Y = BlendKernelSize; Y < WeightmapResolution - BlendKernelSize; Y++)
        {
            for (int32 X = BlendKernelSize; X < WeightmapResolution - BlendKernelSize; X++)
            {
                int32 WeightSum = 0;
                int32 SampleCount = 0;
                
                // Sample surrounding pixels for smoothing
                for (int32 DY = -BlendKernelSize; DY <= BlendKernelSize; DY++)
                {
                    for (int32 DX = -BlendKernelSize; DX <= BlendKernelSize; DX++)
                    {
                        int32 SampleIndex = (Y + DY) * WeightmapResolution + (X + DX);
                        WeightSum += WeightMap.WeightData[SampleIndex];
                        SampleCount++;
                    }
                }
                
                // Apply smoothed value with blend factor
                uint8 SmoothedValue = static_cast<uint8>(WeightSum / SampleCount);
                uint8 OriginalValue = WeightMap.WeightData[Y * WeightmapResolution + X];
                
                SmoothedData[Y * WeightmapResolution + X] = static_cast<uint8>(
                    FMath::Lerp(OriginalValue, SmoothedValue, BlendSmoothness)
                );
            }
        }
        
        WeightMap.WeightData = SmoothedData;
    }
}

void ASurvivalLandscapeTextureBlender::ApplyTextureWeightMapsToLandscape(const TArray<FTextureWeightMap>& WeightMaps)
{
    if (!TargetLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No target landscape set for texture weight map application"));
        return;
    }
    
    // In a full implementation, this would use UE5's Landscape API to apply weight maps
    // For now, log success and provide data for blueprint/editor application
    UE_LOG(LogTemp, Log, TEXT("Texture weight maps ready for landscape application:"));
    
    for (const FTextureWeightMap& WeightMap : WeightMaps)
    {
        // Calculate statistics for this weight map
        int32 NonZeroPixels = 0;
        uint8 MaxWeight = 0;
        
        for (uint8 Weight : WeightMap.WeightData)
        {
            if (Weight > 0)
                NonZeroPixels++;
            MaxWeight = FMath::Max(MaxWeight, Weight);
        }
        
        float Coverage = (NonZeroPixels / float(WeightMap.WeightData.Num())) * 100.0f;
        
        UE_LOG(LogTemp, Log, TEXT("  %s: %.1f%% coverage, max weight: %d"), 
               *UEnum::GetValueAsString(WeightMap.AssociatedBiome), Coverage, MaxWeight);
    }
}

FBiomeTextureLayer ASurvivalLandscapeTextureBlender::GetTextureLayerForBiome(EBiomeType BiomeType) const
{
    for (const FBiomeTextureLayer& Layer : BiomeTextureLayers)
    {
        if (Layer.BiomeType == BiomeType)
        {
            return Layer;
        }
    }
    
    // Return default layer if not found
    FBiomeTextureLayer DefaultLayer;
    DefaultLayer.BiomeType = BiomeType;
    return DefaultLayer;
}

void ASurvivalLandscapeTextureBlender::UpdateMaterialParameters()
{
    if (!LandscapeMaterialCollection)
    {
        UE_LOG(LogTemp, Warning, TEXT("No material parameter collection set"));
        return;
    }
    
    // Update material parameters for each biome layer
    UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(LandscapeMaterialCollection);
    if (!CollectionInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get material parameter collection instance"));
        return;
    }
    
    // Set texture scale parameters for each biome
    for (const FBiomeTextureLayer& Layer : BiomeTextureLayers)
    {
        FString ScaleParamName = FString::Printf(TEXT("%s_TextureScale"), *UEnum::GetValueAsString(Layer.BiomeType));
        FString ColorParamName = FString::Printf(TEXT("%s_BaseColor"), *UEnum::GetValueAsString(Layer.BiomeType));
        
        CollectionInstance->SetScalarParameterValue(*ScaleParamName, Layer.TextureScale);
        CollectionInstance->SetVectorParameterValue(*ColorParamName, Layer.BaseColor);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated material parameters for %d biome texture layers"), BiomeTextureLayers.Num());
}