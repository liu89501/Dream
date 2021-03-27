#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"


/**
	错误提示界面
*/
class SDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialog)
	{}

		SLATE_ARGUMENT(FText, Content)

		SLATE_ARGUMENT(UFont*, FontBase)
		
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs);

protected:

	FReply OnClick();

private:

	FTextBlockStyle ButtonTextBlockStyle;
	FButtonStyle ButtonStyle;

	FSlateBrush InnerBackground;
};