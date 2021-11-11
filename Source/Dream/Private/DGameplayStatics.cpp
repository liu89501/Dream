// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable CppExpressionWithoutSideEffects
#include "DGameplayStatics.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SubclassOf.h"
#include "GameFramework/Actor.h"
#include "JsonUtilities.h"
#include "DreamGameInstance.h"
#include "AIController.h"
#include "DCharacterPlayer.h"
#include "DGameplayEffectUIData.h"
#include "DGameUserSettings.h"
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"
#include "DreamGameplayAbility.h"
#include "MatchmakingCallProxy.h"
#include "NavigationSystem.h"
#include "UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PanelWidget.h"
#include "GameFramework/PhysicsVolume.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceSpline.h"
#include "NiagaraFunctionLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "PlayerDataInterfaceStatic.h"
#include "SViewport.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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

bool UDGameplayStatics::GetQualityInfo(UObject* WorldContextObject, EPropsQuality Quality, FQualityInfo& QualityInfo)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		if (UDreamGameInstance* GameInstance = World->GetGameInstance<UDreamGameInstance>())
		{
			QualityInfo = GameInstance->QualitySettings.FindChecked(Quality);

			return true;
		}
	}

	return false;
}

bool UDGameplayStatics::ContainsActionKey(APlayerController* PlayerController, FKey Key, FName ActionName)
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

const FKey& UDGameplayStatics::GetInputActionKey(APlayerController* PlayerController, FName ActionName)
{
	if (PlayerController)
	{
		TArray<FInputActionKeyMapping> ActionKeyMapping = PlayerController->PlayerInput->GetKeysForAction(ActionName);
		if (ActionKeyMapping.IsValidIndex(0))
		{
			return ActionKeyMapping[0].Key;
		}
	}

	return EKeys::AnyKey;
}

void UDGameplayStatics::ServerTravel(UObject* WorldContextObject, const FString& URL, bool bAbsolute)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (World)
	{
		World->ServerTravel(URL, bAbsolute);
	}
}

FText UDGameplayStatics::ToTimeText(int32 TotalSeconds)
{
	int32 Minute = TotalSeconds / 60;
	int32 Seconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minute, Seconds));
}

void UDGameplayStatics::StopMatchmaking(const FMatchmakingHandle& Handle)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(PartySessionName);
	}
	
	if (Handle.IsValid())
	{
		Handle.MatchmakingCallProxy->ClearAllHandle();
	}
}

int32 UDGameplayStatics::GetSessionPlayers()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		if (FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(PartySessionName))
		{
			return NamedSession->SessionSettings.NumPublicConnections - NamedSession->NumOpenPublicConnections;
		}
	}

	// 默认为1 因为你自己也是个人
	return 1;
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

	FGameplayEventData Payload;
	Payload.TargetData = FGameplayAbilityTargetDataHandle(new FGameplayAbilityTargetData_SingleTargetHit(OutHit));
	Payload.Instigator = Source;
	AbilitySystem->HandleGameplayEvent(InEventTag, &Payload);
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
	if (ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(WorldContextObject, 0)))
	{
		PlayerCharacter->AddInfiniteActor(Actor);
	}
}

void UDGameplayStatics::RemoveIconInPlayerMinimap(UObject* WorldContextObject, AActor* Actor)
{
	if (ADCharacterPlayer* PlayerCharacter = Cast<ADCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(WorldContextObject, 0)))
	{
		PlayerCharacter->RemoveInfiniteActor(Actor);
	}
}

void UDGameplayStatics::ReplaceWidgetChildAt(UPanelWidget* ParentWidget, int32 ChildIndex, UWidget* NewWidget)
{
	if (ParentWidget)
	{
		ParentWidget->ReplaceChildAt(ChildIndex, NewWidget);
	}
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
		float Health = TargetAbilityComponent->GetNumericAttribute(DreamAttrStatics().HealthProperty);
		PlayerShooter->ShowHitEnemyTips(Health == 0.f);
	}
}

FDreamGameplayEffectContext* UDGameplayStatics::MakeDreamEffectContext(AActor* SourceActor, float DamageFalloff,
                                                                       const FHitResult& Hit, const FVector& Origin)
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor))
	{
		FDreamGameplayEffectContext* EffectContext = new FDreamGameplayEffectContext();
		EffectContext->SetFalloffDamage(DamageFalloff);
		EffectContext->AddInstigator(AbilitySystem->GetOwnerActor(), AbilitySystem->GetAvatarActor());
		EffectContext->AddOrigin(Origin);
		EffectContext->AddHitResult(Hit);

		return EffectContext;
	}

	DREAM_NLOG(Error, TEXT("未能正确的创建FDreamGameplayEffectContext结构体, 原因：SourceActor不能获取到AbilitySystem"));

	return nullptr;
}

FGameplayEffectContextHandle UDGameplayStatics::MakeDreamEffectContextHandle(
	AActor* SourceActor, float DamageFalloff, const FHitResult& Hit, const FVector& Origin)
{
	return FGameplayEffectContextHandle(MakeDreamEffectContext(SourceActor, DamageFalloff, Hit, Origin));
}

UDGameplayEffectUIData* UDGameplayStatics::GetGameplayUIData(const TSubclassOf<UDreamGameplayAbility>& AbilityClass)
{
	if (AbilityClass)
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

bool UDGameplayStatics::ApplyGameplayEffectToAllActors(UGameplayAbility* Ability, const FGameplayEventData& EventData,
                                                       TSubclassOf<UGameplayEffect> EffectClass)
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
		if (!Target.IsValid())
		{
			continue;
		}

		UAbilitySystemComponent* TargetComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
			Target.Get());

		if (TargetComponent == nullptr)
		{
			continue;
		}

		FGameplayEffectContextHandle EffectContextHandle = SourceComponent->MakeEffectContext();
		EffectContextHandle.SetAbility(Ability);
		EffectContextHandle.AddOrigin(TargetData->GetOrigin().GetLocation());

		UGameplayEffect* GE = EffectClass->GetDefaultObject<UGameplayEffect>();
		SourceComponent->ApplyGameplayEffectToTarget(GE, TargetComponent, Ability->GetAbilityLevel(),
		                                             EffectContextHandle);
	}

	return true;
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

void UDGameplayStatics::CalculateFBDirection(const FVector& Velocity, const FRotator& BaseRotation, float& Angle,
                                             bool& bIsBackward)
{
	bIsBackward = false;
	Angle = 0.f;

	if (!Velocity.IsNearlyZero())
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

		//DREAM_NLOG(Error, TEXT("ForwardCosAngle: %f"), ForwardCosAngle)
		//DREAM_NLOG(Error, TEXT("RightCosAngle: %f"), RightCosAngle)

		ForwardCosAngle = FMath::IsNearlyZero(ForwardCosAngle, 0.001f) ? 0.f : ForwardCosAngle;
		RightCosAngle = FMath::IsNearlyZero(RightCosAngle, 0.001f) ? 0.f : RightCosAngle;

		bIsBackward = ForwardCosAngle < 0;
		bool bIsLeft = RightCosAngle < 0;

		if (bIsBackward)
		{
			ForwardDeltaDegree = 180.f - ForwardDeltaDegree;
		}

		if (bIsBackward)
		{
			if (!bIsLeft)
			{
				ForwardDeltaDegree *= -1;
			}
		}
		else
		{
			if (bIsLeft)
			{
				ForwardDeltaDegree *= -1;
			}
		}

		Angle = ForwardDeltaDegree;
	}
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
				UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World, TrailVfx.TrailEffect, SpawnLocation, FRotator::ZeroRotator,
					FVector::ZeroVector, true, true, ENCPoolMethod::AutoRelease);

				NC->SetVectorParameter(TrailVfx.TrailEndLocationParamName, EndLocation);
				NC->SetFloatParameter(TrailVfx.TrailLifeTimeParamName, Distance / TrailVfx.TrailFlyingSpeed);
			}
		}
	}
}

int32 UDGameplayStatics::GetWidgetZOrder(TEnumAsByte<EWidgetOrder::Type> Type)
{
	return static_cast<uint8>(Type.GetValue());
}

void UDGameplayStatics::FocusOnViewport(UWidget* Widget)
{
	if (Widget == nullptr)
	{
		return;
	}

	if (UGameViewportClient* ViewportClient = Widget->GetWorld()->GetGameViewport())
	{
		TSharedPtr<SViewport> ViewportWidget = ViewportClient->GetGameViewportWidget();
		APlayerController* PlayerController = Widget->GetOwningPlayer();
			
		if (ViewportWidget.IsValid() && PlayerController)
		{
			FReply& SlateOperations = PlayerController->GetLocalPlayer()->GetSlateOperations();
			SlateOperations.SetUserFocus(ViewportWidget.ToSharedRef());
		}
	}
}

ULevelListAsset* UDGameplayStatics::GetLevelList(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UDreamGameInstance* Instance = Cast<UDreamGameInstance>(World->GetGameInstance()))
		{
			return Instance->Levels;
		}
	}

	return nullptr;
}

void UDGameplayStatics::ForceDestroyComponent(UActorComponent* Component)
{
	if (Component)
	{
		Component->DestroyComponent(true);
	}
}

UItemDataWeapon* UDGameplayStatics::WeaponCastToExInformation(const FPlayerWeapon& Weapon)
{
	UItemDataWeapon* ItemDataWeapon = NewObject<UItemDataWeapon>();
	ItemDataWeapon->WeaponClass = Weapon.WeaponClass;
	ItemDataWeapon->Attributes = Weapon.Attributes;
	ItemDataWeapon->ItemId = Weapon.WeaponId;
	ItemDataWeapon->InitializeExtraProperties();
	return ItemDataWeapon;
}

UItemDataModule* UDGameplayStatics::ModuleCastToExInformation(const FPlayerModule& Module)
{
	UItemDataModule* ItemDataModule = NewObject<UItemDataModule>();
	ItemDataModule->ModuleClass = Module.ModuleClass;
	ItemDataModule->Attributes = Module.Attributes;
	ItemDataModule->ItemId = Module.ModuleId;
	ItemDataModule->InitializeExtraProperties();
	return ItemDataModule;
}

const FPlayerProperties& UDGameplayStatics::GetCachedPlayerProperties()
{
	return FPDIStatic::Get()->GetCachedProperties();
}

void UDGameplayStatics::BindPropertiesChangeDelegate(UObject* Object, FName FunctionName, FMulticastDelegateHandle& Handle)
{
	Handle.Handle = FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.AddUFunction(Object, FunctionName);
}

void UDGameplayStatics::RemovePropertiesChangeDelegateHandle(const FMulticastDelegateHandle& MulticastDelegateHandle)
{
	FPDIStatic::Get()->GetPlayerDataDelegate().OnPropertiesChange.Remove(MulticastDelegateHandle.Handle);
}

void UDGameplayStatics::GroupModules(const TArray<FPlayerModule>& Modules, TMap<EModuleCategory, FPlayerModuleList>& GroupModules)
{
	for (const FPlayerModule& PM : Modules)
	{
		GroupModules.FindOrAdd(PM.Category).Modules.Add(PM);
	}
}

void UDGameplayStatics::GetEquippedModule(const TArray<FPlayerModule>& Modules, TArray<FPlayerModule>& EquippedModules)
{
	for (const FPlayerModule& PM : Modules)
	{
		if (PM.bEquipped)
		{
			EquippedModules.Add(PM);
		}
	}
}

void UDGameplayStatics::GetAllItems(UItemData* ItemData, TArray<UItemData*>& Items)
{
	for (UItemData* Item : FItemDataRange(ItemData))
	{
		Items.Add(Item);
	}
}

TSubclassOf<AShootWeapon> UDGameplayStatics::SoftClassPathToWeaponClass(const FSoftClassPath& SoftClassPath)
{
	return SoftClassPath.TryLoadClass<AShootWeapon>();
}

TSubclassOf<UDModuleBase> UDGameplayStatics::SoftClassPathToModuleClass(const FSoftClassPath& SoftClassPath)
{
	return SoftClassPath.TryLoadClass<UDModuleBase>();
}

UObject* UDGameplayStatics::TryLoadObject(const FSoftObjectPath& ObjectPath)
{
	return ObjectPath.TryLoad();
}

UDQuestDescription* UDGameplayStatics::SoftObjectToDQuestDescription(const FSoftObjectPath& ObjectPath)
{
	return Cast<UDQuestDescription>(ObjectPath.TryLoad());
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

void UDGameplayStatics::PlayImpactAnim(ACharacter* Character, const FImpactMontages& HitMontages, float RotationYaw)
{
	float AbsYaw = FMath::Abs(RotationYaw);
	if (AbsYaw < 45.f)
	{
		Character->PlayAnimMontage(HitMontages.FHitMontage);
	}
	else if (AbsYaw < 90.f)
	{
		if (RotationYaw > 0)
		{
			Character->PlayAnimMontage(HitMontages.RHitMontage);
		}
		else
		{
			Character->PlayAnimMontage(HitMontages.LHitMontage);
		}
	}
	else
	{
		Character->PlayAnimMontage(HitMontages.BHitMontage);
	}
}

void UDGameplayStatics::OverrideSystemUserVariableSplineComponent(UNiagaraComponent* NiagaraSystem, const FString& OverrideName, USplineComponent* SplineComponent)
{
	if (NiagaraSystem)
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
	}
}

void UDGameplayStatics::CloseGame()
{
	FPlatformMisc::RequestExit(false);
}
