#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "MiniMapDataInterface.h"
#include "WeaknessInterface.h"
#include "GameplayEffectTypes.h"

#include "DCharacterBase.generated.h"

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
                                public IAbilitySystemInterface, public IMiniMapDataInterface,
                                public IWeaknessInterface
{
    GENERATED_BODY()

public:

    ADCharacterBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UMiniMapDataComponent* MiniMapData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shooter)
    FGenericTeamId TeamID;

    UPROPERTY(BlueprintReadWrite, Replicated, Category = Shooter)
    EPawnStatus PawnStatus;

    // 死亡动画
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shooter)
    UAnimMontage* PawnDeathMontage;

    UPROPERTY(BlueprintReadOnly, Category = Shooter)
    class UDreamAttributeSet* AttributeSet;

    UPROPERTY(EditAnywhere, Category = Shooter)
    TMap<FName, float> WeakPoint;

    
    
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetHealth() const;
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetMaxHealth() const;
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetShield() const;
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetMaxShield() const;
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetCriticalRate() const;
    UFUNCTION(BlueprintCallable, Category="Shooter|Attributes")
    float GetCriticalDamage() const;


    FCharacterDeathSignature OnCharacterDeath;

    virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle,
        ADCharacterBase* SourceCharacter, const FGameplayTagContainer& AssetTags);

    UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHandleDamage", ScriptName = "OnHandleDamage"))
    void BP_HandleDamage(float Damage, const FHitResult& HitResult, ADCharacterBase* SourceCharacter);

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

    virtual void HealthChanged(const FOnAttributeChangeData& AttrData);

    virtual void BeginPlay() override;

    /** Assigns Team Agent to given TeamID */
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

    virtual UMiniMapDataComponent* GetMiniMapDataComponent() const override;

    virtual float GetWeaknessIncreaseDamagePercentage(const FName& BoneName) override;

    /** Retrieve team identifier in form of FGenericTeamId */
    virtual FGenericTeamId GetGenericTeamId() const override;

    /** 击中敌方目标时触发 */
    virtual void HitEnemy(const FDamageTargetInfo& DamageInfo) {};

    /** 当血量为0时触发 */
    virtual void OnDeath(const AActor* Causer);

    virtual FDamageResult CalculationDamage(float Damage, AActor* DamageCauser);

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                             AActor* DamageCauser) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = Shooter)
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = Shooter)
    float GetShieldPercent() const;

    UFUNCTION(BlueprintImplementableEvent, Category = Shooter, meta = (DisplayName = "OnHealthChanged", ScriptName = "OnHealthChanged"))
    void BP_OnHealthChanged();
};
