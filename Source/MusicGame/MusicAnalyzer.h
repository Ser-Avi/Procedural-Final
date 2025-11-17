// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct MusicData
{
	float length;
	float bpm;
	TArray<float> beats;
	float tuning_frequency;
};

class MUSICGAME_API MusicAnalyzer
{
public:
	MusicAnalyzer();
	~MusicAnalyzer();
	static void Analyze(const FString& filepath, const FString& name);
	static float GetBpm(const FString& name);
	static MusicData GetMusicData(const FString& name);
};
