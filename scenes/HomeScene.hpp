#pragma once
#include <Siv3D.hpp>
#include "../GameData.hpp"

class HomeScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;
	HomeScene(const InitData& init);
	void update() override;
	void draw() const override;

private:
	// ---- UI assets（毎フレ生成しない：W100回避） ----
	s3d::Font mTitle{ 54 };
	s3d::Font mBig{ 32 };
	s3d::Font mLabel{ 20 };
	s3d::Font mSmall{ 18 };
	s3d::Font mHint{ 16 };
	s3d::Font mBtn{ 28 };
	s3d::Font mNote{ 16 };
	s3d::Font mFont{ 28 };

	mutable s3d::String mHover;

	// ---- layout ----
	s3d::RectF mBtnStart{ s3d::Scene::CenterF().x - 160, 360, 320, 56 };
	s3d::RectF mBtnQuit{ s3d::Scene::CenterF().x - 160, 430, 320, 52 };
	s3d::RectF mChkCursor{ 60, 330, 260, 32 };
	s3d::RectF mChkGaze{ 60, 372, 260, 32 };

	// ---- weapon list scrolling ----
	int mWeaponListRow = 0;
	static constexpr int cVisibleRows = 7;
	static constexpr double cWeaponItemHeight = 26.0;
	static constexpr double cWeaponListHeight = cVisibleRows * cWeaponItemHeight + 8.0; // slight extra bottom padding

	s3d::RectF weaponListArea() const {
		return s3d::RectF{ s3d::Scene::CenterF().x - 240, s3d::Scene::CenterF().y + 120, 480, cWeaponListHeight };
	}

	// ---- helpers ----
	void drawButton(const s3d::RectF& r, const s3d::String& text) const;
	void drawToggle(const s3d::RectF& r, const s3d::String& label, bool on) const;
};
