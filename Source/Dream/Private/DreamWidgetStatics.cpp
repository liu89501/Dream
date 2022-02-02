// Fill out your copyright notice in the Description page of Project Settings.

#include "DreamWidgetStatics.h"
#include "DGameplayStatics.h"
#include "DModuleBase.h"
#include "DMViewportClient.h"
#include "DMPlayerController.h"
#include "DPlayerState.h"
#include "DMProjectSettings.h"
#include "GameMapsSettings.h"
#include "PanelWidget.h"
#include "SViewport.h"
#include "WidgetLayoutLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetTextLibrary.h"
#include "UI/SDialog.h"

struct FPlayerStatisticsSort
{
	bool operator()(APlayerState& A, APlayerState& B) const
	{
		ADPlayerState* CastA = Cast<ADPlayerState>(&A);
		ADPlayerState* CastB = Cast<ADPlayerState>(&B);

		return CastA && (!CastB || (CastA->GetTotalDamage() < CastB->GetTotalDamage()));
	}
};

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

void UDreamWidgetStatics::PopupDialog(EDialogType DialogType, FText Content, float DisplayTime)
{
	if (UDMViewportClient* ViewportClient = Cast<UDMViewportClient>(GEngine->GameViewport))
	{
		ViewportClient->PopupDialog(Content, DialogType, DisplayTime);
	}
}

void UDreamWidgetStatics::DismissDialog()
{
	if (UDMViewportClient* ViewportClient = Cast<UDMViewportClient>(GEngine->GameViewport))
	{
		ViewportClient->DismissDialog();
	}
}

const FPropsInfo& UDreamWidgetStatics::GetPropsInfoByItemGuid(int32 ItemGuid)
{
	const FItemDef& ItemDef = GSProject->GetItemDefinition(ItemGuid);
	return ItemDef.ItemBaseInfo;
}

UClass* UDreamWidgetStatics::GetItemClassByGuid(int32 ItemGuid)
{
	return GSProject->GetItemClassFromGuid(ItemGuid);
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
		if (ADMPlayerController* Controller = Cast<ADMPlayerController>(GEngine->GetFirstLocalPlayerController(GuideActor->GetWorld())))
		{
			Controller->AddGuideActor(GuideActor);
		}
	}
}

void UDreamWidgetStatics::RemoveGuideActorFromLocalPlayer(AActor* GuideActor)
{
	if (GuideActor != nullptr)
	{
		if (ADMPlayerController* Controller = Cast<ADMPlayerController>(GEngine->GetFirstLocalPlayerController(GuideActor->GetWorld())))
		{
			Controller->RemoveGuideActor(GuideActor);
		}
	}
}

void UDreamWidgetStatics::DisplaySubtitle(FText Text)
{
	if (UDMViewportClient* ViewportClient = Cast<UDMViewportClient>(GEngine->GameViewport))
	{
		ViewportClient->DisplaySubtitle(Text);
	}
}

void UDreamWidgetStatics::AddWidgetToViewport(UUserWidget* Widget, TEnumAsByte<EWidgetOrder::Type> ZOrder)
{
	if (Widget && !Widget->IsInViewport())
	{
		Widget->AddToViewport(ZOrder.GetValue());
	}
}

FText UDreamWidgetStatics::WeightToPercentageText(float Weight)
{
	FString Percentage;
	Percentage.AppendInt(FMath::RoundFromZero(Weight * 100));
	Percentage.AppendChar(TEXT('%'));
	return FText::FromString(Percentage);
}

FText UDreamWidgetStatics::GetWeaponTypeName(EWeaponType WeaponType)
{
	return UDMProjectSettings::GetProjectSettings()->GetWeaponTypeName(WeaponType);
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
	return UDMProjectSettings::GetProjectSettings()->GetQualityInfo(Quality);
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