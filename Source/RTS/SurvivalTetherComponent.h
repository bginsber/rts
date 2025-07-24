#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalTetherComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTS_API USurvivalTetherComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalTetherComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
    float MaxTetherDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
    class ASurvivalCharacter* TetheredPartner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
    float TetherTension;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Tether")
    void SetTetheredPartner(ASurvivalCharacter* Partner);

    UFUNCTION(BlueprintCallable, Category = "Tether")
    float GetDistanceToPartner() const;

    UFUNCTION(BlueprintCallable, Category = "Tether")
    bool IsWithinTetherRange() const;

    UFUNCTION(BlueprintCallable, Category = "Tether")
    float GetTetherTension() const { return TetherTension; }

private:
    void UpdateTetherConstraints();
};