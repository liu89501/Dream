// Fill out your copyright notice in the Description page of Project Settings.


#include "DEnemyBase.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "DAIGenerator.h"
#include "DreamAttributeSet.h"
#include "GameFramework/GameModeBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "PlayerDataStore.h"
#include "Perception/AISense_Damage.h"
#include "DreamGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "GameplayTasksComponent.h"
#include "HealthWidgetComponent.h"
#include "Character/DPlayerController.h"
#include "Character/DCharacterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DreamDropProps.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ADEnemyBase::ADEnemyBase()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	HealthUI = CreateDefaultSubobject<UHealthWidgetComponent>(TEXT("HealthUI"));
	HealthUI->SetupAttachment(RootComponent);

	TasksComponent = CreateDefaultSubobject<UGameplayTasksComponent>(TEXT("TasksComponent"));

	PrimaryActorTick.bCanEverTick = false;

	RunSpeed = 600.f;
	
	//PrimaryActorTick.bStartWithTickEnabled = false;
	//PrimaryActorTick.TickInterval = .4f;
}

void ADEnemyBase::HealthChanged(const FOnAttributeChangeData& AttrData)
{
	Super::HealthChanged(AttrData);
	
	if (IsRunningDedicatedServer())
	{
		return;
	}
	
	HealthUI->UpdateStatus(GetHealthPercent(), GetShieldPercent());

	if (GetHealth() == 0)
	{
		if (TSubclassOf<ADreamDropProps> AmmunitionClass = FRandomProbability::RandomProbability(Reward.AmmunitionReward))
		{
			GetWorld()->SpawnActor<ADreamDropProps>(AmmunitionClass, FTransform(GetActorLocation()));
		}
	}
}

void ADEnemyBase::ShowHealthUI()
{
	if (UUserWidget* Widget = HealthUI->GetUserWidgetObject())
	{
		Widget->SetRenderOpacity(1.f);
		GetWorldTimerManager().SetTimer(Handle_ShowUI, this, &ADEnemyBase::HiddenHealthUI, HealthUIShowSecond);
	}
}

AAIController* ADEnemyBase::GetAIController() const
{
	return Cast<AAIController>(Controller);
}

void ADEnemyBase::SetAIGenerator(ADAIGenerator* Generator)
{
	OwnerAIGenerator = Generator;
}

void ADEnemyBase::HiddenHealthUI()
{
	if (UUserWidget* Widget = HealthUI->GetUserWidgetObject())
	{
		Widget->SetRenderOpacity(0);
	}
}

void ADEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	if (GetLocalRole() == ROLE_Authority)
	{
		if (DefaultAttributes)
		{
			AttributeSet->InitFromMetaDataTable(DefaultAttributes);
		}

		for (TSubclassOf<UGameplayAbility> OwningAbility : OwningAbilities)
		{
			AbilitySystem->GiveAbility(FGameplayAbilitySpec(OwningAbility));
		}
	}
	
	if (IsRunningDedicatedServer())
	{
		HealthUI->DestroyComponent();
	}
	else
	{
		HiddenHealthUI();
	}
}

void ADEnemyBase::HandleDamage(const float DamageDone, const FGameplayEffectContextHandle& Handle,
        ADCharacterBase* SourceCharacter, const FGameplayTagContainer& AssetTags)
{
	Super::HandleDamage(DamageDone, Handle, SourceCharacter, AssetTags);
	
	if (SourceCharacter == nullptr || SourceCharacter->IsA<ADEnemyBase>())
	{
		return;
	}

	float& DamageCount = HostileDamageCount.FindOrAdd(SourceCharacter);
	DamageCount += DamageDone;

	UAISense_Damage::ReportDamageEvent(GetWorld(), this, SourceCharacter, DamageDone, SourceCharacter->GetActorLocation(), GetActorLocation());
}

void ADEnemyBase::OnDeath(const AActor* Causer)
{
	Super::OnDeath(Causer);

	if (OwnerAIGenerator)
	{
		OwnerAIGenerator->AIDeathCount();
	}

	AIPerception->OnOwnerEndPlay(this, EEndPlayReason::Destroyed);
	
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

	TArray<FPlayerWeaponAdd> Weapons;

	for (TActorIterator<ADPlayerController> It(GetWorld()); It; ++It)
	{
		TSubclassOf<AShootWeapon> WeaponClass = FRandomProbability::RandomProbability(Reward.WeaponReward);

		if (*WeaponClass == nullptr)
		{
			continue;
		}

		const FUniqueNetIdRepl& UniqueId = It->PlayerState->GetUniqueId();
		if (!UniqueId.IsValid())
		{
			DREAM_NLOG(Error, TEXT("ADEnemyBase::OnDeath UniqueId Invalid"));
			continue;
		}

		FPlayerWeaponAdd Weapon;
		Weapon.WeaponClass = WeaponClass->GetPathName();
		It->ClientReceiveRewardMessage(WeaponClass);
		Weapons.Add(Weapon);

	}
#if !WITH_EDITOR
	if (Weapons.Num() > 0)
	{
		FPlayerDataStore* PDS = FPlayerDataStoreModule::Get();
		PDS->AddWeapons(Weapons);
	}
#endif

	SetLifeSpan(3.f);
}

UAIPerceptionComponent* ADEnemyBase::GetPerceptionComponent()
{
	return AIPerception;
}

FDamageResult ADEnemyBase::CalculationDamage(float Damage, AActor* DamageCauser)
{
	FDamageResult DamageResult;

	if (ADCharacterPlayer* Player = Cast<ADCharacterPlayer>(DamageCauser))
	{
		//Damage *= 1000.f / FMath::Max(Defense - Player->Penetration, 1.f);

		if (UKismetMathLibrary::RandomBoolWithWeight(Player->GetCriticalRate()))
		{
			Damage *= FMath::Max(Player->GetCriticalDamage(), 1.f);
			DamageResult.bCritical = true;
		}
	}

	DamageResult.Damage = Damage;

	return DamageResult;
}

void ADEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
