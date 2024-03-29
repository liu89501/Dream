﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "IconInterface.h"
#include "WeaknessInterface.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "DreamType.h"
#include "DCharacterBase.generated.h"

class UDMAttributeSet;
class UIconComponent;

USTRUCT()
struct FDamageTargetInfo
{
    GENERATED_USTRUCT_BODY()

    FDamageTargetInfo() = default;

    FDamageTargetInfo(float DamageAmount, bool bKilled, const FGameplayEffectContextHandle& Handle)
        : DamageAmount(DamageAmount),
          bKilled(bKilled),
          Handle(Handle)
    {
    }
    
    UPROPERTY()
    float DamageAmount;
    UPROPERTY()
    bool bKilled;
    UPROPERTY()
    FGameplayEffectContextHandle Handle;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FCharacterDeathSignature, class ADCharacterBase*)

UCLASS(Abstract)
class DREAM_API ADCharacterBase : public ACharacter, public IGenericTeamAgentInterface,
                                public IAbilitySystemInterface, public IIconInterface,
                                public IWeaknessInterface
{
    GENERATED_BODY()

public:

    ADCharacterBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UIconComponent* IconComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharacterBase)
    FGenericTeamId TeamID;

    UPROPERTY(BlueprintReadOnly, Category = CharacterBase)
    UDMAttributeSet* CharacterAttributes;

    UPROPERTY(EditAnywhere, Category = CharacterBase)
    TMap<FName, float> WeakPoint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterBase)
    EPawnType PawnType;

public:

    UFUNCTION(BlueprintCallable, Category = CharacterBase)
    float GetBaseHealth();

    UFUNCTION(BlueprintCallable, Category = CharacterBase)
    float GetBaseMaxHealth();

    UFUNCTION(BlueprintCallable, Category = CharacterBase)
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = CharacterBase)
    bool IsDeath() const;

    virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle);

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    FORCEINLINE int32 GetCharacterLevel() const
    {
        return Level;
    }

    FCharacterDeathSignature& GetCharacterDeathDelegate()
    {
        return OnCharacterDeath;
    }

protected:

    UPROPERTY(EditAnywhere, Replicated, Category = CharacterBase)
    int32 Level;

    FCharacterDeathSignature OnCharacterDeath;

protected:

    UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHandleDamage", ScriptName = "OnHandleDamage"))
    void BP_HandleDamage(float Damage, const FHitResult& HitResult, ADCharacterBase* SourceCharacter);

    UFUNCTION(BlueprintImplementableEvent, Category = CharacterBase, meta = (DisplayName = "OnHealthChanged", ScriptName = "OnHealthChanged"))
    void BP_OnHealthChanged();

    virtual void HealthChanged(const FOnAttributeChangeData& AttrData);

    virtual void BeginPlay() override;

    /** Assigns Team Agent to given TeamID */
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    virtual UIconComponent* GetIconComponent() const override;

    virtual void PostInitializeComponents() override;

    virtual float GetWeaknessIncreaseDamagePercentage(const FName& BoneName) override;

    /** Retrieve team identifier in form of FGenericTeamId */
    virtual FGenericTeamId GetGenericTeamId() const override;

    /** 击中敌方目标时触发 */
    virtual void HitEnemy(const FDamageTargetInfo& DamageInfo, ADCharacterBase* HitTarget) {}

    /** 当血量为0时触发 */
    virtual void OnDeath(const AActor* Causer);

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
};
