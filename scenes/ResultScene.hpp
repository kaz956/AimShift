#pragma once
#include <Siv3D.hpp>
#include "../GameData.hpp"
#include "../Util.hpp"

class ResultScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;
	ResultScene(const InitData& init);
	void update() override;
	void draw() const override;

private:
	// W100 対策：フォントを一度だけ生成
	s3d::Font mTitle{ 36, Typeface::Bold };
	s3d::Font mBody{ 22 };

	// %表示の小ヘルパ
	static s3d::String fmtPercent(double x) {
		return U"{:.1f}%"_fmt(x * 100.0);
	}
};
