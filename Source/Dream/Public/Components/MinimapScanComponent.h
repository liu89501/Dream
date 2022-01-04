// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.h"
#include "Components/ActorComponent.h"
#include "MinimapScanComponent.generated.h"

struct FMinimapDataIterator;

enum class EMinimapDataType
{
	Warning,
	Icon
};

struct FMinimapDataBase
{
public:

	explicit FMinimapDataBase(EMinimapDataType Type)
		: Direction(0.f),
		  Distance(0.f),
		  DataType(Type)
	{
	}

	FMinimapDataBase(float InDirection, float InDistance, EMinimapDataType Type)
		: Direction(InDirection),
		  Distance(InDistance),
		  DataType(Type)
	{
	}

	float Direction;

	float Distance;
	
	EMinimapDataType DataType;
};

struct FMiniMapWarning : FMinimapDataBase
{
	FMiniMapWarning()
		: FMinimapDataBase(EMinimapDataType::Warning)
	{
	}

	FMiniMapWarning(float InDirection, float InDistance)
		: FMinimapDataBase(InDirection, InDistance, EMinimapDataType::Warning)
	{
	}
};

struct FMiniMapIcon : FMinimapDataBase
{
	
public:
	
	FMiniMapIcon()
		: FMinimapDataBase(EMinimapDataType::Icon),
		  Position(FVector2D::ZeroVector),
		  Icon(nullptr)
	{
	}

	FMiniMapIcon(const FVector2D& InPosition, float InYaw, float InDistance, FSlateBrush* InIcon)
		: FMinimapDataBase(InYaw, InDistance, EMinimapDataType::Icon),
		  Position(InPosition),
		  Icon(InIcon)
	{
	}

	FVector2D Position;
	FSlateBrush* Icon;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), config=Game)
class DREAM_API UMinimapScanComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMinimapScanComponent();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

public:

	void AddInfiniteActors(const TArray<AActor*>& TargetActors);

	bool AddInfiniteActor(AActor* TargetActor);

	void RemoveInfiniteActor(AActor* TargetActor);

	void ClearInfiniteActors();

	FMinimapDataIterator GetScannedItemsIterator();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/**
	* 雷达扫描半径， 用于显示小地图相关的东西
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	float RadarScanRadius;
	/**
	* 雷达扫描间隔
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	float ScanInterval;
	/**
	* 雷达扫描半径， 用于显示小地图相关的东西
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPlayer|MiniMap")
	TArray<TEnumAsByte<EObjectTypeQuery>> ScanObjectTypes;

private:

	friend struct FMinimapDataIterator;

	/**
	*  无视小地图扫描半径的Actor 将一直显示在小地图上 需要手动删除
	*/
	UPROPERTY()
	TArray<AActor*> InfiniteActors;

	UPROPERTY(config)
	int32 MaxInfiniteItemNum;

	/**
	*  小地图扫描半径内的Actor
	*/
	UPROPERTY()
	TArray<AActor*> ActorWithinRadius;

	UPROPERTY(config)
	int32 MaxWithinRadiusItemNum;

	UPROPERTY()
	TArray<AActor*> IgnoredTraceOwner;
};


struct FMinimapDataIterator
{
	FMinimapDataIterator()
		: ScanComponentPtr(nullptr),
		  InfiniteActorsIndex(0),
		  ActorWithinRadiusIndex(0)
	{
	}

	explicit FMinimapDataIterator(UMinimapScanComponent* ScanComponent)
		: ScanComponentPtr(ScanComponent),
		  InfiniteActorsIndex(0),
		  ActorWithinRadiusIndex(0)
	{
	}

	TSharedPtr<FMinimapDataBase> Get() const;
	
	void operator++() const
	{
		UMinimapScanComponent* Component = GetComponent();
		if (Component == nullptr)
		{
			return;
		}

		if (Component->InfiniteActors.IsValidIndex(InfiniteActorsIndex))
		{
			++InfiniteActorsIndex;
		}
		else if (Component->ActorWithinRadius.IsValidIndex(ActorWithinRadiusIndex))
		{
			++ActorWithinRadiusIndex;
		}
	}

	operator bool() const
	{
		UMinimapScanComponent* Component = GetComponent();
		if (Component == nullptr)
		{
			return false;
		}

		return Component->InfiniteActors.IsValidIndex(InfiniteActorsIndex) ||
			Component->ActorWithinRadius.IsValidIndex(ActorWithinRadiusIndex);
	}

private:

	UMinimapScanComponent* GetComponent() const
	{
		UMinimapScanComponent* Component = ScanComponentPtr.Get();
		if (Component == nullptr || Component->IsPendingKill())
		{
			return nullptr;
		}

		return Component;
	}

private:

	TWeakObjectPtr<UMinimapScanComponent> ScanComponentPtr;
	mutable int32 InfiniteActorsIndex;
	mutable int32 ActorWithinRadiusIndex;
};
