// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnalyzerFunctionLibrary.h"

struct FMusicData;

class MUSICGAME_API MusicAnalyzer
{
public:
	MusicAnalyzer();
	~MusicAnalyzer();
	static void Analyze(const FString& filepath, const FString& name);
	static float GetBpm(const FString& name);
	static FMusicData GetMusicData(const FString& name);
};
