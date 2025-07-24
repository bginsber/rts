#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "SurvivalBiomeManager.h"
#include "SurvivalLandscapeTextureBlender.generated.h"

USTRUCT(BlueprintType)
struct FBiomeTextureLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTexture2D* DiffuseTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTexture2D* NormalTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UTexture2D* RoughnessTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TextureScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendRadius;

    FBiomeTextureLayer()
    {
        BiomeType = EBiomeType::Forest;
        DiffuseTexture = nullptr;
        NormalTexture = nullptr;
        RoughnessTexture = nullptr;
        TextureScale = 10.0f;
        BaseColor = FLinearColor::White;
        BlendRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct FTextureWeightMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<uint8> WeightData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType AssociatedBiome;

    FTextureWeightMap()
    {
        Width = 1009;
        Height = 1009;
        AssociatedBiome = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class RTS_API ASurvivalLandscapeTextureBlender : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalLandscapeTextureBlender();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Blending")
    class ASurvivalBiomeManager* BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Blending")
    class ALandscape* TargetLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Layers")
    TArray<FBiomeTextureLayer> BiomeTextureLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Settings")
    int32 WeightmapResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Settings")
    float BlendSmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Settings")
    bool bUseAltitudeBlending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    class UMaterialParameterCollection* LandscapeMaterialCollection;

public:
    UFUNCTION(BlueprintCallable, Category = "Texture Blending")
    void InitializeTextureLayersForBiomes();

    UFUNCTION(BlueprintCallable, Category = "Texture Blending")
    TArray<FTextureWeightMap> GenerateTextureWeightMaps();

    UFUNCTION(BlueprintCallable, Category = "Texture Blending")
    void ApplyTextureWeightMapsToLandscape(const TArray<FTextureWeightMap>& WeightMaps);

    UFUNCTION(BlueprintCallable, Category = "Texture Blending")
    float CalculateBiomeInfluenceAtLocation(const FVector& WorldLocation, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Texture Blending")
    void UpdateMaterialParameters();

    UFUNCTION(BlueprintCallable, Category = "Texture Layers")
    FBiomeTextureLayer GetTextureLayerForBiome(EBiomeType BiomeType) const;

private:
    void CreateDefaultTextureLayers();
    FTextureWeightMap GenerateWeightMapForBiome(EBiomeType BiomeType);
    uint8 WorldInfluenceToWeightValue(float Influence) const;
    void BlendWeightMapsAtBorders(TArray<FTextureWeightMap>& WeightMaps);
};