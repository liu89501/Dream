#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayEffectTypes.h"
#include "DreamGameplayType.generated.h"

UENUM(BlueprintType)
enum class EDDamageType : uint8
{
    Weapon_AssaultRifle,
    Weapon_GrenadeLaunch,
    Weapon_Shotgun,
    Weapon_SniperRifle,
    Weapon_PrecisionRifle,
    Weapon,
    Other
};

USTRUCT()
struct DREAM_API FDreamGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_USTRUCT_BODY()
    
public:
    
    FDreamGameplayEffectContext()
        : DistanceDamageFalloff(100)
        , bDamageCritical(false)
        , DamageType(EDDamageType::Other)
    {
    }

    virtual float GetDamageFalloffPercentage();

    virtual void SetFalloffDamage(float FalloffDamage)
    {
        DistanceDamageFalloff = FalloffDamage * 100;
    }
    
    virtual float GetWeakPointIncreaseDamagePercentage() const;

    void SetDamageCritical(bool bCritical)
    {
        bDamageCritical = bCritical;
    }

    bool GetDamageCritical() const
    {
        return bDamageCritical;
    }

    void SetDamageType(EDDamageType InDamageType)
    {
        DamageType = InDamageType;
    }

    EDDamageType GetDamageType() const
    {
        return DamageType;
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

    UPROPERTY()
    EDDamageType DamageType;
    
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
