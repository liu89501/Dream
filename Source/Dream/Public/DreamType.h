// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamType.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDream, Log, All);

#define DREAM_NLOG(Verbosity, Format, ...) UE_LOG(LogDream, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));
#define DREAM_RLOG(Verbosity, Format, ...) UE_LOG(LogDream, Verbosity, TEXT("%s <-> %s"), *UEnum::GetValueAsString(GetLocalRole()), *FString::Printf(Format, ##__VA_ARGS__));
#define DREAM_CRLOG(Verbosity, Format, ...) UE_LOG(LogDream, Verbosity, TEXT("%s <-> %s"), *UEnum::GetValueAsString(GetOwnerRole()), *FString::Printf(Format, ##__VA_ARGS__));

#define Collision_ObjectType_Projectile ECollisionChannel::ECC_GameTraceChannel1
#define Collision_ObjectType_Weapon ECollisionChannel::ECC_GameTraceChannel2

#define Collision_ObjectTrace_Player EObjectTypeQuery::ObjectTypeQuery3

#define Collision_TraceType_Climbable ETraceTypeQuery::TraceTypeQuery3

#define SOCKET_WeaponMuzzle TEXT("Muzzle")

#define MAX_WeaponSlot 2

namespace DMActorTagName
{
	extern const FName Death;
}


UENUM(BlueprintType)
enum class EPawnType : uint8
{
	Shooter,
    Monster
};

UENUM(BlueprintType)
namespace EWidgetOrder
{
	enum Type
	{
		None,
		Player,
		PlayerPopup,
		PlayerOuter,
		PlayerCtrl,
		PlayerCtrlPopup,
		PlayerCtrlOuter,
		InteractiveUI,
		RewardPopup,
		LoadingPopup,
		Subtitle,
		Max
	};
}

USTRUCT(BlueprintType)
struct FSurfaceImpactEffect
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* ImpactDecal;
};

USTRUCT(BlueprintType)
struct FRangeRandomFloat
{
	GENERATED_BODY()

	FRangeRandomFloat()
        : Min(0),
          Max(0)
	{
	}

	FRangeRandomFloat(float InMin, float InMax)
        : Min(InMin),
          Max(InMax)
	{
	}

	float GetRandomFloat() const;

	UPROPERTY(EditAnywhere)
	float Min;

	UPROPERTY(EditAnywhere)
	float Max;
};

USTRUCT(BlueprintType)
struct FRangeRandomInt
{
	GENERATED_BODY()

	FRangeRandomInt()
        : Min(0),
          Max(0)
	{
	}

	FRangeRandomInt(int32 InMin, int32 InMax)
        : Min(InMin),
          Max(InMax)
	{
	}

	int32 GetRandomInt() const;

	UPROPERTY(EditAnywhere)
	int32 Min;

	UPROPERTY(EditAnywhere)
	int32 Max;
};

namespace IPTools
{
	uint32 IPV4StringToUint32(const FString& IpString);
	FString IPV4Uint32ToString(uint32 IP);
}

struct FTimerTemplate
{
	
public:

	FTimerTemplate()
		: TargetTime(0)
		, CurrentTime(0)
		, bPause(true)
	{
	}

	FORCEINLINE bool Tick(float DeltaTime)
	{
		if (TargetTime > 0)
		{
			CurrentTime = FMath::Min(CurrentTime + DeltaTime, TargetTime);
			return IsCompleted();
		}
		return false;
	}

	FORCEINLINE bool IsCompleted() const
	{
		return TargetTime > 0 && FMath::IsNearlyEqual(CurrentTime, TargetTime, KINDA_SMALL_NUMBER);
	}
	
	FORCEINLINE bool IsInProgress() const
	{
		return !bPause && CurrentTime < TargetTime;
	}

	FORCEINLINE void Reset()
	{
		CurrentTime = 0;
	}
	
	FORCEINLINE void Reset(float NewTargetTime)
	{
		CurrentTime = 0;
		TargetTime = NewTargetTime;
	}

	FORCEINLINE void SetTargetTime(float NewTargetTime)
	{
		TargetTime = NewTargetTime;
	}

	FORCEINLINE void Pause()
	{
		bPause = true;
	}
	
	FORCEINLINE void Resume()
	{
		bPause = false;
	}
	
	FORCEINLINE bool IsPause() const
	{
		return bPause;
	}

private:

	float TargetTime;

	float CurrentTime;

	bool bPause;
};


struct FIntervalGate
{

public:

	FIntervalGate()
		: Interval(0)
		, Delta(0)
	{
	}
	
	explicit FIntervalGate(float InInterval)
		: Interval(InInterval)
		, Delta(0)
	{
	}

	FORCEINLINE void SetInterval(float InInterval)
	{
		Interval = InInterval;
	}

	FORCEINLINE bool Tick(float DeltaTime, float& OutDelta)
	{
		Delta += DeltaTime;

		if (Delta >= Interval)
		{
			OutDelta = Delta;
			Delta = 0;
			return true;
		}

		return false;
	}

private:

	float Interval;

	float Delta;
};

struct FTimeInterval
{

public:

	FTimeInterval()
		: Interval(0)
		, LastArriveTime(0)
	{
	}
	
	FTimeInterval(float InInterval)
		: Interval(InInterval)
		, LastArriveTime(0)
	{
	}
	
	FORCEINLINE bool IsArrive(float CurrentWorldSeconds)
	{
		if (LastArriveTime + Interval < CurrentWorldSeconds)
		{
			LastArriveTime = CurrentWorldSeconds;
			return true;
		}
		
		return LastArriveTime == 0;
	}

private:

	float Interval;
	
	float LastArriveTime;
};

/*
	按几率随机
*/
struct FRandomProbability
{
public:

	static int32 RandomProbability(const TArray<float>& Probability)
	{
		float RandomValue = FMath::FRandRange(0.f, 1.f);

		int32 Index = INDEX_NONE;

		for (int I = 0; I < Probability.Num(); I++)
		{
			float Min = Sum(Probability, I);
			float Max = Sum(Probability, I + 1);

			if (RandomValue >= Min && RandomValue <= Max)
			{
				Index = I;
				break;
			}
		}

		return Index;
	}
	
	template <typename SourceType>
	static SourceType RandomProbabilityEx(const TMap<SourceType, float>& ProbabilitySource)
	{
		TArray<SourceType> Values;
		TArray<float> Probability;

		GetArrayPair(ProbabilitySource, Values, Probability);

		int32 Index = RandomProbability(Probability);

		if (Index == INDEX_NONE)
		{
			return SourceType();
		}

		return Values[Index];
	}

private:

	template <typename SourceType>
	static void GetArrayPair(const TMap<SourceType, float>& ProbabilitySource, TArray<SourceType>& Sources,
	                         TArray<float>& Probability)
	{
		for (TPair<SourceType, float> Pair : ProbabilitySource)
		{
			Sources.Add(Pair.Key);
			Probability.Add(Pair.Value);
		}
	}

	static float Sum(const TArray<float>& Array, int32 EndIndex, int32 StartIndex = 0)
	{
		float SumResult = 0.f;
		for (int I = StartIndex; I < EndIndex; I++)
		{
			SumResult += Array[I];
		}
		return SumResult;
	}
};

struct FHitResultKeyFuncs : DefaultKeyFuncs<FHitResult>
{
	static FORCEINLINE uint32 GetKeyHash(FHitResult const& Key)
	{
		return Key.Actor.IsValid() ? reinterpret_cast<uint64>(Key.Actor.Get()) : 0;
	}

	static FORCEINLINE bool Matches(FHitResult const& A, FHitResult const& B) { return (A.GetActor() == B.GetActor()); }
};

template <typename ElementType, typename SetKeyFunc = DefaultKeyFuncs<ElementType>>
struct FArrayDistinctIterator;

template <typename ElementType, typename SetKeyFunc>
struct FArrayDistinctIterator
{
	typedef typename TArray<ElementType>::SizeType SizeType;

	explicit FArrayDistinctIterator(const TArray<ElementType>& InArray)
		: Ptr(InArray.GetData()),
		  ArrayNum(InArray.Num()),
		  CurrentIndex(0)
	{
		if (Ptr)
		{
			Exist.Add(*Ptr);
		}
	};

	void operator++()
	{
		bool bIsAlreadyInSet = true;
		do
		{
			++Ptr;
			++CurrentIndex;

			Exist.Add(*Ptr, &bIsAlreadyInSet);
		}
		while (bIsAlreadyInSet && CurrentIndex < ArrayNum);
	}

	FORCEINLINE const ElementType& operator*() const
	{
		return *Ptr;
	}

	FORCEINLINE operator bool() const
	{
		return CurrentIndex < ArrayNum;
	}

	FORCEINLINE const ElementType& operator->() const
	{
		return *Ptr;
	}

	const ElementType* Ptr;

	SizeType ArrayNum;
	SizeType CurrentIndex;

	TSet<ElementType, SetKeyFunc> Exist;
};
