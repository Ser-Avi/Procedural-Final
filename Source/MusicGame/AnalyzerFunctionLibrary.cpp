// Fill out your copyright notice in the Description page of Project Settings.


#include "AnalyzerFunctionLibrary.h"

FVector VecSin(FVector v)
{
    return FVector(FMath::Sin(v[0]), FMath::Sin(v[1]), FMath::Sin(v[2]));
}

FVector VecFrac(FVector v)
{
    return FVector(FMath::Frac(v[0]), FMath::Frac(v[1]), FMath::Frac(v[2]));
}

FVector VecFloor(FVector v)
{
    return FVector(FMath::Floor(v[0]), FMath::Floor(v[1]), FMath::Floor(v[2]));
}

FVector VecAbs(FVector v)
{
    return FVector(FMath::Abs(v[0]), FMath::Abs(v[1]), FMath::Abs(v[2]));
}

FVector random3(FVector p) {
    return VecFrac(VecSin(FVector(FVector::DotProduct(p, FVector(127.1, 311.7, 253.6f)),
        FVector::DotProduct(p, FVector(269.5, 183.3, 320.6f)),
        FVector::DotProduct(p, FVector(420.6, 631.2, 457.9f))
    )) * 43758.5453f);
}

float Surflet3D(FVector p, FVector gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    FVector t2 = VecAbs(p - gridPoint);
    FVector t = FVector(1.f)
        - 6.f * FVector(FMath::Pow(t2[0], 5.f), FMath::Pow(t2[1], 5.f), FMath::Pow(t2[2], 5.f)) +
        15.f * FVector(FMath::Pow(t2[0], 4.f), FMath::Pow(t2[1], 4.f), FMath::Pow(t2[2], 4.f)) -
        10.f * FVector(FMath::Pow(t2[0], 3.f), FMath::Pow(t2[1], 3.f), FMath::Pow(t2[2], 3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    FVector gradient = (random3(gridPoint) * 2.f - FVector(1.f, 1.f, 1.f));
    // Get the vector from the grid point to P
    FVector diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = FVector::DotProduct(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t[0] * t[1] * t[2];
}

float perlinNoise3D(FVector p) {
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dz = 0; dz <= 1; ++dz) {
                surfletSum += Surflet3D(p, VecFloor(p) + FVector(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}

float WorleyNoise3D(FVector p, int tiles)
{
    p *= tiles;
    // Tile the space
    FVector pointInt = VecFloor(p);
    FVector pointFract = VecFrac(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for (int z = -1; z <= 1; z++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                FVector neighbor = FVector(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                FVector pt = random3(pointInt + neighbor);

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                FVector diff = neighbor + pt - pointFract;
                float dist = FVector::Dist(diff, FVector::ZeroVector);
                minDist = FMath::Min(minDist, dist);
            }
        }
    }
    return minDist;
}

FNoiseResultData UAnalyzerFunctionLibrary::CalculateNoiseResults(FVector position, FMusicData music)
{
    FNoiseResultData data = {};
    float perlin = perlinNoise3D(position);
    //GEngine->AddOnScreenDebugMessage(
    //    -1,                 // Key: A unique identifier for the message. -1 means no key, so it will be a new message each time.
    //    5.0f,               // TimeToDisplay: How long the message will remain on screen (in seconds).
    //    FColor::Red,        // DisplayColor: The color of the text.
    //    FString::SanitizeFloat(perlin) // DebugMessage: The actual text to display. Use TEXT() macro for string literals.
    //);
    // the more danceable, the more holes -> we need do --dance-- jump.
    data.isHole = perlin > 0.5 * (1.3 - music.danceability);
    // Offset is more erratic the louder we are -> the more Worley cells we want
    // for this, we map [0, 1] to [5, 40]
    int loud_tiles = music.loudness * 35 + 5;
    // for the transform offset, we use this worley noise loudness
    // louder music -> more erratic hole placement
    data.transformOffset = FVector(WorleyNoise3D(position, loud_tiles) * 50.0, WorleyNoise3D(random3(position), loud_tiles) * 50.0, 0.0);
    // length of a hole depends on bpm divided by a random beat timing.
    // this makes it predictable in constant tempo songs, but more erratic in others
    data.sizeX = music.bpm / music.beats_diff[int(perlin) % music.beats_diff.Num()];
    // usually tuning frequency is either 435 and 440, so this gets the diff from the mean of that
    float normalized_tuning_diff = abs(music.tuning_frequency - 437.5f);
    // width of a hole depends on bpm dividid by a noise scaled tuning diff
    data.sizeY = music.bpm / (normalized_tuning_diff * perlin);
    return data;
}

FMusicData UAnalyzerFunctionLibrary::GetData(const FString& name)
{
    FString path = "/Game/Songs/" + name + "." + name;
    FMusicData data = MusicAnalyzer::GetMusicData(name);
    data.sound = LoadObject<USoundWave>(nullptr, *path);

    //print loaded data from path
	GEngine->AddOnScreenDebugMessage(
		-1,                 // Key: A unique identifier for the message. -1 means no key, so it will be a new message each time.
		5.0f,               // TimeToDisplay: How long the message will remain on screen (in seconds).
		FColor::Red,        // DisplayColor: The color of the text.
		"Loaded music data from: " + path // DebugMessage: The actual text to display. Use TEXT() macro for string literals.
	);
    
    return data;
}
