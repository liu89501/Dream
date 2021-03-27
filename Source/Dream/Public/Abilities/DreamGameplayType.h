#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayEffectTypes.h"
#include "DreamGameplayType.generated.h"

USTRUCT()
struct DREAM_API FDreamGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_USTRUCT_BODY()
    
public:
    
    FDreamGameplayEffectContext()
        : DistanceDamageFalloff(100)
        , bDamageCritical(false)
    {
    }

    virtual float GetDamageFalloffPercentage();

    virtual void SetFalloffDamage(float FalloffDamage)
    {
        DistanceDamageFalloff = FalloffDamage * 100;
    }
    
    virtual float GetWeakPointIncreaseDamagePercentage() const;

    /*virtual void AddHitPoint(const FVector& HitPoint)
    {
        HitPoints.Add(HitPoint);
    }

    virtual void AddHitPoints(const TArray<FVector>& Points)
    {
        HitPoints.Append(Points);
    }

    virtual const TArray<FVector>& GetHitPoints() const
    {
        return HitPoints;
    }*/

    void SetDamageCritical(bool bCritical)
    {
        bDamageCritical = bCritical;
    }

    FORCEINLINE bool GetDamageCritical() const
    {
        return bDamageCritical;
    }

    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

    virtual FGameplayEffectContext* Duplicate() const override;

    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FDreamGameplayEffectContext::StaticStruct();
    }

private:

    UPROPERTY()
    uint8 DistanceDamageFalloff;

    UPROPERTY()
    bool bDamageCritical;
    
    /*UPROPERTY()
    TArray<FVector> HitPoints;*/
};

template<>
struct TStructOpsTypeTraits< FDreamGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FDreamGameplayEffectContext >
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true
    };
};
