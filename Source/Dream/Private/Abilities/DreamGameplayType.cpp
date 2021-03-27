#include "DreamGameplayType.h"
#include "WeaknessInterface.h"

float FDreamGameplayEffectContext::GetDamageFalloffPercentage()
{
    return (DistanceDamageFalloff / 100.f);
}

float FDreamGameplayEffectContext::GetWeakPointIncreaseDamagePercentage() const
{
    if (HitResult.IsValid())
    {
        if (IWeaknessInterface* WeaknessInterface = Cast<IWeaknessInterface>(HitResult->GetActor()))
        {
            return WeaknessInterface->GetWeaknessIncreaseDamagePercentage(HitResult->BoneName);
        }
    }
    return 0.f;
}

bool FDreamGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    if (!Super::NetSerialize(Ar, Map, bOutSuccess))
    {
        return false;
    }

    Ar << DistanceDamageFalloff;
    Ar << bDamageCritical;

    bOutSuccess = true;
    return true;
}

FGameplayEffectContext* FDreamGameplayEffectContext::Duplicate() const
{
    FDreamGameplayEffectContext* NewContext = new FDreamGameplayEffectContext();
    *NewContext = *this;
    NewContext->AddActors(Actors);
    //NewContext->AddHitPoints(HitPoints);
    if (GetHitResult())
    {
        // Does a deep copy of the hit result
        NewContext->AddHitResult(*GetHitResult(), true);
    }
    return NewContext;
}
