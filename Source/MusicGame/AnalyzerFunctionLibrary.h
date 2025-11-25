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
	float loudness;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float length;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float bpm;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	TArray<float> beats;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	TArray<float> beats_diff;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float tuning_frequency;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	float danceability;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MusicData")
	USoundWave* sound;
};

USTRUCT(BlueprintType)
struct FNoiseResultData
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NoiseData")
	bool isHole;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NoiseData")
	float sizeX;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NoiseData")
	float sizeY;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NoiseData")
	FVector transformOffset;

};

/**
 *
 */
UCLASS()
class MUSICGAME_API UAnalyzerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable)
	static FNoiseResultData CalculateNoiseResults(FVector position, FMusicData music);
	UFUNCTION(BlueprintCallable, Category = "MusicData")
	static FMusicData GetData(const FString& name);
};