#include "ResultScene.hpp"

ResultScene::ResultScene(const InitData& init) : IScene(init) {}

void ResultScene::update() {
	if (s3d::KeyEnter.down()) {
		changeScene(SceneID::Home);
	}
	if (s3d::KeyS.down()) {
		// 発射ログを CSV に保存（存在する場合）
		SaveShotsCSV(getData().shots);
	}
}

void ResultScene::draw() const {
	const auto& g = getData();
	s3d::Scene::SetBackground(s3d::ColorF(0.05, 0.07, 0.10));

	// ---- ショット集計 ----
	const int totalShots = static_cast<int>(g.shots.size());
	int hits = 0;
	int hitNormal = 0;
	int hitForbidden = 0;

	// Forbidden への命中時：警告に「気づいていた（= 警告表示後に撃った）」内訳
	int forbAwareHits = 0;  // warnShownAt 後に当てた
	int forbUnawareHits = 0;  // warnShownAt 前に当てた

	for (const auto& s : g.shots) {
		if (s.hit) {
			++hits;
			if (s.hitType == TargetType::Forbidden) {
				++hitForbidden;
				if (s.forbiddenPreAware) ++forbAwareHits;
				else                     ++forbUnawareHits;
			}
			else {
				++hitNormal;
			}
		}
	}

	const int   misses = s3d::Max(0, totalShots - hits);
	const double acc = (totalShots > 0 ? (double)hits / totalShots : 0.0);
	const double missRate = 1.0 - acc;
	const double forbAwareRate = (hitForbidden > 0 ? (double)forbAwareHits / hitForbidden : 0.0);

	// ---- 表示 ----
	mTitle(U"Result").drawAt(s3d::Scene::CenterF().x, 80, s3d::Palette::White);

	const int xL = 80;
	int y = 160;

	mBody(U"Stage Reached : {}"_fmt(g.stage)).draw(xL, y);           y += 36;
	mBody(U"Score         : {}"_fmt(g.score)).draw(xL, y);           y += 36;

	// 命中/ミス
	mBody(U"Shots         : {}"_fmt(totalShots)).draw(xL, y);        y += 28;
	mBody(U"Hits          : {} (Normal {}, Forbidden {})"_fmt(hits, hitNormal, hitForbidden)).draw(xL, y); y += 28;
	mBody(U"Accuracy      : {:.1f}%”"_fmt(acc * 100.0)).draw(xL, y); y += 28;
	mBody(U"Miss Ratio    : {:.1f}%  ({} misses)"_fmt(missRate * 100.0, misses)).draw(xL, y); y += 36;

	// Forbidden で「気づいていたか」内訳
	if (hitForbidden > 0) {
		mBody(U"Forbidden awareness : {:.1f}%  (aware {} / {} forbidden hits)"_fmt(forbAwareRate * 100.0, forbAwareHits, hitForbidden)).draw(xL, y); y += 28;
		mBody(U"  └ aware(after warning)   : {}"_fmt(forbAwareHits)).draw(xL + 24, y); y += 24;
		mBody(U"  └ unaware(before warning): {}"_fmt(forbUnawareHits)).draw(xL + 24, y); y += 36;
	}
	else {
		mBody(U"Forbidden awareness : N/A").draw(xL, y); y += 36;
	}

	// 簡易バー（命中/ミス）
	{
		const double wMax = 320.0;
		const double wHit = wMax * acc;
		const double wMiss = wMax - wHit;
		const int bx = xL, by = y + 8, h = 16;
		s3d::RectF(bx, by, wHit, h).draw(s3d::ColorF(0.20, 0.85, 0.60, 0.8));
		s3d::RectF(bx + wHit, by, wMiss, h).draw(s3d::ColorF(0.85, 0.25, 0.25, 0.8));
		y += 36;
	}

	mBody(U"[Enter] Home  /  [S] Save Shots CSV").draw(xL, s3d::Scene::Height() - 60);
}
