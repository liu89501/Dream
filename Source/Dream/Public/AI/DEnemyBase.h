// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "ShootWeapon.h"
#include "Character/DCharacterBase.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "DEnemyBase.generated.h"

USTRUCT(BlueprintType)
struct FReward
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<TSubclassOf<AShootWeapon>, float> WeaponReward;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<int32, float> WeaponRewardFrequency;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<TSubclassOf<class ADreamDropProps>, float> AmmunitionReward;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<int32, float> AmmunitionRewardFrequency;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FInt32Range MoneyReward;
};

/**
 * 
 */
UCLASS()
class DREAM_API ADEnemyBase : public ADCharacterBase, public IAIPerceptionListenerInterface
{
	GENERATED_BODY()

public:

	ADEnemyBase();

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere)
	class UGameplayTasksComponent* TasksComponent;

	UPROPERTY(VisibleAnywhere)
	class UHealthWidgetComponent* HealthUI;

public:

	/**
	 * 被击杀时的奖励
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AIModule)
	FReward Reward;

	UPROPERTY(EditAnywhere, Category = "Enemy|Abilities")
	UDataTable* DefaultAttributes;

	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AIModule)
	float HealthUIShowSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AIModule)
	TArray<TSubclassOf<class UGameplayAbility>> OwningAbilities;

	/**
	 * 	AI最大的巡逻范围, 相对于 SpawnLocation
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AIModule)
	float MaxPatrolRange;
	
	/**
	 *  发现敌人时的奔跑速度
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AIModule)
	float RunSpeed;

public:

	virtual void ShowHealthUI();

	class AAIController* GetAIController() const;

	void SetAIGenerator(class ADAIGenerator* Generator);

protected:

    //void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void HiddenHealthUI();

	virtual void BeginPlay() override;

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle,
        ADCharacterBase* SourceCharacter, const FGameplayTagContainer& AssetTags) override;
	
	virtual UAIPerceptionComponent* GetPerceptionComponent() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual FDamageResult CalculationDamage(float Damage, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY()
	TMap<AActor*, float> HostileDamageCount;

	UPROPERTY()
	ADAIGenerator* OwnerAIGenerator;

private:

	FTimerHandle Handle_ShowUI;

	FDelegateHandle HostileDeathHandle;
	
};
