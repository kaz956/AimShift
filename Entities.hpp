#pragma once
#include <Siv3D.hpp>

struct Difficulty {
	double spawnRate;
	double targetSpeed;
	double targetRadius;
	double forbiddenRatio;
};

inline Difficulty DifficultyForStage(int stage, double skill) {
	const double f = (stage - 1) * 0.08;
	return Difficulty{
		0.8 * (1.0 + f) * skill,           // spawnRate
		80.0 * (1.0 + f * 0.9),             // targetSpeed
		22.0 * s3d::Max(0.7, 1.0 - f * 0.15), // radius
		s3d::Clamp(0.10 + 0.02 * (stage - 1), 0.10, 0.40)
	};
}
