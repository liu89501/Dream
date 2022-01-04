// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SInteractive.h"
#include "DGameplayStatics.h"
#include "SImage.h"
#include "SlateOptMacros.h"
#include "SlateUtils.h"
#include "GameFramework/InputSettings.h"
#include "Style/DreamStyle.h"
#include "Style/InteractiveWidgetStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInteractive::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._InPlayerOwner;

	InteractiveStyle = FDreamStyle::Get().GetWidgetStyle<FInteractiveStyle>("InteractiveStyle");

	InProgressBrush = InteractiveStyle.PressedProgressBrush;
	PressedInProgress = SlateUtils::GetDynamicFromBrush(InProgressBrush, nullptr);

	InitializeInputComponent();

	FName KeyName = UDGameplayStatics::GetInputActionKeyName(InArgs._InPlayerOwner, "Interactive");

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(&InProgressBrush)
			]
			+ SOverlay::Slot()
            .Padding(2.f)
            [
                SNew(SImage)
                .Image(&InteractiveStyle.KeyBackgroundBrush)
            ]
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(KeyName))
				.TextStyle(&InteractiveStyle.KeyFontStyle)
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		.AutoWidth()
		.Padding(4.f, 0.f)
		[
			SAssignNew(HintText, STextBlock)
            .TextStyle(&InteractiveStyle.HintFontStyle)
		]
	];
}

void SInteractive::EnableInput() const
{
	APlayerController* Controller = PlayerOwner.Get();
	if (InputComponent && Controller != nullptr)
	{
		Controller->PushInputComponent(InputComponent);
	}
}

void SInteractive::DisableInput() const
{
	APlayerController* Controller = PlayerOwner.Get();
	if (InputComponent && Controller != nullptr)
	{
		Controller->PopInputComponent(InputComponent);
	}
}

SInteractive::~SInteractive()
{
	DestroyInputComponent();
}

void SInteractive::InitializeInputComponent()
{
	if (APlayerController* Player = PlayerOwner.Get())
	{
		InputComponent = NewObject<UInputComponent>(Player, UInputSettings::GetDefaultInputComponentClass());
	
		InputComponent->Priority = 10;
		InputComponent->bBlockInput = false;

		FInputActionBinding PressedBinding("Interactive", EInputEvent::IE_Pressed);
		PressedBinding.bConsumeInput = true;
		PressedBinding.ActionDelegate.GetDelegateForManualSet().BindRaw(this, &SInteractive::OnInteractiveButtonPressed);
	
		FInputActionBinding ReleasedBinding("Interactive", EInputEvent::IE_Released);
		ReleasedBinding.bConsumeInput = true;
		ReleasedBinding.ActionDelegate.GetDelegateForManualSet().BindRaw(this, &SInteractive::OnInteractiveButtonReleased);
	
		InputComponent->AddActionBinding(PressedBinding);
		InputComponent->AddActionBinding(ReleasedBinding);
	}
}

void SInteractive::DestroyInputComponent()
{
	DisableInput();

	if (InputComponent)
	{
		InputComponent->ClearActionBindings();
		InputComponent = nullptr;
	}
}

void SInteractive::SetOnCompleted(FOnInteractiveCompleted Delegate)
{
	OnCompleted = Delegate;
}

void SInteractive::SetInteractiveTime(float InInteractiveTime)
{
	InteractiveTime = InInteractiveTime;
}

void SInteractive::ForceDeactivate()
{
	InteractiveTime.Reset();
	InteractiveTime.Deactivate();
}

void SInteractive::SetInteractiveText(const FText& InteractiveText) const
{
	if (HintText.IsValid())
	{
		HintText->SetText(InteractiveText);
	}
}

void SInteractive::OnInteractiveButtonPressed()
{
	APlayerController* Controller = PlayerOwner.Get();
	if (InteractiveStyle.InteractiveSoundCue && Controller)
	{
		UGameplayStatics::PlaySound2D(Controller, InteractiveStyle.InteractiveSoundCue);
	}
	
	InteractiveTime.Activate();
}

void SInteractive::OnInteractiveButtonReleased()
{
	ForceDeactivate();
	UpdateProgress();
}

void SInteractive::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (InteractiveTime.IsActive())
	{
		InteractiveTime.InterpConstantTo(InDeltaTime);

		if (InteractiveTime.IsCompleted())
		{
			ForceDeactivate();
			OnCompleted.ExecuteIfBound();
		}

		UpdateProgress();
	}
}

void SInteractive::UpdateProgress() const
{
	PressedInProgress->SetScalarParameterValue("Percentage", InteractiveTime.GetInProgress());
}

void SInteractive::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(InputComponent);
	Collector.AddReferencedObject(PressedInProgress);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
