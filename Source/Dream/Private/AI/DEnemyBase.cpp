// Fill out your copyright notice in the Description page of Project Settings.


#include "DEnemyBase.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "DAIGeneratorBase.h"
#include "DreamAttributeSet.h"
#include "Perception/AIPerceptionComponent.h"
#include "PlayerDataInterface.h"
#include "Perception/AISense_Damage.h"
#include "DreamGameInstance.h"
#include "HealthWidgetComponent.h"
#include "Character/DPlayerController.h"
#include "Character/DCharacterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DreamDropProps.h"
#include "DRewardPool.h"
#include "PlayerDataInterfaceStatic.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Team.h"

ADEnemyBase::ADEnemyBase()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	HealthUI = CreateDefaultSubobject<UHealthWidgetComponent>(TEXT("HealthUI"));
	HealthUI->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	JogSpeed = 400.f;

	//DREAM_NLOG(Error, TEXT("ADEnemyBase AttributeSet: %s"), *(AttributeSet ? FCoreTexts::Get().Yes : FCoreTexts::Get().No).ToString());
}

void ADEnemyBase::HealthChanged(const FOnAttributeChangeData& AttrData)
{
	Super::HealthChanged(AttrData);

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (IsDeath())
	{
		// todo 放在这可能不太好
		if (TSubclassOf<ADreamDropProps> DropMagazine = FRandomProbability::RandomProbabilityEx<TSubclassOf<ADreamDropProps>>(AmmunitionReward))
		{
			GetWorld()->SpawnActor<ADreamDropProps>(DropMagazine, FTransform(GetActorLocation()));
		}

		HealthUI->DestroyComponent();
	}
	else
	{
		HealthUI->SetVisibility(true);
		GetWorldTimerManager().SetTimerForNextTick(this, &ADEnemyBase::UpdateHealthUI);
		GetWorldTimerManager().SetTimer(Handle_ShowUI, this, &ADEnemyBase::HiddenHealthUI, HealthUIShowSecond);
	}
}

AAIController* ADEnemyBase::GetAIController() const
{
	return AIController;
}

void ADEnemyBase::SetAIGenerator(ADAIGeneratorBase* Generator)
{
	OwnerAIGenerator = Generator;
}

FRotator ADEnemyBase::GetReplicationControllerRotation() const
{
	return FRotator(ReplicatedCtrlRotation.X, ReplicatedCtrlRotation.Y, 0);
}

void ADEnemyBase::HiddenHealthUI()
{
	HealthUI->SetVisibility(false);
}

void ADEnemyBase::UpdateHealthUI()
{
	HealthUI->UpdateStatus(GetHealthPercent());
}

void ADEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	AIController = Cast<AAIController>(Controller);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (BehaviorTree)
		{
			AIController->RunBehaviorTree(BehaviorTree);
		}

		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ADEnemyBase::OnTargetPerceptionUpdated0);

		if (DefaultAttributes)
		{
			AttributeSet->InitFromMetaDataTable(DefaultAttributes);
		}

		for (TSubclassOf<UGameplayAbility> OwningAbility : OwningAbilities)
		{
			AbilitySystem->GiveAbility(FGameplayAbilitySpec(OwningAbility));
		}

		SetActorTickEnabled(true);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		HiddenHealthUI();
	}
	else
	{
		HealthUI->DestroyComponent();
	}
}

void ADEnemyBase::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
	Super::HandleDamage(DamageDone, Handle);

	AActor* OriginalInstigator = Handle.GetOriginalInstigator();

	if (OriginalInstigator == nullptr || OriginalInstigator->IsA<ADEnemyBase>())
	{
		return;
	}

	UAISense_Damage::ReportDamageEvent(GetWorld(), this, OriginalInstigator, DamageDone,
	                                   OriginalInstigator->GetActorLocation(), GetActorLocation());
}

void ADEnemyBase::OnDeath(const AActor* Causer)
{
	Super::OnDeath(Causer);

	if (OwnerAIGenerator)
	{
		OwnerAIGenerator->AIDeathCount();
	}

	if (AAIController* AICtrl = Cast<AAIController>(Controller))
	{
		AICtrl->StopMovement();
		AICtrl->ClearFocus(EAIFocusPriority::Gameplay);
		if (UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(AICtrl->BrainComponent))
		{
			BehaviorTreeComponent->StopTree(EBTStopMode::Forced);
		}
	}

	TArray<AActor*> HostileActors;
	AIPerception->GetHostileActors(HostileActors);

	if (!FPDIStatic::IsLocalInterface())
	{
		UItemDataContainer* DataContainer = NewObject<UItemDataContainer>();
		TMap<ADPlayerController*, UItemData*> AllPlayerRewards;

		for (AActor* Hostile : HostileActors)
		{
			ADPlayerController* PlayerController = nullptr;
			
			if (ADCharacterPlayer* HostileCharacter = Cast<ADCharacterPlayer>(Hostile))
			{
				PlayerController = HostileCharacter->GetPlayerController();
			}

			if (PlayerController == nullptr)
			{
				continue;
			}

			UItemData* Rewards = RewardPool->GenerateRewards(PlayerController->GetPlayerState<ADPlayerState>());
			if (Rewards->IsValidData())
			{
				AllPlayerRewards.Add(PlayerController, Rewards);
				DataContainer->AddItem(Rewards);
			}
		}

		if (DataContainer->IsValidData())
		{
			Handle_Rewards = FPDIStatic::Get()->AddOnAddPlayerRewards(
				FOnCompleted::FDelegate::CreateUObject(this, &ADEnemyBase::OnRewardsAddCompleted, AllPlayerRewards));
			
			FPDIStatic::Get()->AddPlayerRewards(FItemDataHandle(DataContainer));
		}
	}
	else
	{
		for (AActor* Hostile : HostileActors)
		{
			if (ADCharacterPlayer* HostileCharacter = Cast<ADCharacterPlayer>(Hostile))
			{
				if (ADPlayerController* PlayerController = HostileCharacter->GetPlayerController())
				{
					PlayerController->ClientHandleKilledRewardsGenerate(GetClass());
				}
			}
		}
	}

	SetLifeSpan(3.f);
}

void ADEnemyBase::OnRewardsAddCompleted(bool bSuccess, TMap<ADPlayerController*, UItemData*> Rewards) const
{
	FPDIStatic::Get()->RemoveOnAddPlayerRewards(Handle_Rewards);
	
	if (bSuccess)
	{
		for (TPair<ADPlayerController*, UItemData*> PlayerReward : Rewards)
		{
			TArray<FRewardMessage> RewardMessages;
			for (UItemData* RewardItem : FItemDataRange(PlayerReward.Value))
			{
				FRewardMessage RewardMessage;
				RewardMessage.RewardPropsClass = RewardItem->GetItemClass();
				RewardMessage.RewardNum = RewardItem->GetItemAmount();
				
				RewardMessages.Add(RewardMessage);
			}
			
			PlayerReward.Key->ClientReceiveRewardMessage(RewardMessages);
		}
	}
}

UAIPerceptionComponent* ADEnemyBase::GetPerceptionComponent()
{
	return AIPerception;
}

void ADEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADEnemyBase, ReplicatedCtrlRotation);
}

void ADEnemyBase::ActivateHostile(ADCharacterBase* Hostile, bool bTriggerTeamStimulus)
{
	if (!bUseControllerRotationYaw)
	{
		AIController->SetFocus(Hostile);
	}

	if (bTriggerTeamStimulus)
	{
		UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld());
		FAITeamStimulusEvent TeamStimulusEvent(this, Hostile, GetActorLocation(), NotifyTeamDiameter);
		PerceptionSystem->OnEvent(TeamStimulusEvent);
	}

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	Blackboard->SetValueAsObject(BlackboardName_HostileTarget, Hostile);

	FCharacterDeathSignature& Delegate = Hostile->GetCharacterDeathDelegate();
	if (!Delegate.IsBoundToObject(this))
	{
		Delegate.AddUObject(this, &ADEnemyBase::HostileTargetDestroy);
	}
}

void ADEnemyBase::RefreshActiveHostile()
{
	TArray<AActor*> HostileActors;
	AIPerception->GetHostileActors(HostileActors);

	if (HostileActors.Num() > 0)
	{
		if (ADCharacterBase* NewHostile = Cast<ADCharacterBase>(HostileActors[0]))
		{
			ActivateHostile(NewHostile);
		}
	}
	else
	{
		LostAllHostileTarget();
	}
}

void ADEnemyBase::LostAllHostileTarget()
{
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	AIController->GetBlackboardComponent()->ClearValue(BlackboardName_HostileTarget);
	GetCharacterMovement()->MaxWalkSpeed = GetClass()->GetDefaultObject<ADEnemyBase>()->GetCharacterMovement()->MaxWalkSpeed;
}

void ADEnemyBase::OnTargetPerceptionUpdated(ADCharacterBase* StimulusPawn, FAIStimulus Stimulus)
{
	TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);

	if (SenseClass->IsChildOf<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActivateHostile(StimulusPawn);
		}
		else
		{
			// UAISense_Damage 感知超时的时候会走这段代码
			RefreshActiveHostile();
		}
	}
	else if (SenseClass->IsChildOf<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActivateHostile(StimulusPawn);
		}
	}
	else if (SenseClass->IsChildOf<UAISense_Team>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			AActor* FocusEnemy = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardName_HostileTarget));

			if (FocusEnemy == nullptr || FocusEnemy->IsPendingKill())
			{
				ActivateHostile(StimulusPawn, false);
			}
		}
	}
}

void ADEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FRotator Rotation = GetControlRotation();
	ReplicatedCtrlRotation.X = Rotation.Pitch;
	ReplicatedCtrlRotation.Y = Rotation.Yaw;
	ReplicatedCtrlRotation.Z = 0;

	if (!bUseControllerRotationYaw)
	{
		FRotator ActorRotation = GetActorRotation();
		FRotator DeltaRotation = Rotation - ActorRotation;
		DeltaRotation.Normalize();

		float YawAbs = FMath::Abs(DeltaRotation.Yaw);

		if (YawAbs >= 60.f || bTurnInProgress)
		{
			SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, Rotation.Yaw, 0), DeltaSeconds, 10.f));

			if (FMath::IsNearlyZero(YawAbs, 2.f))
			{
				bTurnInProgress = false;
			}
			else
			{
				bTurnInProgress = true;
			}
		}
	}
}

void ADEnemyBase::HostileTargetDestroy(ADCharacterBase* DestroyedActor)
{
	AIPerception->ForgetActor(DestroyedActor);
	RefreshActiveHostile();
}

void ADEnemyBase::OnTargetPerceptionUpdated0(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor == nullptr)
	{
		DREAM_NLOG(Warning, TEXT("OnTargetPerceptionUpdated0 -> Actor Invalid"));
		return;
	}

	if (ADCharacterBase* StimulusPawn = Cast<ADCharacterBase>(Actor->GetInstigator()))
	{
		if (StimulusPawn->IsDeath())
		{
			// 死亡的敌人不用走后续逻辑
			return;
		}

		OnTargetPerceptionUpdated(StimulusPawn, Stimulus);
	}
}
