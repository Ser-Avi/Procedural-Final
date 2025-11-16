// Fill out your copyright notice in the Description page of Project Settings.


#include "FileFunctionLibrary.h"

#include "MusicAnalyzer.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"

FString UFileFunctionLibrary::AddSongToGame(FString songName, FString sourcePath)
{
	//validate file exists
	IFileManager& FileManager = IFileManager::Get();
	bool fileExists = FileManager.FileExists(*sourcePath);
	if (!fileExists)
	{
		return FString("Error: File does not exist at the specified path.");
	}

	//validate its a valid file type (wav, mp3)
	FString fileExtension = FPaths::GetExtension(sourcePath).ToLower();
	if (fileExtension != "wav" && fileExtension != "mp3")
	{
		return FString("Error: Invalid file type. Only WAV and MP3 files are supported.");
	}

	//copy file to game directory
	FString destinationPath = FPaths::ProjectContentDir() + "Songs/" + songName + "." + fileExtension;
	FileManager.Copy(*destinationPath, *sourcePath);

	//run music analyzer
	MusicAnalyzer::Analyze(destinationPath, songName);

	//return
	return FString("Success: Song added to the game.");

}
