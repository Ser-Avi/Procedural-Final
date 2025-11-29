// Fill out your copyright notice in the Description page of Project Settings.


#include "FileFunctionLibrary.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/SoundFactory.h"
#include "MusicAnalyzer.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"

FString UFileFunctionLibrary::AddSongToGame(FString songName, FString sourcePath)
{
	//validate file exists
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.FileExists(*sourcePath))
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: File not found: %s at %s"), *songName, *sourcePath);
		return FString("Error: File does not exist at the specified path.");
	}

	//validate its a valid file type (wav, mp3)
	FString fileExtension = FPaths::GetExtension(sourcePath).ToLower();
	if (fileExtension != "wav" && fileExtension != "mp3")
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Invalid file type. Only WAV and MP3 files are supported."));
		return FString("Error: Invalid file type. Only WAV and MP3 files are supported.");
	}

    // Get Asset Tools - THIS IS CRITICAL
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    // Create Sound Factory - THIS CREATES THE UASSET
    USoundFactory* SoundFactory = NewObject<USoundFactory>();

    // Define where to put the asset in Content Browser
    FString PackagePath = TEXT("/Game/Songs/");

    // Import the file - THIS CREATES THE .uasset
    TArray<FString> FilesToImport;
    FilesToImport.Add(sourcePath);

    TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssets(FilesToImport, PackagePath, SoundFactory);

    if (ImportedObjects.Num() == 0)
    {
		UE_LOG(LogTemp, Error, TEXT("Import failed - no UAsset created."));
        return FString("Error: Import failed - no UAsset created.");
    }

    // Get the imported SoundWave UAsset
    USoundWave* SoundWave = Cast<USoundWave>(ImportedObjects[0]);
    if (!SoundWave)
    {
		UE_LOG(LogTemp, Error, TEXT("Imported object is not a SoundWave."));
		return FString("Error: Imported object is not a SoundWave.");
    }

	// Rename if needed
	if (SoundWave->GetName() != songName)
	{
		TArray<FAssetRenameData> RenameData;
		FAssetRenameData RenameInfo;
		RenameInfo.Asset = SoundWave;
		RenameInfo.NewPackagePath = PackagePath;
		RenameInfo.NewName = songName;
		RenameData.Add(RenameInfo);

		AssetToolsModule.Get().RenameAssets(RenameData);
	}

	// Mark as dirty and notify
	SoundWave->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(SoundWave);

	UE_LOG(LogTemp, Log, TEXT("Successfully created UAsset: %s"), *SoundWave->GetName());



	//copy file to game directory
	//FString destinationPath = FPaths::ProjectContentDir() + "Songs/" + songName + "." + fileExtension;
	//FileManager.Copy(*destinationPath, *sourcePath);

	//run music analyzer
	MusicAnalyzer::Analyze(sourcePath, songName);

	//return
	UE_LOG(LogTemp, Log, TEXT("Song %s added to the game"), *songName);
	return FString("Success: Song added to the game.");
}

TArray<FString> UFileFunctionLibrary::GetSongNames()
{
	TArray<FString> names{};
	FString path = FPaths::Combine(FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("SongData"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*path))
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Directory not found: %s"), *path);
		return names;
	}

	TArray<FString> FoundFiles;

	// Find all files in the directory
	PlatformFile.FindFiles(FoundFiles, *FPaths::ConvertRelativePathToFull(path), TEXT(".json"));

	for (const FString& FilePath : FoundFiles)
	{
		FString name = FPaths::GetBaseFilename(FilePath);
		names.Add(name);
	}

	return names;
}
