// Copyright Epic Games, Inc. All Rights Reserved.

#include "DreamLoadingScreen.h"
#include "GenericApplication.h"
#include "GenericApplicationMessageHandler.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "Engine/Font.h"
#include "Engine/GameViewportClient.h"

#define LOCTEXT_NAMESPACE "LodingScreen"

struct FDreamLoadingScreenBrush : FSlateDynamicImageBrush, FGCObject
{
	FDreamLoadingScreenBrush( const FName InTextureName, const FVector2D& InImageSize )
		: FSlateDynamicImageBrush( InTextureName, InImageSize )
	{
		SetResourceObject(LoadObject<UObject>( nullptr, *InTextureName.ToString() ));
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		FSlateBrush::AddReferencedObjects(Collector);
	}
};

struct FDreamLoadingScreenFont : FSlateFontInfo
{
	FDreamLoadingScreenFont( const FString& InFont, int32 InFontSize, const FName& TypeFace )
		: FSlateFontInfo( LoadObject<UObject>( nullptr, *InFont ), InFontSize, TypeFace )
	{
	}
};

class SLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		static const FName LoadingScreenName(TEXT("/Game/Main/Widget/Texture/Misc/LoadingScreen"));
		static const FName ThrobberBrushName(TEXT("/Game/Main/Widget/Material/Props/MI_Loading_Point"));
		static const FString FontName(TEXT("/Game/Main/Asset/Font/Dream_Font"));

		//since we are not using game styles here, just load one image
		LoadingScreenBrush = MakeShareable( new FDreamLoadingScreenBrush( LoadingScreenName, FVector2D(1920, 1080) ) );
		ThrobberBrush = MakeShareable( new FDreamLoadingScreenBrush( ThrobberBrushName, FVector2D(16, 16) ) );

		LineBrush = MakeShared<FSlateColorBrush>(FLinearColor(0.776042f,0.605725f,0.07294f,1));
		LineBrush->ImageSize = FVector2D(1, 1);

		LineBackgroundBrush = MakeShared<FSlateColorBrush>(FLinearColor(0,0,0,1));
		LineBackgroundBrush->ImageSize = FVector2D(64, 64);

		ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(LoadingScreenBrush.Get())
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SSafeZone)
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Left)
				.Padding(FMargin(100, 0, 0, 180))
				.IsTitleSafe(true)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(4)
					[
						SNew(STextBlock)
						.Font(FDreamLoadingScreenFont(FontName, 20, TEXT("Light")))
						.Text(LOCTEXT("Loading", "加载中"))
					]
					+SHorizontalBox::Slot()
					.Padding(4)
					[
						SNew(SThrobber)
                        .Visibility(EVisibility::SelfHitTestInvisible)
                        .PieceImage(ThrobberBrush.Get())
                        .NumPieces(6)
					]
				]
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
            .VAlign(VAlign_Bottom)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(LineBackgroundBrush.Get())
				]
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
				[
					SNew(SImage)
					.Image(LineBrush.Get())
				]
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(LineBackgroundBrush.Get())
				]
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					SNew(SImage)
					.Image(LineBrush.Get())
				]
			]
		];
	}

private:
	
	EVisibility GetLoadIndicatorVisibility() const
	{
		return EVisibility::SelfHitTestInvisible;
	}

	TSharedPtr<FSlateBrush> LineBrush;
	TSharedPtr<FSlateBrush> LineBackgroundBrush;

	TSharedPtr<FSlateBrush> LoadingScreenBrush;
	
	TSharedPtr<FSlateBrush> ThrobberBrush;
	
};

class FDreamLoadingScreenModule : public IDreamLoadingScreenModule
{
public:


	virtual TSharedRef<SWidget> NewLoadingScreen() override
	{
		return SNew(SLoadingScreen);
	}

	virtual void StartupModule() override
	{		
	}
	
	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void StartInGameLoadingScreen() override
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.MinimumLoadingScreenDisplayTime = 3.f;
		LoadingScreen.WidgetLoadingScreen = NewLoadingScreen();
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
};

IMPLEMENT_GAME_MODULE(FDreamLoadingScreenModule, DreamLoadingScreen);

