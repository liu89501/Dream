#pragma once

class FSlateStyleSet;

struct FDreamStyle
{
	
public:

	static void Initialize();

	static void Shutdown();

	static void ReloadTextures();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:

	static TSharedRef< FSlateStyleSet > Create();

private:

	static TSharedPtr< FSlateStyleSet > DreamStyleInstance;
};