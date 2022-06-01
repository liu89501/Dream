// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable CppExpressionWithoutSideEffects
#include "DGameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SubclassOf.h"
#include "GameFramework/Actor.h"
#include "JsonUtilities.h"
#include "AIController.h"
#include "DCharacterPlayer.h"
#include "DGameplayEffectUIData.h"
#include "DGameUserSettings.h"
#include "DMAttributeSet.h"
#include "DMProjectSettings.h"
#include "DreamGameMode.h"
#include "DreamGameplayType.h"
#include "DreamGameplayAbility.h"
#include "DreamLoadingScreen.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "GameFramework/PhysicsVolume.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceSpline.h"
#include "NiagaraFunctionLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "ShootWeapon.h"
#include "Kismet/KismetMathLibrary.h"

template<class GameMode>
GameMode* GetGameMode(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return Cast<GameMode>(World->GetAuthGameMode());
	}
	return nullptr;
}

template<class GameInstance>
GameInstance* GetGameInstance(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetGameInstance<GameInstance>();
	}
	return nullptr;
}

UClass* UDGameplayStatics::LoadUClass(FString QualifiedName)
{
	return LoadClass<UObject>(nullptr, *QualifiedName);
}

FString UDGameplayStatics::GetClassPathName(TSubclassOf<UObject> ObjectClass)
{
	return ObjectClass->GetPathName();
}

const FLinearColor& UDGameplayStatics::GetGameThemeColor()
{
	if (UDGameUserSettings* UserSettings = Cast<UDGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		return UserSettings->GetThemeColor();
	}

	return FLinearColor::White;
}

bool UDGameplayStatics::ContainsActionKey(APlayerController* PlayerController, const FKey& Key, FName ActionName)
{
	bool bResult = false;

	if (PlayerController)
	{
		TArray<FInputActionKeyMapping> ActionKeyMapping = PlayerController->PlayerInput->GetKeysForAction(ActionName);

		for (int32 Index = 0; Index < ActionKeyMapping.Num(); Index++)
		{
			if (ActionKeyMapping[Index].Key == Key)
			{
				bResult = true;
				break;
			}
		}
	}
	return bResult;
}

FName UDGameplayStatics::GetInputActionKeyName(APlayerController* PlayerController, FName ActionName)
{
	if (PlayerController)
	{
		TArray<FInputActionKeyMapping> ActionKeyMapping = PlayerController->PlayerInput->GetKeysForAction(ActionName);
		if (ActionKeyMapping.IsValidIndex(0))
		{
			return ActionKeyMapping[0].Key.GetFName();
		}
	}

	return TEXT("NONE");
}

float UDGameplayStatics::GetDefaultGravityZ(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetDefaultPhysicsVolume()->GetGravityZ();
	}
	return UPhysicsSettings::Get()->DefaultGravityZ;
}

void UDGameplayStatics::AddIconToPlayerMinimap(UObject* WorldContextObject, AActor* Actor)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(World))
		{
			if (ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(PlayerController->GetPawn()))
			{
				PlayerCharacter->AddInfiniteActor(Actor);
			}
		}
	}
}

void UDGameplayStatics::RemoveIconInPlayerMinimap(UObject* WorldContextObject, AActor* Actor)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(World))
		{
			if (ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(PlayerController->GetPawn()))
			{
				PlayerCharacter->RemoveInfiniteActor(Actor);
			}
		}
	}
}

FText UDGameplayStatics::ToTimeText(int32 TotalSeconds)
{
	int32 Minute = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minute, Seconds));
}

FString UDGameplayStatics::GetLocalPlayerName()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	IOnlineIdentityPtr OnlineIdentityPtr = OnlineSubsystem->GetIdentityInterface();

	if (OnlineIdentityPtr.IsValid())
	{
		return OnlineIdentityPtr->GetPlayerNickname(0);
	}

	return TEXT("LocalPlayer");
}

void UDGameplayStatics::CopyMeshComponents(UObject* WorldContextObject, UMeshComponent* Source, UMeshComponent* Target)
{
	const TArray<USceneComponent*>& Components = Target->GetAttachChildren();

	if (Components.Num() > 0)
	{
		for (USceneComponent* Component : Components)
		{
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
			{
				UMeshComponent* NewMesh = NewObject<UMeshComponent>(WorldContextObject, MeshComponent->GetClass(), NAME_None, RF_NoFlags, MeshComponent);
				
				NewMesh->RegisterComponent();
				NewMesh->AttachToComponent(Source, FAttachmentTransformRules::KeepRelativeTransform, MeshComponent->GetAttachSocketName());

				CopyMeshComponents(WorldContextObject, NewMesh, MeshComponent);
			}
		}
	}
}

void UDGameplayStatics::StopMatchmaking(FMatchmakingHandle& Handle)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
	if (SessionInterface->GetNamedSession(PartySessionName) != nullptr)
	{
		SessionInterface->DestroySession(PartySessionName);
	}
	Handle.Clear();
}

USceneComponent* UDGameplayStatics::GetAttachComponentFromSocketName(USceneComponent* ParentComponent,
                                                                     const FName& SocketName)
{
	USceneComponent* ChildComponent = nullptr;

	if (ParentComponent)
	{
		const TArray<USceneComponent*> Components = ParentComponent->GetAttachChildren();

		for (USceneComponent* Component : Components)
		{
			if (Component->GetAttachSocketName() == SocketName)
			{
				ChildComponent = Component;
				break;
			}
		}
	}

	return ChildComponent;
}

void UDGameplayStatics::ApplyModToAttribute(AActor* Source, FGameplayAttribute Attribute, TEnumAsByte<EGameplayModOp::Type> ModifierOp, float ModifierMagnitude)
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Source))
	{
		AbilitySystem->ApplyModToAttribute(Attribute, ModifierOp, ModifierMagnitude);
	}
	else
	{
		DREAM_NLOG(Error, TEXT("AbilitySystem Not Found"));
	}
}

bool UDGameplayStatics::LineTraceAndSendEvent(
	AActor* Source,
	FGameplayTag InEventTag,
	FVector TraceStart,
	FVector TraceEnd,
	ECollisionChannel TraceChannel,
	FHitResult& OutHit)
{
	UWorld* World = Source->GetWorld();
	if (World == nullptr)
	{
		DREAM_NLOG(Error, TEXT("World Invalid"));
		return false;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Source);
	if (AbilitySystem == nullptr)
	{
		DREAM_NLOG(Error, TEXT("AbilitySystemComponent Invalid"));
		return false;
	}

	FCollisionQueryParams QueryParams(TEXT("UDGameplayStatics::LineTraceAndSendEvent"), false, Source);
	bool bHit = World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, TraceChannel, QueryParams);

#if WITH_EDITOR

	//DrawDebugSphere(World, TraceEnd, 100.f, 32, FColor::Red, false, 2, 0, 2);
	
	//AActor* HitActor = OutHit.GetActor();
	//UE_LOG(LogDream, Error, TEXT("LineTraceAndSendEvent: %s, bHit: %d"), HitActor ? *HitActor->GetFullName() : TEXT("NONE"), bHit);

#endif

	if (bHit)
	{
		FGameplayEventData Payload;
		Payload.TargetData = FGameplayAbilityTargetDataHandle(new FGameplayAbilityTargetData_SingleTargetHit(OutHit));
		Payload.Instigator = Source;
		AbilitySystem->HandleGameplayEvent(InEventTag, &Payload);
	}
	
	return bHit;
}

bool UDGameplayStatics::SphereTraceAndSendEvent(
	AActor* Source,
	FGameplayTag InEventTag,
	FVector Origin,
	float Radius,
	ETraceTypeQuery TraceChannel,
	bool bTraceComplex,
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Source);
	if (AbilitySystemComponent == nullptr)
	{
		DREAM_NLOG(Error, TEXT("AbilitySystemComponent Invalid"));
		return false;
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(Source);

	TArray<FHitResult> Hits;
	bool bBlocking = UKismetSystemLibrary::SphereTraceMulti(Source, Origin, Origin + 10.f,
		Radius, TraceChannel, bTraceComplex, IgnoredActors, DrawDebugType, Hits, true);

	if (!bBlocking)
	{
		DREAM_NLOG(Verbose, TEXT("SphereTraceMulti Miss"));
		return false;
	}

	FGameplayEventData Payload;

	FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
	FGameplayAbilityTargetingLocationInfo SourceLocation;
	SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	SourceLocation.LiteralTransform = FTransform(Origin);
	NewData->SourceLocation = SourceLocation;

	TSet<TWeakObjectPtr<AActor>> TargetActors;

	for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(Hits); It; ++It)
	{
		NewData->TargetActorArray.Add((*It).Actor);
	}

	Payload.Instigator = Source;
	Payload.TargetData = FGameplayAbilityTargetDataHandle(NewData);
	AbilitySystemComponent->HandleGameplayEvent(InEventTag, &Payload);
	return true;
}

void ApplyGameplayEffect(UAbilitySystemComponent* SourceASC, AActor* TargetActor,
	UClass* GEClass, float GELevel, bool bIncludeHitResult, EDDamageType DamageType)
{
	UAbilitySystemComponent* TargetComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	if (TargetComponent == nullptr)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();

	FGameplayEffectContext* EffectContext = EffectContextHandle.Get();
	if (EffectContext->GetScriptStruct() == FDreamGameplayEffectContext::StaticStruct())
	{
		FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContext);

		DreamEffectContext->SetDamageType(DamageType);
		
		if (bIncludeHitResult)
		{
			FHitResult SimpleHit;
			SimpleHit.ImpactPoint = TargetActor->GetActorLocation();
			SimpleHit.Location = TargetActor->GetActorLocation();
			SimpleHit.bBlockingHit = true;
			DreamEffectContext->AddHitResult(SimpleHit);
		}
		
	}

	UGameplayEffect* GE = GEClass->GetDefaultObject<UGameplayEffect>();
	SourceASC->ApplyGameplayEffectToTarget(GE, TargetComponent, GELevel, EffectContextHandle);
}

bool UDGameplayStatics::SphereTraceAndApplyEffect
(
	AActor* Source,
	FVector Origin,
	float Radius,
	ECollisionChannel TraceChannel,
	TSubclassOf<UGameplayEffect> ApplyEffect,
	bool bIgnoredSelf,
	bool bTraceComplex,
	bool bIncludeHitResult,
	EDDamageType EffectDamageType
)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Source);
	if (SourceASC == nullptr)
	{
		DREAM_NLOG(Error, TEXT("SourceASC SourceAbilityComponent Invalid"));
		return false;
	}

	UWorld* World = Source->GetWorld();

	TArray<FHitResult> Hits;

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.bTraceComplex = bTraceComplex;

	if (bIgnoredSelf)
	{
		QueryParams.AddIgnoredActor(Source);
	}

	bool bHit = World->SweepMultiByChannel(Hits, Origin, Origin, FQuat::Identity,
		TraceChannel, FCollisionShape::MakeSphere(Radius), QueryParams);

	if (!bHit)
	{
		DREAM_NLOG(Verbose, TEXT("SphereTraceAndApplyEffect SphereTraceMulti Miss"));
		return false;
	}

	for (FArrayDistinctIterator<FHitResult, FHitResultKeyFuncs> It(Hits); It; ++It)
	{
		AActor* TargetActor = (*It).GetActor();

		ApplyGameplayEffect(SourceASC, TargetActor, ApplyEffect, 1, bIncludeHitResult, EffectDamageType);
	}

	return true;
}

bool UDGameplayStatics::ApplyGameplayEffectToAllActors(UGameplayAbility* Ability, const FGameplayEventData& EventData,
                                                       TSubclassOf<UGameplayEffect> EffectClass, bool bForceIncludeHitResult,
                                                       EDDamageType EffectDamageType)
{
	if (Ability == nullptr)
	{
		DREAM_NLOG(Error, TEXT("Ability Invalid"));
		return false;
	}

	if (EventData.TargetData.Data.Num() == 0)
	{
		DREAM_NLOG(Error, TEXT("EventData.TargetData Is Empty"));
		return false;
	}

	TSharedPtr<FGameplayAbilityTargetData> TargetData = EventData.TargetData.Data[0];

	if (!TargetData.IsValid())
	{
		DREAM_NLOG(Error, TEXT("TargetData Invalid"));
		return false;
	}

	UAbilitySystemComponent* SourceComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EventData.Instigator);
	if (!SourceComponent)
	{
		DREAM_NLOG(Verbose, TEXT("SourceComponent Invalid"));
		return false;
	}

	TArray<TWeakObjectPtr<AActor>> TargetActors = TargetData->GetActors();

	for (TWeakObjectPtr<AActor> Target : TargetActors)
	{
		AActor* TargetActor = Target.Get();
		if (TargetActor == nullptr)
		{
			continue;
		}

		ApplyGameplayEffect(SourceComponent, TargetActor, EffectClass, Ability->GetAbilityLevel(), bForceIncludeHitResult, EffectDamageType);
	}

	return true;
}

void UDGameplayStatics::SpawnDamageWidgets(AActor* TargetActor, const FGameplayCueParameters& Parameters,
                                           bool bHealthSteal)
{
	UAbilitySystemComponent* SourceAbilityComponent = Parameters.EffectContext.GetInstigatorAbilitySystemComponent();
	UAbilitySystemComponent* TargetAbilityComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

	if (SourceAbilityComponent == nullptr || TargetAbilityComponent == nullptr)
	{
		return;
	}

	ADCharacterPlayer* PlayerShooter = Cast<ADCharacterPlayer>(SourceAbilityComponent->GetOwnerActor());
	if (PlayerShooter == nullptr || !PlayerShooter->IsLocallyControlled())
	{
		return;
	}

	const FGameplayEffectContext* EffectContext = Parameters.EffectContext.Get();
	if (EffectContext == nullptr || EffectContext->GetScriptStruct() != FDreamGameplayEffectContext::StaticStruct())
	{
		return;
	}

	if (!bHealthSteal)
	{
		float Health = TargetAbilityComponent->GetNumericAttribute(DMAttrStatics().HealthProperty);
		PlayerShooter->ShowHitEnemyTips(Health == 0.f);
	}
}

FDreamGameplayEffectContext* UDGameplayStatics::MakeDreamEffectContext(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit)
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor))
	{
		FDreamGameplayEffectContext* EffectContext = new FDreamGameplayEffectContext();
		EffectContext->SetFalloffDamage(DamageFalloff);
		EffectContext->AddInstigator(AbilitySystem->GetOwnerActor(), AbilitySystem->GetAvatarActor());
		EffectContext->AddHitResult(Hit);

		return EffectContext;
	}

	DREAM_NLOG(Error, TEXT("未能正确的创建FDreamGameplayEffectContext结构体, 原因：SourceActor不能获取到AbilitySystem"));

	return nullptr;
}

FGameplayEffectContextHandle UDGameplayStatics::MakeDreamEffectContextHandle(AActor* SourceActor, float DamageFalloff, const FHitResult& Hit)
{
	return FGameplayEffectContextHandle(MakeDreamEffectContext(SourceActor, DamageFalloff, Hit));
}

UDGameplayEffectUIData* UDGameplayStatics::GetGameplayUIData(UClass* AbilityClass)
{
	if (AbilityClass && AbilityClass->IsChildOf(UDreamGameplayAbility::StaticClass()))
	{
		if (UDreamGameplayAbility* CDO = AbilityClass->GetDefaultObject<UDreamGameplayAbility>())
		{
			return CDO->AbilityUIData;
		}
	}
	return nullptr;
}

TSubclassOf<UDreamGameplayAbility> UDGameplayStatics::SoftClassToDAbility(const FSoftClassPath& SoftClassPath)
{
	return SoftClassPath.TryLoadClass<UDreamGameplayAbility>();
}

APlayerController* UDGameplayStatics::GetActorPlayerController(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		return PC;
	}

	AActor* OwnerActor = Actor->GetOwner();
	while (OwnerActor != nullptr)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerActor))
		{
			return PC;
		}

		OwnerActor = OwnerActor->GetOwner();
	}

	return nullptr;
}

void UDGameplayStatics::ReturnToMainMenuWithTextReason(APlayerController* PlayerCtrl, FText Reason)
{
	if (PlayerCtrl)
	{
		PlayerCtrl->ClientReturnToMainMenuWithTextReason(Reason);
	}
}

void UDGameplayStatics::CalculateOrientation(const FVector& Velocity, const FRotator& BaseRotation, float& Angle, int32& Orientation)
{
	Angle = 0.f;
	Orientation = 0;
	
	if (!Velocity.IsNearlyZero())
	{
		FRotator DeltaRotation = Velocity.Rotation() - BaseRotation;
		DeltaRotation.Normalize();

		float Tolerance = 2.f;

		bool bRight = DeltaRotation.Yaw > 0;

		float YawAbs = FMath::Abs(DeltaRotation.Yaw);

		if (YawAbs < 45.f + Tolerance)
		{
			Orientation = 0;
			Angle = DeltaRotation.Yaw;
		}
		else if (YawAbs < 135.f - Tolerance)
		{
			float Abs = FMath::Abs(YawAbs - 90.f);
			if (bRight)
			{
				Orientation = 1;
				
				Angle = Abs;
			}
			else
			{
				Orientation = 3;
				Angle = -Abs;
			}
		}
		else
		{
			Orientation = 2;
			float BackwardDegrees = 180.f - YawAbs;
			Angle = bRight ? -BackwardDegrees : BackwardDegrees;
		}

		//UE_LOG(LogDream, Error, TEXT("Orientation: %d, Angle: %f, YawAbs: %f"), Orientation, Angle, YawAbs);
	}
}

float UDGameplayStatics::CalculateDir(const FVector& Velocity, const FRotator& BaseRotation)
{
	FMatrix RotMatrix = FRotationMatrix(BaseRotation);
	FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
	FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
	FVector NormalizedVel = Velocity.GetSafeNormal2D();

	// get a cos(alpha) of forward vector vs velocity
	float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalizedVel);
	// now get the alpha and convert to degree
	float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

	// depending on where right vector is, flip it
	float RightCosAngle = FVector::DotProduct(RightVector, NormalizedVel);
	if (RightCosAngle < 0)
	{
		ForwardDeltaDegree *= -1;
	}

	return ForwardDeltaDegree;
}

void UDGameplayStatics::SpawnWeaponTrailParticles(UObject* WorldContextObject, const FWeaponTrailVFX& TrailVfx,
                                                  const FVector& StartLocation, const FVector& EndLocation)
{
	if (TrailVfx.TrailEffect)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
		{
			FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, EndLocation);
			FVector SpawnLocation = StartLocation + Rotation.RotateVector(TrailVfx.SpawnPositionOffset);

			float Distance = FVector::Distance(SpawnLocation, EndLocation);
			if (Distance > TrailVfx.MinimumSpawnDistance)
			{
				UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, TrailVfx.TrailEffect, SpawnLocation);
				NC->SetVectorParameter(TrailVfx.TrailEndLocationParamName, EndLocation);
				NC->SetFloatParameter(TrailVfx.TrailLifeTimeParamName, Distance / TrailVfx.TrailFlyingSpeed);
			}
		}
	}
}

FName UDGameplayStatics::GetWeaponSocketName(bool bMasterSocket)
{
	UDMProjectSettings* ProjectSettings = UDMProjectSettings::GetProjectSettings();
	return bMasterSocket ? ProjectSettings->GetWepActiveSockName() : ProjectSettings->GetWepHolsterSockName();
}

const FItemDef& UDGameplayStatics::GetItemDef(int32 ItemGuid)
{
	return GSProject->GetItemDefinition(ItemGuid);
}

ULevelListAsset* UDGameplayStatics::GetLevelListAsset()
{
	return GSProject->GetLevelListAsset();
}

void UDGameplayStatics::ReturnToHomeWorld(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		IDreamLoadingScreenModule::Get().StartInGameLoadingScreen();
		const FSoftObjectPath& MainUILevel = UDMProjectSettings::GetProjectSettings()->GetMainUILevel();
		GEngine->SetClientTravel(World, *MainUILevel.GetLongPackageName(), ETravelType::TRAVEL_Absolute);
	}
}

UObject* UDGameplayStatics::TryLoadObject(const FSoftObjectPath& ObjectPath)
{
	return ObjectPath.TryLoad();
}

void UDGameplayStatics::ClearHoldStateHandle(UObject* WorldContextObject, const FHoldStateHandle& Handle)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (Handle.Handle_Task.IsValid())
		{
			World->GetTimerManager().ClearTimer(*Handle.Handle_Task);
		}
	}
}

void UDGameplayStatics::OverrideSystemUserVariableSplineComponent(UNiagaraComponent* NiagaraSystem, const FString& OverrideName, USplineComponent* SplineComponent)
{
	/*if (NiagaraSystem)
	{
		const FNiagaraParameterStore& OverrideParameters = NiagaraSystem->GetOverrideParameters();
		FNiagaraVariable Variable(FNiagaraTypeDefinition(UNiagaraDataInterfaceSpline::StaticClass()), *OverrideName);

		const int32 Index = OverrideParameters.IndexOf(Variable);
		if (Index != INDEX_NONE)
		{
			UNiagaraDataInterfaceSpline* InterfaceSpline = Cast<UNiagaraDataInterfaceSpline>(OverrideParameters.GetDataInterface(Index));
			InterfaceSpline->Source = SplineComponent->GetOwner();
			InterfaceSpline->SourceComponent = SplineComponent;
		}
	}*/
}

void UDGameplayStatics::CloseGame()
{
	FPlatformMisc::RequestExit(false);
}
