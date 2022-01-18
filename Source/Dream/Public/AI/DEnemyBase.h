// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootWeapon.h"
#include "Character/DCharacterBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "DEnemyBase.generated.h"

USTRUCT()
struct FAmmunitionDropProbability
{
	GENERATED_BODY()

	FAmmunitionDropProbability()
		: Ammunition_L1(0.5f)
		, Ammunition_L2(0.3f)
		, Ammunition_L3(0.2f)
	{
	}

	/**
	 * L1 弹药的掉落几率
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L1;

	/**
	* L2 弹药的掉落几率
	*/
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L2;

	/**
	* L3 弹药的掉落几率
	*/
	UPROPERTY(EditAnywhere, Meta = (ClampMin=0, ClampMax=1))
	float Ammunition_L3;

	bool RandomDrawing(EAmmoType& Type);
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
	class UHealthWidgetComponent* HealthUI;
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	class UBehaviorTree* BehaviorTree;
	
	/**
	 * 弹药掉落
	 */
	UPROPERTY(EditAnywhere, Category = AISettings)
	FAmmunitionDropProbability AmmunitionDrop;

	UPROPERTY(EditAnywhere, Category = "AISettings|Abilities")
	UDataTable* DefaultAttributes;

	UPROPERTY(BlueprintReadOnly)
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	float HealthUIShowSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	TArray<TSubclassOf<class UGameplayAbility>> OwningAbilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AISettings)
	class UDRewardPool* RewardPool;

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

	class AAIController* GetAIController() const;

	void SetAIGenerator(class ADAIGeneratorBase* Generator);

	UFUNCTION(BlueprintPure, Category=AIEnemy)
	FRotator GetReplicationControllerRotation() const;

protected:

    //void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

	virtual void HealthChanged(const FOnAttributeChangeData& AttrData) override;

	virtual void HiddenHealthUI();
	virtual void UpdateHealthUI();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle) override;
	
	virtual UAIPerceptionComponent* GetPerceptionComponent() override;

	virtual void OnDeath(const AActor* Causer) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ActivateHostile(ADCharacterBase* Hostile, bool bTriggerTeamStimulus = true);

	virtual void RefreshActiveHostile();

	virtual void LostAllHostileTarget();

	virtual void OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, struct FAIStimulus Stimulus);

	virtual void Tick(float DeltaSeconds) override;

	void HostileTargetDestroy(ADCharacterBase* DestroyedActor);

protected:

	UPROPERTY()
	ADAIGeneratorBase* OwnerAIGenerator;

	UPROPERTY()
	class AAIController* AIController;

	UPROPERTY(Replicated)
	FVector_NetQuantize10 ReplicatedCtrlRotation;

	UPROPERTY(BlueprintReadOnly)
	bool bTurnInProgress;

private:

	FTimerHandle Handle_ShowUI;

	FDelegateHandle HostileDeathHandle;

	UFUNCTION()
    void OnTargetPerceptionUpdated0(AActor* Actor, FAIStimulus Stimulus);
};
