#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayEffectUIData.h"
#include "DGameplayEffectUIData.generated.h"

UCLASS()
class DREAM_API UDGameplayEffectUIData : public UGameplayEffectUIData
{
    GENERATED_BODY()
    
public:
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIData")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIData")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIData")
    FText Description;
};

UCLASS()
class DREAM_API UDGameplayEffectUIData_Buff : public UGameplayEffectUIData
{
    GENERATED_BODY()
    
public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIData")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIData")
    FGameplayTag BuffTag;
};