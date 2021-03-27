#include "DCharacterBase.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "DreamGameMode.h"
#include "UnrealNetwork.h"
#include "DreamType.h"
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"

ADCharacterBase::ADCharacterBase()
{
    //StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
    AttributeSet = CreateDefaultSubobject<UDreamAttributeSet>(TEXT("DreamAttributeSet"));
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    MiniMapData = CreateDefaultSubobject<UMiniMapDataComponent>(TEXT("MiniMapData"));
    MiniMapData->SetAutoActivate(false);

    AbilitySystem->SetIsReplicated(true);
}

float ADCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                AActor* DamageCauser)
{
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ADCharacterBase::OnDeath(const AActor* Causer)
{
    OnCharacterDeath.Broadcast(this);
}

FDamageResult ADCharacterBase::CalculationDamage(float Damage, AActor* DamageCauser)
{
    return FDamageResult(Damage, false);
}

void ADCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADCharacterBase, PawnStatus);
}

float ADCharacterBase::GetHealth() const
{
    return AttributeSet->GetHealth();
}

float ADCharacterBase::GetMaxHealth() const
{
    return AttributeSet->GetMaxHealth();
}

float ADCharacterBase::GetShield() const
{
    return AttributeSet->GetShield();
}

float ADCharacterBase::GetMaxShield() const
{
    return AttributeSet->GetMaxShield();
}

float ADCharacterBase::GetCriticalRate() const
{
    return AttributeSet->GetCriticalRate();
}

float ADCharacterBase::GetCriticalDamage() const
{
    return AttributeSet->GetCriticalDamage();
}

void ADCharacterBase::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle,
        ADCharacterBase* SourceCharacter, const FGameplayTagContainer& AssetTags)
{
    if (SourceCharacter == nullptr || SourceCharacter->IsPendingKill())
    {
        return;
    }

    const FHitResult* HitResult = Handle.GetHitResult();
    BP_HandleDamage(DamageDone, HitResult ? *HitResult : FHitResult(), SourceCharacter);

    bool bDeath = GetHealth() == 0;
    SourceCharacter->HitEnemy(FDamageTargetInfo(DamageDone, bDeath, Handle));

    if (bDeath)
    {
        OnDeath(SourceCharacter);
    }
}

UAbilitySystemComponent* ADCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystem;
}

void ADCharacterBase::HealthChanged(const FOnAttributeChangeData& AttrData)
{
    BP_OnHealthChanged();

    if (GetLocalRole() != ROLE_Authority)
    {
        if (PawnDeathMontage && GetHealth() == 0)
        {
            PlayAnimMontage(PawnDeathMontage);
        }
    }
}

void ADCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    AbilitySystem->GetGameplayAttributeValueChangeDelegate(DreamAttrStatics().HealthProperty).AddUObject(
        this, &ADCharacterBase::HealthChanged);
    AbilitySystem->GetGameplayAttributeValueChangeDelegate(DreamAttrStatics().ShieldProperty).AddUObject(
        this, &ADCharacterBase::HealthChanged);
}

void ADCharacterBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    TeamID = NewTeamID;
}

UMiniMapDataComponent* ADCharacterBase::GetMiniMapDataComponent() const
{
    return MiniMapData;
}

float ADCharacterBase::GetWeaknessIncreaseDamagePercentage(const FName& BoneName)
{
    float* IncreaseDmgPercentage = WeakPoint.Find(BoneName);
    return IncreaseDmgPercentage ? *IncreaseDmgPercentage : 0.f;
}

FGenericTeamId ADCharacterBase::GetGenericTeamId() const
{
    return TeamID;
}

float ADCharacterBase::GetHealthPercent() const
{
    return GetHealth() / GetMaxHealth();
}

float ADCharacterBase::GetShieldPercent() const
{
    return GetShield() / GetMaxShield();
}
