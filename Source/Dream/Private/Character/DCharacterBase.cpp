#include "DCharacterBase.h"
#include "DMAttributeSet.h"
#include "DPlayerState.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "DreamType.h"
#include "DreamGameplayType.h"
#include "Components/CapsuleComponent.h"

ADCharacterBase::ADCharacterBase()
{
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    IconComponent = CreateDefaultSubobject<UIconComponent>(TEXT("Icon"));
    //AbilitySystem->SetIsReplicated(true);
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

void ADCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    DOREPLIFETIME_CONDITION(ADCharacterBase, Level, COND_OwnerOnly);
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float ADCharacterBase::GetBaseHealth()
{
    if (CharacterAttributes == nullptr)
    {
        return 0.f;
    }
    
    return AbilitySystem->GetNumericAttributeBase(CharacterAttributes->GetHealthAttribute());
}

float ADCharacterBase::GetBaseMaxHealth()
{
    if (CharacterAttributes == nullptr)
    {
        return 0.f;
    }
    
    return AbilitySystem->GetNumericAttributeBase(CharacterAttributes->GetMaxHealthAttribute());
}

bool ADCharacterBase::IsDeath() const
{
    return CharacterAttributes == nullptr ? true : CharacterAttributes->GetHealth() == 0;
}

void ADCharacterBase::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
    ADCharacterBase* DamageInstigator = Cast<ADCharacterBase>(Handle.GetInstigator());
    
    if (DamageInstigator == nullptr || DamageInstigator->IsPendingKill())
    {
        return;
    }

    if (ADPlayerState* State = Cast<ADPlayerState>(DamageInstigator->GetPlayerState()))
    {
        State->RecordDamage(DamageDone);

        if (IsDeath())
        {
            State->IncreaseKills();
        }
    }

    const FHitResult* HitResult = Handle.GetHitResult();
    BP_HandleDamage(DamageDone, HitResult ? *HitResult : FHitResult(), DamageInstigator);

    bool bDeath = IsDeath();
    DamageInstigator->HitEnemy(FDamageTargetInfo(DamageDone, bDeath, Handle), this);

    if (IsDeath())
    {
        OnDeath(DamageInstigator);
    }
}

UAbilitySystemComponent* ADCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystem;
}

void ADCharacterBase::HealthChanged(const FOnAttributeChangeData& AttrData)
{
    BP_OnHealthChanged();

    if (IsDeath())
    {
        Tags.Add(DreamActorTagName::Death);
        
        //GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        
        if (GetLocalRole() != ROLE_Authority)
        {
            GetMesh()->SetAllBodiesSimulatePhysics(true);
            GetMesh()->SetAllBodiesPhysicsBlendWeight(1.f);
        }
    }
}

void ADCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    AbilitySystem->GetGameplayAttributeValueChangeDelegate(DMAttrStatics().HealthProperty).AddUObject(this, &ADCharacterBase::HealthChanged);
}

void ADCharacterBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    TeamID = NewTeamID;
}

UIconComponent* ADCharacterBase::GetIconComponent() const
{
    return IconComponent;
}

void ADCharacterBase::PostInitializeComponents()
{
    Super::PostInitializeComponents();
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
    if (CharacterAttributes == nullptr)
    {
        return 0.f;
    }
    
    float MaxHealth = CharacterAttributes->GetMaxHealth();
    return MaxHealth > 0 ? CharacterAttributes->GetHealth() / MaxHealth : 0;
}