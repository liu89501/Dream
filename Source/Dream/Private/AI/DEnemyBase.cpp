// Fill out your copyright notice in the Description page of Project Settings.


#include "DEnemyBase.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "DAIGenerator.h"
#include "DreamAttributeSet.h"
#include "Perception/AIPerceptionComponent.h"
#include "PlayerDataInterface.h"
#include "Perception/AISense_Damage.h"
#include "DreamGameInstance.h"
#include "EngineUtils.h"
#include "GameplayTasksComponent.h"
#include "HealthWidgetComponent.h"
#include "Character/DPlayerController.h"
#include "Character/DCharacterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DreamDropProps.h"
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

	TasksComponent = CreateDefaultSubobject<UGameplayTasksComponent>(TEXT("TasksComponent"));
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	JogSpeed = 400.f;
}

void ADEnemyBase::HealthChanged(const FOnAttributeChangeData& AttrData)
{
	Super::HealthChanged(AttrData);

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (GetHealth() == 0)
	{
		TSubclassOf<ADreamDropProps> RandomProbability = FRandomProbability::RandomProbabilityEx<TSubclassOf<ADreamDropProps>>(Reward.AmmunitionReward);
		if (RandomProbability)
		{
			GetWorld()->SpawnActor<ADreamDropProps>(RandomProbability, FTransform(GetActorLocation()));
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

void ADEnemyBase::SetAIGenerator(ADAIGenerator* Generator)
{
	OwnerAIGenerator = Generator;
}

void ADEnemyBase::HiddenHealthUI()
{
	HealthUI->SetVisibility(false);
}

void ADEnemyBase::UpdateHealthUI()
{
	HealthUI->UpdateStatus(GetHealthPercent(), GetShieldPercent());
}

void ADEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	AIController = Cast<AAIController>(Controller);

	if (GetLocalRole() == ROLE_Authority)
	{
		AIController->RunBehaviorTree(BehaviorTree);
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
		TasksComponent->Deactivate();
		if (UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(AICtrl->BrainComponent))
		{
			BehaviorTreeComponent->StopTree(EBTStopMode::Forced);
		}
	}

	TArray<UItemData*> FinalRewards;

	TMap<ADPlayerController*, TArray<UItemData*>> AllPlayerRewards;

	for (TActorIterator<ADPlayerController> It(GetWorld()); It; ++It)
	{
		for (FRewardProbabilityList& RewardProbabilities : Reward.Rewards)
		{
			TArray<UItemData*> RewardsTemplate;
			TArray<float> Probability;
		
			for (FRewardProbability RewardProbability : RewardProbabilities.RewardList)
			{
				RewardsTemplate.Add(RewardProbability.Reward);
				Probability.Add(RewardProbability.Probability);
			}

			int32 Index = FRandomProbability::RandomProbability(Probability);
			if (Index > INDEX_NONE)
			{
				FinalRewards.Add(RewardsTemplate[Index]);
				AllPlayerRewards.FindOrAdd(*It).Add(RewardsTemplate[Index]);
			}
		}
	}

	FCommonCompleteNotify Delegate;
	Delegate.BindUObject(this, &ADEnemyBase::OnRewardsAddCompleted, AllPlayerRewards);
	FPlayerDataInterfaceStatic::Get()->AddPlayerRewards(FinalRewards, Delegate);

	SetLifeSpan(3.f);
}

void ADEnemyBase::OnRewardsAddCompleted(const FString& ErrorMessage, TMap<ADPlayerController*, TArray<UItemData*>> Rewards)
{
	if (ErrorMessage.IsEmpty())
	{
		for (TPair<ADPlayerController*, TArray<UItemData*>> PlayerReward : Rewards)
		{
			TArray<FRewardMessage> RewardMessages;
			for (UItemData* RewardItem : PlayerReward.Value)
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
	DOREPLIFETIME(ADEnemyBase, CtrlYaw);
}

void ADEnemyBase::ActivateHostile(ADCharacterBase* Hostile, bool bTriggerTeamStimulus)
{
	AIController->SetFocus(Hostile);

	if (bTriggerTeamStimulus)
	{
		UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld());
		FAITeamStimulusEvent TeamStimulusEvent(this, Hostile, GetActorLocation(), NotifyTeamDiameter);
		PerceptionSystem->OnEvent(TeamStimulusEvent);
	}

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	Blackboard->SetValueAsObject(BlackboardName_HostileTarget, Hostile);

	if (!Hostile->OnCharacterDeath.IsBoundToObject(this))
	{
		Hostile->OnCharacterDeath.AddUObject(this, &ADEnemyBase::HostileTargetDestroy);
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
	GetCharacterMovement()->MaxWalkSpeed = GetClass()->GetDefaultObject<ADEnemyBase>()->GetCharacterMovement()->
	                                                   MaxWalkSpeed;
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
			AActor* FocusEnemy = AIController->GetFocusActorForPriority(EAIFocusPriority::Gameplay);

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

	FRotator ControlRotation = GetControlRotation();
	CtrlYaw = ControlRotation.Yaw;

	FRotator ActorRotation = GetActorRotation();
	float YawDelta = FMath::Abs(FRotator::NormalizeAxis(CtrlYaw - ActorRotation.Yaw));

	if (FMath::IsNearlyZero(GetVelocity().Size2D()))
	{
		if (YawDelta >= 60.f || bTurnInProgress)
		{
			SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, CtrlYaw, 0), DeltaSeconds, 10.f));

			if (FMath::IsNearlyZero(YawDelta, 1.f))
			{
				bTurnInProgress = false;
			}
			else
			{
				bTurnInProgress = true;
			}
		}
	}
	else
	{
		if (YawDelta > 40.f)
		{
			SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, CtrlYaw, 0), DeltaSeconds, 10.f));
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
		if (StimulusPawn->GetHealth() == 0)
		{
			// 死亡的敌人不用走后续逻辑
			return;
		}

		OnTargetPerceptionUpdated(StimulusPawn, Stimulus);
	}
}
