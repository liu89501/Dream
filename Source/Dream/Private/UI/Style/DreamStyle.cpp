#include "DreamStyle.h"
#include "SlateGameResources.h"
#include "SlateStyleRegistry.h"
#include "SlateStyle.h"

#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::ProjectContentDir() / "Slate" / RelativePath + TEXT(".ttf"), __VA_ARGS__ )
#define EMPTY_IMAGE_BRUSH( ... ) FSlateImageBrush( (UObject*)nullptr, __VA_ARGS__ )

TSharedPtr< FSlateStyleSet > FDreamStyle::DreamStyleInstance;

void FDreamStyle::Initialize()
{
	if (!DreamStyleInstance.IsValid())
	{
		DreamStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*DreamStyleInstance);
	}
}

void FDreamStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*DreamStyleInstance);
	DreamStyleInstance.Reset();
}

TSharedRef<FSlateStyleSet> FDreamStyle::Create()
{
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(GetStyleSetName(), "/Game/Slate/Style","/Game/Slate/Style");

	StyleRef->Set("Dream.DefaultProgressBarStyle", FProgressBarStyle()
	                                        .SetBackgroundImage(FSlateColorBrush(FLinearColor::Transparent))
	                                        .SetFillImage(EMPTY_IMAGE_BRUSH(FVector2D(32,32)))
	);

	StyleRef->Set("Dream.BlackTranslucentBackground", new EMPTY_IMAGE_BRUSH(FVector2D(1,1), FLinearColor(0,0,0,0.4f)));
	StyleRef->Set("Dream.BlackBackground", new EMPTY_IMAGE_BRUSH(FVector2D(1,1), FLinearColor(0,0,0,1)));
	
	StyleRef->Set("Dream.WhiteBrush", new EMPTY_IMAGE_BRUSH(FVector2D(1,1), FLinearColor::White));

	return StyleRef;
}

void FDreamStyle::ReloadTextures()
{
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FDreamStyle::Get()
{
	return *DreamStyleInstance;
}

FName FDreamStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DreamStyle"));
	return StyleSetName;
}

#undef ASSET_FONT
