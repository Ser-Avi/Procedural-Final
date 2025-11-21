// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MusicAnalyzer.h"
#include "Sound/SoundWave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnalyzerFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FMusicData
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float length;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float bpm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	TArray<float> beats;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float tuning_frequency;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	USoundWave* sound;
};

/**
 *
 */
UCLASS()
class MUSICGAME_API UAnalyzerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable)
	static bool IsHole(FVector position);
	UFUNCTION(BlueprintCallable, Category = "MusicData")
	static FMusicData GetData(const FString& name);
};