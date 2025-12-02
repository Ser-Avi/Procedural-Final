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

FNoiseResultData UAnalyzerFunctionLibrary::CalculateHole(FVector position, FMusicData music)
{
    FNoiseResultData data = {};
    float perlin = perlinNoise3D(position * 0.01);
    //GEngine->AddOnScreenDebugMessage(
    //    -1,                 // Key: A unique identifier for the message. -1 means no key, so it will be a new message each time.
    //    5.0f,               // TimeToDisplay: How long the message will remain on screen (in seconds).
    //    FColor::Red,        // DisplayColor: The color of the text.
    //    FString::SanitizeFloat(perlin) // DebugMessage: The actual text to display. Use TEXT() macro for string literals.
    //);
    // presuming we start at 0, move down x at 200 velocity.
    float normalizedSongTime = (position[0] / 200.f) / music.length;
    if (position[0] < 1000.f)
    {
        data.isHole = false;
        return data; 
    }
    // the more danceable, the more holes -> we need do --dance-- jump.
    // also, the later in the song, the more holes, so it is harder
    data.isHole = perlin > 0.5 * (1.5 - music.danceability) * (1.0 - normalizedSongTime * 0.8);
    // return early if we are not a hole
    if (!data.isHole)
    {
        return data;
    }
    // Offset is more erratic the louder we are -> the more Worley cells we want
    // for this, we map [0, 1] to [5, 40]
    int loudTiles = music.loudness * 35 + 5;
    // for the transform offset, we use this worley noise loudness
    // louder music -> more erratic hole placement
    FVector offsetDir = random3(position);
    float worley = WorleyNoise3D(position * 0.1, loudTiles);
    data.transformOffset = offsetDir * worley * 50.f;
    // length of a hole depends on bpm divided by a random beat timing.
    // this makes it predictable in constant tempo songs, but more erratic in others
    data.sizeX = music.bpm / music.beats_diff[abs(int(perlin)) % music.beats_diff.Num()];
    // usually tuning frequency is either 435 and 440, so this gets the diff from the mean of that
    float normalizedTuningDiff = abs(music.tuning_frequency - 437.5f);
    // width of a hole depends on bpm dividid by a noise scaled tuning diff
    data.sizeY = music.bpm / (normalizedTuningDiff * perlin) * 3.f;
    return data;
}

FTerrainGenData UAnalyzerFunctionLibrary::CalculateNoiseResults(FVector position, FVector dimension, FMusicData music)
{
    FTerrainGenData outData;

	outData.dimensions.SetNum(4);
	outData.positions.SetNum(4);
    //default initialize dims and pos
	for (int i = 0; i < 4; ++i)
	{
		outData.positions[i] = FVector::ZeroVector;
		outData.dimensions[i] = FVector::ZeroVector;
	}

    FNoiseResultData holeData = CalculateHole(position, music);
    if (!holeData.isHole)
    {
        outData.positions[0] = position;
        outData.dimensions[0] = dimension;
        for (int i = 1; i < 4; ++i)
        {
            outData.positions[i] = FVector::ZeroVector;
            outData.dimensions[i] = FVector::ZeroVector;
        }
        return outData;
    }

	holeData.sizeX = FMath::Min(holeData.sizeX, dimension.X * 0.9f);
	holeData.sizeY = FMath::Min(holeData.sizeY, dimension.Y * 0.9f);

    FVector slabFrontLeftCorner = position + FVector(dimension.X * 0.5f, dimension.Y * -0.5f, 0.0f);
    FVector slabFrontRightCorner = position + FVector(dimension.X * 0.5f, dimension.Y * 0.5f, 0.0f);
    FVector slabBackLeftCorner = position + FVector(dimension.X * -0.5f, dimension.Y * -0.5f, 0.0f);
    FVector slabBackRightCorner = position + FVector(dimension.X * -0.5f, dimension.Y * 0.5f, 0.0f);
    
    FVector holePos = position + holeData.transformOffset;
    FVector holeFrontLeftCorner = holePos + FVector(holeData.sizeX * 0.5f, holeData.sizeY * -0.5f, 0.0f);
    FVector holeFrontRightCorner = holePos + FVector(holeData.sizeX * 0.5f, holeData.sizeY * 0.5f, 0.0f);
    FVector holeBackLeftCorner = holePos + FVector(holeData.sizeX * -0.5f, holeData.sizeY * -0.5f, 0.0f);
    FVector holeBackRightCorner = holePos + FVector(holeData.sizeX * -0.5f, holeData.sizeY * 0.5f, 0.0f);

    //front box
	outData.positions[outData.boxCount] = FVector(
		(slabFrontLeftCorner.X + holeFrontLeftCorner.X) * 0.5f,
		position.Y,
		position.Z
	);
    outData.dimensions[outData.boxCount] = FVector(
        holeFrontLeftCorner.X - slabFrontLeftCorner.X,
        dimension.Y,
        dimension.Z
    );

	outData.boxCount++;

	//right box
	outData.positions[outData.boxCount] = FVector(
		position.X,
		(slabFrontRightCorner.Y + holeFrontRightCorner.Y) * 0.5f + (slabFrontRightCorner.Y - holeFrontRightCorner.Y),
		position.Z
	);
	outData.dimensions[outData.boxCount] = FVector(
		dimension.X,
		-(slabFrontRightCorner.Y - holeFrontRightCorner.Y),
		dimension.Z
	);

	outData.boxCount++;

	//back box
	outData.positions[outData.boxCount] = FVector(
		(slabBackLeftCorner.X + holeBackLeftCorner.X) * 0.5f,
		position.Y,
		position.Z
	);
	outData.dimensions[outData.boxCount] = FVector(
		holeBackLeftCorner.X - slabBackLeftCorner.X,
		dimension.Y,
		dimension.Z
	);

	outData.boxCount++;

	//left box
	outData.positions[outData.boxCount] = FVector(
		position.X,
		(slabFrontLeftCorner.Y + holeFrontLeftCorner.Y) * 0.5f + (slabFrontLeftCorner.Y - holeFrontLeftCorner.Y),
		position.Z
	);

	outData.dimensions[outData.boxCount] = FVector(
		dimension.X,
		-(holeFrontLeftCorner.Y - slabFrontLeftCorner.Y),
		dimension.Z
	);

    //ensure all dimensions are positive and clamped
	for (int i = 0; i < outData.boxCount; ++i)
	{
		outData.dimensions[i] = FVector(
			FMath::Abs(outData.dimensions[i].X),
			FMath::Abs(outData.dimensions[i].Y),
			FMath::Abs(outData.dimensions[i].Z)
		);
		outData.dimensions[i].X = FMath::Clamp(outData.dimensions[i].X, 0.0f, dimension.X);
		outData.dimensions[i].Y = FMath::Clamp(outData.dimensions[i].Y, 0.0f, dimension.Y);
	}

    return outData;
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
