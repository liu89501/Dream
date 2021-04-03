// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "SubclassOf.h"
#include "GameFramework/Actor.h"
#include "JsonUtilities.h"
#include "DreamGameInstance.h"
#include "PDSAsync_RunServer.h"
#include "AIController.h"
#include "DCharacterPlayer.h"
#include "DreamAttributeSet.h"
#include "DreamGameplayType.h"
#include "GameplayAbility.h"
#include "MatchmakingCallProxy.h"
#include "NavigationSystem.h"
#include "UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PanelWidget.h"
#include "GameplayEffectTypes.h"
#include "PropsInterface.h"
#include "GameFramework/PhysicsVolume.h"
#include "PhysicsEngine/PhysicsSettings.h"

TSubclassOf<AShootWeapon> UDGameplayStatics::LoadWeaponClass(FString QualifiedName)
{
	return LoadClass<AActor>(nullptr, *QualifiedName);
}

FString UDGameplayStatics::GetClassPathName(TSubclassOf<UObject> ObjectClass)
{
	return ObjectClass->GetPathName();
}

bool UDGameplayStatics::GetClassPropsInfo(const FString& ClassString, FPropsInfo& PropsInfo)
{
	UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassString);
	if (LoadedClass)
	{
		if (LoadedClass->ImplementsInterface(IPropsInterface::UClassType::StaticClass()))
		{
			PropsInfo = IPropsInterface::Execute_GetPropsInfo(LoadedClass->GetDefaultObject());
			return true;
		}
	}
	return false;
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

FLinearColor UDGameplayStatics::ParseColorFromString(const FString& ColorString)
{
	FColor Color;
	Color.InitFromString(ColorString);
	return FLinearColor(Color);
}

bool UDGameplayStatics::ContainsActionKey(APlayerController* PlayerController, FKey Key , FName ActionName)
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

	return NAME_None;
}

bool UDGameplayStatics::SetFocus(UWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	if (APlayerController* PlayerController = Widget->GetOwningPlayer())
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			TSharedRef<SWidget> SafeWidget = Widget->TakeWidget();
			FReply& DelayedSlateOperations = LocalPlayer->GetSlateOperations();
			DelayedSlateOperations.SetUserFocus(SafeWidget);

			return true;
		}
	}

	return false;
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
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);

	if (OSS)
	{
		IOnlineSessionPtr SessionInt = OSS->GetSessionInterface();
		SessionInt->CancelFindSessions();
		SessionInt->DestroySession(GameSessionName);
	}

	if (Handle.IsValid())
	{
		Handle.MatchmakingCallProxy->ClearAllHandle();
	}
}

int32 UDGameplayStatics::GetJoinedPlayerNum()
{
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
	{
		FNamedOnlineSession* Session = OSS->GetSessionInterface()->GetNamedSession(GameSessionName);
		if (Session)
		{
			return Session->SessionSettings.NumPublicConnections - Session->NumOpenPublicConnections;
		}
	}
	return 0;
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
	bool bBlocking = UKismetSystemLibrary::SphereTraceMulti(Source, Origin,
		(Origin + 10.f), Radius, TraceChannel, bTraceComplex, IgnoredActors, DrawDebugType, Hits, true);

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

void UDGameplayStatics::ReplaceWidgetChildAt(UPanelWidget* ParentWidget, int32 ChildIndex, UWidget* NewWidget)
{
	if (ParentWidget)
	{
		ParentWidget->ReplaceChildAt(ChildIndex, NewWidget);
	}
}

void UDGameplayStatics::SpawnDamageWidgets(AActor* TargetActor, const FGameplayCueParameters& Parameters, bool bHealthSteal)
{
	UAbilitySystemComponent* SourceAbilityComponent = Parameters.EffectContext.GetInstigatorAbilitySystemComponent();
	UAbilitySystemComponent* TargetAbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	
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

	if (ADEnemyBase* Enemy = Cast<ADEnemyBase>(TargetActor))
	{
		Enemy->ShowHealthUI();
	}

	if (!bHealthSteal)
	{
		float Health = TargetAbilityComponent->GetNumericAttribute(DreamAttrStatics().HealthProperty);
		PlayerShooter->ShowHitEnemyTips(Health == 0.f);
	}

	/*const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContext);
	const TArray<FVector>& HitPoints = DreamEffectContext->GetHitPoints();

	for (FVector Point : HitPoints)
	{
		if (bHealthSteal)
		{
			PlayerShooter->SpawnDamageWidget(Point, Parameters.RawMagnitude, false, true);
		}
		else
		{
			PlayerShooter->SpawnDamageWidget(Point, Parameters.RawMagnitude, DreamEffectContext->GetDamageCritical(), false);
		}
	}*/
}

FDreamGameplayEffectContext* UDGameplayStatics::MakeDreamEffectContextHandle(AActor* SourceActor,
                                                                             UCurveFloat* DamageFalloffCurve, const FHitResult& Hit, const FVector& Origin)
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor))
	{
		FDreamGameplayEffectContext* EffectContext = new FDreamGameplayEffectContext();
		if (DamageFalloffCurve)
		{
			float FloatValue = DamageFalloffCurve->GetFloatValue(FVector::Distance(Origin, Hit.ImpactPoint));
			EffectContext->SetFalloffDamage(FloatValue);
		}
		
		EffectContext->AddInstigator(AbilitySystem->GetOwnerActor(), AbilitySystem->GetAvatarActor());
		EffectContext->AddOrigin(Origin);
		EffectContext->AddHitResult(Hit);

		return EffectContext;
	}

	DREAM_NLOG(Error, TEXT("未能正确的创建FDreamGameplayEffectContext结构体, 原因：SourceActor不能获取到AbilitySystem"));

	return nullptr;
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

void UDGameplayStatics::CalculateFBDirection(const FVector& Velocity, const FRotator& BaseRotation, float& Angle, bool& bIsBackward)
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

		Angle = ForwardDeltaDegree * 2;
	}
}
