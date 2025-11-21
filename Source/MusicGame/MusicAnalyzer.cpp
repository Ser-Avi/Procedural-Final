// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicAnalyzer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/CoreMisc.h"
#include "Misc/Paths.h"

MusicAnalyzer::MusicAnalyzer()
{
}

MusicAnalyzer::~MusicAnalyzer()
{
}


void MusicAnalyzer::Analyze(const FString& filepath, const FString& name)
{
    FString Path = FPaths::GetPath(FPaths::GetProjectFilePath());
    FString execPath = Path + "/SongData/streaming_extractor_music";

    FString output = Path + "/SongData/" + name + ".json";
    FString spec = Path + "/SongData/profile.yaml";
    FString cmdString = filepath + " " + output + " " + spec;

    FPlatformProcess::CreateProc(
        *execPath,
        *cmdString,
        true,
        false,
        false,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    FString sysString = execPath + " " + cmdString;
	system(TCHAR_TO_ANSI(*sysString));
    GEngine->AddOnScreenDebugMessage(
        -1,                 // Key: A unique identifier for the message. -1 means no key, so it will be a new message each time.
        5.0f,               // TimeToDisplay: How long the message will remain on screen (in seconds).
        FColor::Red,        // DisplayColor: The color of the text.
        TEXT("I did a thinge") // DebugMessage: The actual text to display. Use TEXT() macro for string literals.
    );
}


FMusicData MusicAnalyzer::GetMusicData(const FString& name)
{
    FString Path = FPaths::GetPath(FPaths::GetProjectFilePath());
    FString filePath = Path + "/SongData/" + name + ".json";
    FMusicData data {};

    // Read file content
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *filePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *filePath);
        return data;
    }

    // Parse JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON from file: %s"), *filePath);
        return data;
    }

    // length : metadata -> audio_properties -> length
    const TSharedPtr<FJsonObject>* MetaObject;
    if (JsonObject->TryGetObjectField(TEXT("metadata"), MetaObject))
    {
        const TSharedPtr<FJsonObject>* PropObject;
        if ((*MetaObject)->TryGetObjectField(TEXT("audio_properties"), PropObject))
        {
            float length = 0.0f;
            if ((*PropObject)->TryGetNumberField(TEXT("length"), length))
            {
                UE_LOG(LogTemp, Log, TEXT("Length: %.2f"), length);
                data.length = length;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Length field not found in audio properties section"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio Properties field not found in metadata section"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Metadata field not found in JSON"));
    }

    // BPM: rhythm -> bpm
    // Beats: rhythm -> beats_position
    const TSharedPtr<FJsonObject>* RhythmObject;
    if (JsonObject->TryGetObjectField(TEXT("rhythm"), RhythmObject))
    {
        float BPM = 0.0f;
        if ((*RhythmObject)->TryGetNumberField(TEXT("bpm"), BPM))
        {
            UE_LOG(LogTemp, Log, TEXT("BPM: %.2f"), BPM);
            data.bpm = BPM;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BPM field not found in rhythm section"));
        }

        const TArray<TSharedPtr<FJsonValue>>* BeatsArray;
        if ((*RhythmObject)->TryGetArrayField(TEXT("beats_position"), BeatsArray))
        {
            for (const TSharedPtr<FJsonValue>& BeatValue : *BeatsArray)
            {
                if (BeatValue->Type == EJson::Number)
                {
                    float BeatPosition = static_cast<float>(BeatValue->AsNumber());
                    data.beats.Add(BeatPosition);
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Found %d beat positions"), data.beats.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("beats_position array not found in rhythm section"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Rhythm section not found in JSON"));
    }

    // tuning frequency: tonal -> tuning_frequency
    const TSharedPtr<FJsonObject>* TonalObject;
    if (JsonObject->TryGetObjectField(TEXT("metadata"), TonalObject))
    {
        float freq = 0.0f;
        if ((*TonalObject)->TryGetNumberField(TEXT("tuning_frequency"), freq))
        {
            UE_LOG(LogTemp, Log, TEXT("Tuning Frequency: %.2f"), freq);
            data.tuning_frequency = freq;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Tuning Frequency field not found in tonal section"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tonal field not found in JSON"));
    }

    return data;
}

float MusicAnalyzer::GetBpm(const FString& name) {
    FString Path = FPaths::GetPath(FPaths::GetProjectFilePath());
    FString filePath = Path + "/SongData/" + name + ".json";

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *filePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *filePath);
        return 0.0f;
    }

    // Parse JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON from file: %s"), *filePath);
        return 0.0f;
    }

    // BPM: rhythm -> bpm
    const TSharedPtr<FJsonObject>* RhythmObject;
    if (JsonObject->TryGetObjectField(TEXT("rhythm"), RhythmObject))
    {
        float BPM = 0.0f;
        if ((*RhythmObject)->TryGetNumberField(TEXT("bpm"), BPM))
        {
            UE_LOG(LogTemp, Log, TEXT("BPM: %.2f"), BPM);
            return BPM;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("BPM not found in file"));

    return 0.0;
}