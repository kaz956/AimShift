#include "AchievementsScene.hpp"

AchievementsScene::AchievementsScene(const InitData& init)
	: IScene(init) {}

void AchievementsScene::update() {
	if (s3d::KeyEscape.down() || s3d::KeyEnter.down()) {
		changeScene(SceneID::Home);
	}
}

void AchievementsScene::draw() const {
	const auto& g = getData();
	s3d::Scene::SetBackground(s3d::ColorF(0.05, 0.07, 0.10));

	mTitle(U"Achievements").draw(40, 24, s3d::Palette::White);
	mSmall(U"Each unlocked achievement grants exactly one weapon.").draw(42, 68, s3d::ColorF(0.78, 0.84, 0.94));

	const s3d::RectF left{ 36, 102, 620, 530 };
	const s3d::RectF right{ 670, 102, 294, 530 };
	left.rounded(12).draw(s3d::ColorF(0.10, 0.12, 0.17, 0.92)).drawFrame(1, s3d::ColorF(0.38, 0.45, 0.58, 0.55));
	right.rounded(12).draw(s3d::ColorF(0.10, 0.12, 0.17, 0.92)).drawFrame(1, s3d::ColorF(0.38, 0.45, 0.58, 0.55));

	int y = static_cast<int>(left.y + 18);
	for (size_t i = 0; i < g.achievements.size(); ++i) {
		const auto& a = g.achievements[i];
		const s3d::ColorF row = (i % 2 == 0) ? s3d::ColorF(1, 1, 1, 0.03) : s3d::ColorF(1, 1, 1, 0.0);
		s3d::RectF(left.x + 10, y - 6, left.w - 20, 58).draw(row);
		const s3d::String icon = a.unlocked ? U"[UNLOCKED]" : U"[LOCKED]";
		const s3d::ColorF iconCol = a.unlocked ? s3d::ColorF(0.3, 0.95, 0.65) : s3d::ColorF(0.72, 0.72, 0.78);
		mBody(U"{} {}"_fmt(icon, a.name)).draw(left.x + 20, y, iconCol);
		mSmall((a.unlocked ? U"Condition cleared" : U"Hint: " + a.hint)).draw(left.x + 24, y + 28, s3d::ColorF(0.82, 0.87, 0.95));
		y += 62;
	}

	mBody(U"Weapons").draw(right.x + 16, right.y + 14, s3d::Palette::White);
	int wy = static_cast<int>(right.y + 50);
	for (size_t i = 0; i < g.armory.size(); ++i) {
		const bool unlocked = (i < g.weaponUnlocked.size()) ? g.weaponUnlocked[i] : false;
		const s3d::ColorF col = unlocked ? s3d::ColorF(0.86, 0.95, 1.0) : s3d::ColorF(0.52, 0.55, 0.62);
		const s3d::String suffix = unlocked ? U"" : U" (LOCKED)";
		mSmall(U"{}: {}{}"_fmt(i, g.armory[i].name, suffix)).draw(right.x + 18, wy, col);
		wy += 28;
	}

	mSmall(U"[Enter]/[Esc] Home").draw(42, s3d::Scene::Height() - 28, s3d::ColorF(0.86, 0.9, 0.98));
}
