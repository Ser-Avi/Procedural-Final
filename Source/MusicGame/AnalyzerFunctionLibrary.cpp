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

bool UAnalyzerFunctionLibrary::IsHole(FVector position)
{
    float perlin = perlinNoise3D(position);
    return perlin > 0.5f;
}

FMusicData UAnalyzerFunctionLibrary::GetData(const FString& name)
{
    FString path = "/Game/Songs/" + name + "." + name;
    FMusicData data = MusicAnalyzer::GetMusicData(name);
    data.sound = LoadObject<USoundWave>(nullptr, *path);
    return data;
}
