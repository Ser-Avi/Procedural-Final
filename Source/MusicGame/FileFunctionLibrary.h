// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FileFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MUSICGAME_API UFileFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FileFunctions")
	static FString AddSongToGame(FString songName, FString sourcePath);
	
};
