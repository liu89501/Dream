// Fill out your copyright notice in the Description page of Project Settings.

#include "DEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "DAIGeneratorBase.h"
#include "DMProjectSettings.h"
#include "DMAttributeSet.h"
#include "Perception/AIPerceptionComponent.h"
#include "PlayerDataInterface.h"
#include "Perception/AISense_Damage.h"
#include "HealthWidgetComponent.h"
#include "Character/DMPlayerController.h"
#include "Character/DCharacterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PDIFunctions.h"
#include "PlayerDataInterfaceStatic.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerState.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Team.h"

bool FAmmunitionDropProbability::RandomDrawing(EAmmoType& Type)
{
	float Rand = FMath::FRand();

	if (Rand <= Ammunition_L1)
	{
		Type = EAmmoType::Level1;
		return true;
	}

	float L2Range = Ammunition_L1 + Ammunition_L2;
	if (Rand > Ammunition_L1 && Rand <= L2Range)
	{
		Type =  EAmmoType::Level2;
		return true;
	}

	if (Rand > L2Range && Rand <= (L2Range + Ammunition_L3))
	{
		Type =  EAmmoType::Level3;
		return true;
	}

	return false;
}

ADEnemyBase::ADEnemyBase()
	: bAutoActivateBehaviorTree(false)
	, JogSpeed(400.f)
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	AIPerception->bAutoActivate = false;

	HealthUI = CreateDefaultSubobject<UHealthWidgetComponent>(TEXT("HealthUI"));
	HealthUI->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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
		EAmmoType RandomDropAmmoType;
		if (AmmunitionDrop.RandomDrawing(RandomDropAmmoType))
		{
			TSubclassOf<ADDropMagazine> MagazineDropClass = UDMProjectSettings::GetProjectSettings()->GetMagazineDropClass(RandomDropAmmoType);

			if (MagazineDropClass)
			{
				GetWorld()->SpawnActor<ADDropMagazine>(MagazineDropClass, FTransform(GetActorLocation()));
			}
		}
	}
	
	HealthUI->SetVisibility(true);
	GetWorldTimerManager().SetTimerForNextTick(this, &ADEnemyBase::UpdateHealthUI);
	GetWorldTimerManager().SetTimer(Handle_ShowUI, this, &ADEnemyBase::HiddenHealthUI, HealthUIShowSecond);
}

AAIController* ADEnemyBase::GetAIController() const
{
	return AIController;
}

void ADEnemyBase::SetAIGenerator(ADAIGeneratorBase* Generator)
{
	OwnerAIGenerator = Generator;
}

void ADEnemyBase::ActivateBehaviorTree()
{
	if (BehaviorTree)
	{
		AIController->RunBehaviorTree(BehaviorTree);
		AIPerception->Activate();
	}
}

FRotator ADEnemyBase::GetReplicationControllerRotation() const
{
	return ReplicatedCtrlRotation.Rotation();
}

void ADEnemyBase::HiddenHealthUI()
{
	if (HealthUI && !IsPendingKillPending())
	{
		HealthUI->SetVisibility(false);
	}
}

void ADEnemyBase::UpdateHealthUI()
{
	if (HealthUI)
	{
		HealthUI->UpdateStatus(GetHealthPercent());
	}
}

void ADEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterAttributes = const_cast<UDMAttributeSet*>(AbilitySystem->GetSet<UDMAttributeSet>());

	SpawnLocation = GetActorLocation();

	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	AIController = Cast<AAIController>(Controller);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (bAutoActivateBehaviorTree)
		{
			ActivateBehaviorTree();
		}

		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ADEnemyBase::OnTargetPerceptionUpdated);

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

void ADEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (Handle_ShowUI.IsValid())
	{
		GetWorldTimerManager().ClearTimer(Handle_ShowUI);
	}
}

void ADEnemyBase::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle)
{
	Super::HandleDamage(DamageDone, Handle);

	AActor* OriginalInstigator = Handle.GetOriginalInstigator();

	if (FGenericTeamId::GetAttitude(OriginalInstigator, this) != ETeamAttitude::Hostile)
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

	for (AActor* Hostile : HostileActors)
	{
		ADMPlayerController* PlayerController = nullptr;

		if (ADCharacterPlayer* HostileCharacter = Cast<ADCharacterPlayer>(Hostile))
		{
			PlayerController = HostileCharacter->GetPlayerController();
		}

		if (PlayerController == nullptr)
		{
			continue;
		}

		APlayerState* HostilePlayerState = PlayerController->PlayerState;
		if (HostilePlayerState == nullptr)
		{
			UE_LOG(LogDream, Error, TEXT("PlayerState Invalid"));
			continue;
		}

		UPDIFunctions::SpawnRewardsAtLocation(PlayerController, RewardPool, GetActorLocation());
	}

	if (const ADCharacterPlayer* Player = Cast<ADCharacterPlayer>(Causer))
	{
		if (ADMPlayerController* Ctrl = Player->GetPlayerController())
		{
			int32 PlayerId = Ctrl->PlayerState->GetPlayerId();
			GDataInterface->UpdateTaskState(FQuestActionHandle(PlayerId, MakeShared<FQuestAction_KilledTarget>(PawnType)));
		}
	}

	SetLifeSpan(5.f);
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

void ADEnemyBase::ActivateHostile(AActor* Hostile, bool bTriggerTeamStimulus)
{
	if (!bUseControllerRotationYaw)
	{
		AIController->SetFocus(Hostile);
	}

	FocusHostile = Hostile;

	if (bTriggerTeamStimulus)
	{
		UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld());
		FAITeamStimulusEvent TeamStimulusEvent(this, Hostile, GetActorLocation(), NotifyTeamDiameter);
		PerceptionSystem->OnEvent(TeamStimulusEvent);
	}

	if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
	{
		Blackboard->SetValueAsObject(BlackboardName_HostileTarget, Hostile);
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

void ADEnemyBase::OnTargetPerceptionUpdated(AActor* StimulusActor, FAIStimulus Stimulus)
{
	if (StimulusActor == nullptr)
	{
		DREAM_NLOG(Warning, TEXT("OnTargetPerceptionUpdated -> Actor Invalid"));
		return;
	}

	if (StimulusActor->ActorHasTag(DMActorTagName::Death))
	{
		return;
	}
	
	TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);

	if (SenseClass->IsChildOf<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ActivateHostile(StimulusActor);
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
			ActivateHostile(StimulusActor);
		}
	}
	else if (SenseClass->IsChildOf<UAISense_Team>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
			{
				AActor* FocusEnemy = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardName_HostileTarget));

				if (FocusEnemy == nullptr || FocusEnemy->IsPendingKill())
				{
					ActivateHostile(StimulusActor, false);
				}
			}
		}
	}
}

void ADEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// [Server]
	FRotator ControllerRotation = GetControlRotation();
	ReplicatedCtrlRotation = ControllerRotation.Vector().GetSafeNormal();
	//UE_LOG(LogDream, Error, TEXT("PreReplication: %s"), *ControllerRotation.ToString());

	if (!bUseControllerRotationYaw)
	{
		FRotator CtrlRotation = GetControlRotation();
		FRotator ActorRotation = GetActorRotation();
		FRotator DeltaRotation = CtrlRotation - ActorRotation;
		DeltaRotation.Normalize();

		float YawAbs = FMath::Abs(DeltaRotation.Yaw);

		if (YawAbs >= 60.f || bTurnInProgress)
		{
			SetActorRotation(FMath::RInterpTo(ActorRotation, FRotator(0, CtrlRotation.Yaw, 0), DeltaSeconds, 10.f));

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

	if (GetLocalRole() == ROLE_Authority)
	{
		if (FocusHostile && (FocusHostile->IsPendingKillPending() || FocusHostile->ActorHasTag(DMActorTagName::Death)))
		{
			AIPerception->ForgetActor(FocusHostile);
			RefreshActiveHostile();
		}
	}
}

