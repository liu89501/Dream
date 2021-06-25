// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "ShootWeapon.h"
#include "Character/DCharacterBase.h"
#include "Perception/AIPerceptionTypes.h"
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
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	class UBehaviorTree* BehaviorTree;
	
	/**
	 * 被击杀时的奖励
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	FReward Reward;

	UPROPERTY(EditAnywhere, Category = "AISettings|Abilities")
	UDataTable* DefaultAttributes;

	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	float HealthUIShowSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	TArray<TSubclassOf<class UGameplayAbility>> OwningAbilities;

	/**
	 * 	AI最大的巡逻范围, 相对于 SpawnLocation
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	float MaxPatrolRange;
	
	UPROPERTY(EditDefaultsOnly, Category = AISettings)
	FName BlackboardName_HostileTarget;

	/**
	* 触发Team感知事件的直径
	*/
	UPROPERTY(EditDefaultsOnly, Category = AISettings)
	float NotifyTeamDiameter;

	UPROPERTY(BlueprintReadOnly, Category = AISettings)
	float WalkSpeed;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = AISettings)
	float JogSpeed;

public:

	virtual void ShowHealthUI();

	class AAIController* GetAIController() const;

	void SetAIGenerator(class ADAIGenerator* Generator);

protected:

    //void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void HiddenHealthUI();

	virtual void BeginPlay() override;

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;
	
	virtual UAIPerceptionComponent* GetPerceptionComponent() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual FDamageResult CalculationDamage(float Damage, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ActivateHostile(ADCharacterBase* Hostile, bool bTriggerTeamStimulus = true);

	virtual void RefreshActiveHostile();

	virtual void LostAllHostileTarget();

	virtual void OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, struct FAIStimulus Stimulus);

	void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

protected:

	UPROPERTY()
	TMap<AActor*, float> HostileDamageCount;

	UPROPERTY()
	ADAIGenerator* OwnerAIGenerator;

	UPROPERTY()
	class AAIController* AIController;

private:

	FTimerHandle Handle_ShowUI;

	FDelegateHandle HostileDeathHandle;

	UFUNCTION()
    void OnTargetPerceptionUpdated0(AActor* Actor, FAIStimulus Stimulus);
	
};
