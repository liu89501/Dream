// Fill out your copyright notice in the Description page of Project Settings.

#include "DreamWidgetStatics.h"
#include "DGameplayStatics.h"
#include "DModuleBase.h"
#include "DPlayerController.h"
#include "DPlayerState.h"
#include "DProjectSettings.h"
#include "DreamGameInstance.h"
#include "DreamGameMode.h"
#include "GameMapsSettings.h"
#include "PanelWidget.h"
#include "SViewport.h"
#include "WidgetLayoutLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetTextLibrary.h"
#include "UI/SDialog.h"
#include "UI/SSubtitle.h"

struct FPlayerStatisticsSort
{
	bool operator()(APlayerState& A, APlayerState& B) const
	{
		ADPlayerState* CastA = Cast<ADPlayerState>(&A);
		ADPlayerState* CastB = Cast<ADPlayerState>(&B);

		return CastA && (!CastB || (CastA->GetTotalDamage() < CastB->GetTotalDamage()));
	}
};

FTimerHandle UDreamWidgetStatics::DialogHandle;

UWItemEquipment* UDreamWidgetStatics::MakeWEquipmentFromPW(UObject* WorldContextObject, const FPlayerWeapon& PW)
{
	UWItemEquipment* ViewData = NewObject<UWItemEquipment>(WorldContextObject);
	ViewData->EquipmentId = PW.WeaponId;
	ViewData->SetItemEquipment(MakeShared<FItemEquipment>(PW.ItemGuid, PW.Attributes));
	return ViewData;
}

UWItemEquipment* UDreamWidgetStatics::MakeWEquipmentFromPM(UObject* WorldContextObject, const FPlayerModule& PM)
{
	UWItemEquipment* ViewData = NewObject<UWItemEquipment>(WorldContextObject);
	ViewData->EquipmentId = PM.ModuleId;
	ViewData->SetItemEquipment(MakeShared<FItemEquipment>(PM.ItemGuid, PM.Attributes));
	return ViewData;
}

void UDreamWidgetStatics::ReplaceWidgetChildAt(UPanelWidget* ParentWidget, int32 ChildIndex, UWidget* NewWidget)
{
	if (ParentWidget)
	{
		ParentWidget->ReplaceChildAt(ChildIndex, NewWidget);
	}
}

int32 UDreamWidgetStatics::GetWidgetZOrder(TEnumAsByte<EWidgetOrder::Type> Type)
{
	return static_cast<uint8>(Type.GetValue());
}

void UDreamWidgetStatics::FocusOnViewport(UWidget* Widget)
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

FVector2D UDreamWidgetStatics::CalculationPopupPosition(UObject* WorldContextObject, const FVector2D& WidgetSize, const FVector2D& PointerSize)
{
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(WorldContextObject);
	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(WorldContextObject);

	FVector2D MouseRelativePos = (MousePosition - FVector2D(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f)) * FVector2D(1.f, -1.f);

	FVector2D Result;

	if (MouseRelativePos.X > 0)
	{
		if (MouseRelativePos.Y > 0)
		{
			Result = MousePosition - FVector2D(PointerSize.X, -PointerSize.Y) - FVector2D(WidgetSize.X, 0);
		}
		else
		{
			Result = MousePosition - PointerSize - WidgetSize;
		}
	}
	else
	{
		if (MouseRelativePos.Y > 0)
		{
			Result = MousePosition + PointerSize;
		}
		else
		{
			Result = MousePosition - FVector2D(-PointerSize.X, PointerSize.Y) - FVector2D(0, WidgetSize.Y);
		}
	}

	FVector2D Padding(8.f, 8.f);
	
	Result.X = FMath::Clamp(Result.X, 0.f, ViewportSize.X - WidgetSize.X - Padding.X);
	Result.Y = FMath::Clamp(Result.Y, 0.f, ViewportSize.Y - WidgetSize.Y - Padding.Y);

	return Result;
}

void UDreamWidgetStatics::PopupDialog(UObject* WorldContextObject, EDialogType DialogType, FText Content, float DisplayTime)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UGameViewportClient* GameViewport = World->GetGameViewport();

		if (SDialog::SINGLETON->GetVisibility().IsVisible())
		{
			if (DialogHandle.IsValid())
			{
				World->GetTimerManager().ClearTimer(DialogHandle);
			}
			
			GameViewport->RemoveViewportWidgetContent(SDialog::SINGLETON.ToSharedRef());
		}

		FVector2D ViewportSize;
		GameViewport->GetViewportSize(ViewportSize);
		
		SDialog::SINGLETON->UpdateDialog(DialogType, Content);
		SDialog::SINGLETON->SetRenderTransform(FTransform2D(FVector2D(-ViewportSize.X, 0)));

		// +10 是因为UserWidget的ZOrder都加了10 不加得话就一定会被UMG覆盖
		int32 ZOrder = EWidgetOrder::PlayerCtrlOuter + 10;
		
		GameViewport->AddViewportWidgetContent(SDialog::SINGLETON.ToSharedRef(), ZOrder);

		SDialog::SINGLETON->PlayFadeInAnim(ViewportSize.X);
		
		if (DisplayTime > 0)
		{
			World->GetTimerManager().SetTimer(DialogHandle,
				FTimerDelegate::CreateStatic(&UDreamWidgetStatics::DismissDialog, WorldContextObject), DisplayTime, false);
		}
	}
}

void UDreamWidgetStatics::DismissDialog(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->GetGameViewport()->RemoveViewportWidgetContent(SDialog::SINGLETON.ToSharedRef());
	}
}

const FPropsInfo& UDreamWidgetStatics::GetPropsInfoByItemGuid(int32 ItemGuid)
{
	const FItemDef& ItemDef = UDProjectSettings::GetProjectSettings()->GetItemDefinition(ItemGuid);
	return ItemDef.ItemBaseInfo;
}

UClass* UDreamWidgetStatics::GetItemClassByGuid(int32 ItemGuid)
{
	return UDProjectSettings::GetProjectSettings()->GetItemClassFromGuid(ItemGuid);
}

FModuleExtraData UDreamWidgetStatics::GetModuleExtraData(UClass* ModuleClass)
{
	FModuleExtraData Data;

	if (ModuleClass)
	{
		UDModuleBase* ModuleBase = Cast<UDModuleBase>(ModuleClass->GetDefaultObject());
		Data.Category = ModuleBase->Category;
	}
	
	return Data;
}

FWeaponExtraData UDreamWidgetStatics::GetWeaponExtraData(UClass* WeaponClass)
{
	FWeaponExtraData Data;

	if (WeaponClass)
	{
		AShootWeapon* Weapon = Cast<AShootWeapon>(WeaponClass->GetDefaultObject());
		Data.Magazine = Weapon->AmmoNum;
		Data.RateOfFire = Weapon->RateOfFire;
		Data.FireMode = Weapon->FireMode;
		Data.WeaponType = Weapon->WeaponType;
	}
	
	return Data;
}

TArray<FPlayerStatistics> UDreamWidgetStatics::GetPlayerStatistics(UObject* WorldContextObject)
{
	TArray<FPlayerStatistics> Result;
	
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		AGameStateBase* GameState = World->GetGameState();

		TArray<APlayerState*> PlayerArrayCopy = GameState->PlayerArray;
		PlayerArrayCopy.Sort(FPlayerStatisticsSort());

		for (APlayerState* State : PlayerArrayCopy)
		{
			ADPlayerState* DState = Cast<ADPlayerState>(State);

			FPlayerStatistics Statistics;
			Statistics.Kills = UKismetTextLibrary::Conv_IntToText(DState->GetKills());
			Statistics.TotalDamage = UKismetTextLibrary::Conv_IntToText(DState->GetTotalDamage());
			Statistics.PlayerName = FText::FromString(DState->GetPlayerName());
				
			Result.Add(Statistics);
		}
	}

	return Result;
}

void UDreamWidgetStatics::AddGuideActorToLocalPlayer(AActor* GuideActor)
{
	if (GuideActor != nullptr)
	{
		if (ADPlayerController* Controller = Cast<ADPlayerController>(GEngine->GetFirstLocalPlayerController(GuideActor->GetWorld())))
		{
			Controller->AddGuideActor(GuideActor);
		}
	}
}

void UDreamWidgetStatics::RemoveGuideActorFromLocalPlayer(AActor* GuideActor)
{
	if (GuideActor != nullptr)
	{
		if (ADPlayerController* Controller = Cast<ADPlayerController>(GEngine->GetFirstLocalPlayerController(GuideActor->GetWorld())))
		{
			Controller->RemoveGuideActor(GuideActor);
		}
	}
}

void UDreamWidgetStatics::DisplaySubtitle(UObject* WorldContextObject, FText Text)
{
	if (UDreamGameInstance* Instance = GetGameInstance<UDreamGameInstance>(WorldContextObject))
	{
		Instance->GetSubtitleWidget()->DisplaySubtitle(Text);
	}
}

void UDreamWidgetStatics::HiddenSubtitle(UObject* WorldContextObject)
{
	if (UDreamGameInstance* Instance = GetGameInstance<UDreamGameInstance>(WorldContextObject))
	{
		Instance->GetSubtitleWidget()->HiddenSubtitle();
	}
}

FText UDreamWidgetStatics::GetWeaponTypeName(EWeaponType WeaponType)
{
	return UDProjectSettings::GetProjectSettings()->GetWeaponTypeName(WeaponType);
}

const FPropsInfo& UDreamWidgetStatics::GetPropsInfo(const FItemHandle& ItemHandle)
{
	if (ItemHandle.IsValid())
	{
		return GetPropsInfoByItemGuid(ItemHandle->GetItemGuid());
	}

	return FEmptyStruct::EmptyPropsInfo;
}

const FQualityInfo& UDreamWidgetStatics::GetQualityInfo(EPropsQuality Quality)
{
	return UDProjectSettings::GetProjectSettings()->GetQualityInfo(Quality);
}

template <class Class>
Class* UDreamWidgetStatics::GetGameInstance(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetGameInstance<Class>();
	}
	return nullptr;
}

int32 FLevelInformation::GetGameModeMaxPlayers() const
{
	if (UClass* GameModeClass = LoadClass<ADreamGameMode>(nullptr, *UGameMapsSettings::GetGameModeForName(GameModeClassAlias)))
	{
		return GameModeClass->GetDefaultObject<ADreamGameMode>()->GetGameModeMaxPlayers();
	}
	
	return ADreamGameMode::DEFAULT_MAX_PLAYERS;
}