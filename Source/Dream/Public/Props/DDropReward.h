// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDropSkeletalMesh.h"
#include "PlayerDataInterfaceType.h"
#include "DDropReward.generated.h"

UCLASS(Abstract)
class DREAM_API ADDropReward : public ADDropSkeletalMesh
{
	GENERATED_BODY()

public:
	
	ADDropReward();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DropReward)
	USoundCue* PickupSound;

public:

	UFUNCTION(BlueprintCallable, Category=DropReward)
	const FItemDef& GetItemDefinition() const;

	UFUNCTION(BlueprintCallable, Category=DropReward)
	const FLinearColor& GetItemQualityColor() const;

	UFUNCTION(Server, Reliable)
	void ServerPickupReward();

public:

	void Initialize(TSharedPtr<FItem> InItem);

protected:

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnPickup(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	              int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	UPROPERTY(Replicated)
	int32 ItemGuid;

	// Server Only
	TSharedPtr<FItem> Item;

private:

	/**
	 * 所属玩家的PlayerID 之所要存一下这个是因为如果这个奖励玩家离开地图都没有拾取时， 这时就需要服务器自动帮他拾取，
	 * 所以这里必须要存一个ID(因为玩家这时已经与服务器断开了连接)
	 */
	int32 OwnerPlayerId;
	
};

