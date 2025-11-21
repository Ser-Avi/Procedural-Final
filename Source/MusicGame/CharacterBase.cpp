// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "MusicAnalyzer.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	FString Path = FPaths::GetPath(FPaths::GetProjectFilePath());
	FString songPath = Path + "/Content/Songs/WeWillRockYou.wav";
	FString name = "WeWillRockYou";
	//MusicAnalyzer::Analyze(songPath,name);
	bpm = MusicAnalyzer::GetBpm(name);
	const FString bpmString = FString::SanitizeFloat(bpm);
	GEngine->AddOnScreenDebugMessage(
		-1,                 // Key: A unique identifier for the message. -1 means no key, so it will be a new message each time.
		5.0f,               // TimeToDisplay: How long the message will remain on screen (in seconds).
		FColor::Red,        // DisplayColor: The color of the text.
		bpmString // DebugMessage: The actual text to display. Use TEXT() macro for string literals.
	);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

