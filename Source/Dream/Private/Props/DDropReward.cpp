// Fill out your copyright notice in the Description page of Project Settings.

#include "DDropReward.h"
#include "DCharacterPlayer.h"
#include "DGameplayStatics.h"
#include "DMProjectSettings.h"
#include "PlayerDataInterface.h"
#include "PlayerDataInterfaceStatic.h"
#include "UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"
#include "Sound/SoundCue.h"

// Sets default values
ADDropReward::ADDropReward()
	: ItemGuid(0)
	, OwnerPlayerId(0)
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ADDropReward::OnPickup);
	
	bReplicates = true;

	Mesh->SetSimulatePhysics(false);

	// 奖励掉落物应该只与拥有者相关
	bOnlyRelevantToOwner = true;

	Mesh->BodyInstance.bUseCCD = true;
}

const FItemDef& ADDropReward::GetItemDefinition() const
{
	return GSProject->GetItemDefinition(ItemGuid);
}

const FLinearColor& ADDropReward::GetItemQualityColor() const
{
	const EPropsQuality& PropsQuality = GetItemDefinition().ItemBaseInfo.PropsQuality;
	return UDMProjectSettings::GetProjectSettings()->GetQualityInfo(PropsQuality).QualityThemeColor;
}

void ADDropReward::ServerPickupReward_Implementation()
{
	if (Item.IsValid() && OwnerPlayerId > 0)
	{
		FPDIStatic::Get()->AddPlayerRewards(FItemListParam(OwnerPlayerId, Item));
	}

	Destroy();
}

void ADDropReward::Initialize(TSharedPtr<FItem> InItem)
{
	if (InItem.IsValid())
	{
		Item = InItem;
		ItemGuid = InItem->GetItemGuid();
	}
}

void ADDropReward::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() != ROLE_Authority || GetNetMode() == NM_Standalone)
	{
		Mesh->SetSimulatePhysics(true);
		ApplyImpulse(Mesh);
	}

	AActor* OwnerActor = GetOwner();

	if (OwnerActor == nullptr)
	{
		UE_LOG(LogDream, Warning, TEXT("奖励没有所有者"));
		return;
	}

	APlayerController* PlayerOwner = nullptr;

	if (OwnerActor->IsA<APlayerController>())
	{
		PlayerOwner = static_cast<APlayerController*>(OwnerActor);
	}
	else if (OwnerActor->IsA<APawn>())
	{
		PlayerOwner = Cast<APlayerController>(static_cast<APawn*>(OwnerActor)->Controller);
	}

	if (PlayerOwner)
	{
		OwnerPlayerId = PlayerOwner->PlayerState->GetPlayerId();
	}
	else
	{
		UE_LOG(LogDream, Error, TEXT("获取PlayerOwner失败"));
	}
}

void ADDropReward::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetNetMode() == NM_DedicatedServer && GetLocalRole() == ROLE_Authority)
	{
		// 服务器不需要这些操作
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ADDropReward::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADDropReward, ItemGuid, COND_InitialOnly);
}

void ADDropReward::OnPickup(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(OtherActor);

	if (PlayerCharacter == nullptr)
	{
		// 非玩家不能拾取
		return;
	}
	
	// 只对本地玩家有效
	if (PlayerCharacter->IsLocallyControlled())
	{
		if (APlayerState* PlayerState = PlayerCharacter->GetPlayerState())
		{
			// 只能指定的玩家拾取
			if (PlayerState->GetPlayerId() == OwnerPlayerId)
			{
				if (PickupSound)
				{
					// 由于拾取物品的声音效果只需要自己听见, 并且相隔玩家非常近， 所以不需要在世界中播放
					UGameplayStatics::PlaySound2D(this, PickupSound);
				}
			
				ServerPickupReward();
			}
		}
	}
}
