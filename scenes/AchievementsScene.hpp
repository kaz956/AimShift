#pragma once
#include <Siv3D.hpp>
#include "../GameData.hpp"

class AchievementsScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;
	AchievementsScene(const InitData& init);
	void update() override;
	void draw() const override;

private:
	s3d::Font mTitle{ 34, s3d::Typeface::Bold };
	s3d::Font mBody{ 20 };
	s3d::Font mSmall{ 16 };
};
