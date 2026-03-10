#include "ResultScene.hpp"

ResultScene::ResultScene(const InitData& init) : IScene(init) {}

void ResultScene::update() {
	if (!mEvaluated) {
		getData().evaluateRunAchievements();
		mEvaluated = true;
	}
	if (s3d::KeyEnter.down()) {
		changeScene(SceneID::Home);
	}
	if (s3d::KeyA.down()) {
		changeScene(SceneID::Achievements);
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

	const int   misses = hitForbidden; // ミスターゲット撃破のみをミスとして計上
	const double acc = (totalShots > 0 ? (double)hits / totalShots : 0.0);
	const double missRate = (totalShots > 0 ? (double)misses / totalShots : 0.0);
	const double forbAwareRate = (hitForbidden > 0 ? (double)forbAwareHits / hitForbidden : 0.0);

	// 3区分（排他的）
	const int correctShots = hitNormal; // 正解
	const int awareMissShots = forbAwareHits; // 気づいていたミス（警告後にForbiddenへ命中）
	const int missShots = forbUnawareHits; // 気づく前にForbiddenへ命中したミス
	const int judgedShots = s3d::Max(1, correctShots + missShots + awareMissShots);
	const double correctRate = (double)correctShots / judgedShots;
	const double awareMissRate = (double)awareMissShots / judgedShots;
	const double missOnlyRate = (double)missShots / judgedShots;

	// 弾ベース（命中 / 空振り）
	const int hitShots = hits;
	const int missedShots = s3d::Max(0, totalShots - hits);
	const double hitShotRate = (totalShots > 0 ? (double)hitShots / totalShots : 0.0);
	const double missedShotRate = (totalShots > 0 ? (double)missedShots / totalShots : 0.0);

	// ---- 5x5 ヒートマップ集計（発射回数 / 命中回数）----
	constexpr int kGrid = 5;
	s3d::Array<int32> shotCells(kGrid * kGrid, 0);
	s3d::Array<int32> hitCells(kGrid * kGrid, 0);
	for (const auto& s : g.shots) {
		const int gx = s3d::Clamp((int)(s.pos.x / s3d::Scene::Width() * kGrid), 0, kGrid - 1);
		const int gy = s3d::Clamp((int)(s.pos.y / s3d::Scene::Height() * kGrid), 0, kGrid - 1);
		const int idx = gy * kGrid + gx;
		++shotCells[idx];
		if (s.hit) ++hitCells[idx];
	}
	int maxCellShots = 1;
	for (const int v : shotCells) maxCellShots = s3d::Max(maxCellShots, v);

	// ---- 表示 ----
	mTitle(U"RESULT").draw(42, 32, s3d::Palette::White);
	mLabel(U"Round {}   Score {}"_fmt(g.stage, g.score))
		.draw(46, 74, s3d::ColorF(0.82, 0.88, 0.96));

	const s3d::RectF leftPanel{ 36, 108, 520, 536 };
	const s3d::RectF rightPanel{ 572, 108, 392, 536 };
	leftPanel.rounded(14).draw(s3d::ColorF(0.10, 0.12, 0.17, 0.92)).drawFrame(1, s3d::ColorF(0.38, 0.45, 0.58, 0.55));
	rightPanel.rounded(14).draw(s3d::ColorF(0.10, 0.12, 0.17, 0.92)).drawFrame(1, s3d::ColorF(0.38, 0.45, 0.58, 0.55));

	int y = static_cast<int>(leftPanel.y + 24);
	const int xL = static_cast<int>(leftPanel.x + 22);

	mBody(U"SUMMARY").draw(xL, y, s3d::ColorF(0.98, 0.99, 1.0)); y += 36;
	mLabel(U"Shots: {}"_fmt(totalShots)).draw(xL, y); y += 28;
	mLabel(U"Hits: {} (Normal {}, Forbidden {})"_fmt(hits, hitNormal, hitForbidden)).draw(xL, y); y += 28;
	mLabel(U"Accuracy: {:.1f}%"_fmt(acc * 100.0)).draw(xL, y); y += 28;
	mLabel(U"Miss Ratio: {:.1f}% ({} misses)"_fmt(missRate * 100.0, misses)).draw(xL, y); y += 36;

	mBody(U"JUDGED OUTCOME").draw(xL, y, s3d::ColorF(0.98, 0.99, 1.0)); y += 30;
	{
		const int bx = xL;
		const int by = y;
		const int h = 16;
		const double wMax = leftPanel.w - 44.0;
		const double wCorrect = wMax * correctRate;
		const double wMissOnly = wMax * missOnlyRate;
		const double wAwareMiss = wMax * awareMissRate;

		double x = bx;
		s3d::RectF(x, by, wCorrect, h).draw(s3d::ColorF(0.20, 0.85, 0.60, 0.90)); x += wCorrect;
		s3d::RectF(x, by, wMissOnly, h).draw(s3d::ColorF(0.85, 0.25, 0.25, 0.90)); x += wMissOnly;
		s3d::RectF(x, by, wAwareMiss, h).draw(s3d::ColorF(0.95, 0.70, 0.20, 0.90));
		y += 26;
		mLabel(U"Correct {} ({:.1f}%)  Miss {} ({:.1f}%)  Aware {} ({:.1f}%)"_fmt(
			correctShots, correctRate * 100.0, missShots, missOnlyRate * 100.0, awareMissShots, awareMissRate * 100.0
		)).draw(xL, y);
		y += 36;
	}

	mBody(U"HIT VS MISSED BULLETS").draw(xL, y, s3d::ColorF(0.98, 0.99, 1.0)); y += 30;
	{
		const int bx = xL;
		const int by = y;
		const int h = 14;
		const double wMax = leftPanel.w - 44.0;
		const double wHit = wMax * hitShotRate;
		const double wMissed = wMax * missedShotRate;
		s3d::RectF(bx, by, wHit, h).draw(s3d::ColorF(0.25, 0.70, 1.00, 0.90));
		s3d::RectF(bx + wHit, by, wMissed, h).draw(s3d::ColorF(0.45, 0.45, 0.50, 0.85));
		y += 24;
		mLabel(U"Hit {} ({:.1f}%)   Missed {} ({:.1f}%)"_fmt(
			hitShots, hitShotRate * 100.0, missedShots, missedShotRate * 100.0
		)).draw(xL, y);
		y += 34;
	}

	if (hitForbidden > 0) {
		mLabel(U"Forbidden awareness: {:.1f}%  (aware {} / {})"_fmt(
			forbAwareRate * 100.0, forbAwareHits, hitForbidden
		)).draw(xL, y);
	}
	else {
		mLabel(U"Forbidden awareness: N/A").draw(xL, y);
	}

	// ---- 右パネル：ヒートマップ + 軌跡 ----
	mBody(U"HEATMAP (5x5)").draw(rightPanel.x + 18, rightPanel.y + 18, s3d::ColorF(0.98, 0.99, 1.0));
	mSmall(U"Each cell: hit / shot").draw(rightPanel.x + 20, rightPanel.y + 48, s3d::ColorF(0.75, 0.82, 0.94));

	const s3d::RectF heatmapArea{ rightPanel.x + 18, rightPanel.y + 72, rightPanel.w - 36, 268 };
	const double cw = heatmapArea.w / kGrid;
	const double ch = heatmapArea.h / kGrid;
	for (int gy = 0; gy < kGrid; ++gy) {
		for (int gx = 0; gx < kGrid; ++gx) {
			const int idx = gy * kGrid + gx;
			const int sCount = shotCells[idx];
			const int hCount = hitCells[idx];
			const double t = static_cast<double>(sCount) / maxCellShots;
			const s3d::ColorF cold{ 0.16, 0.20, 0.29, 0.86 };
			const s3d::ColorF hot{ 0.95, 0.48, 0.14, 0.90 };
			const s3d::ColorF col{
				cold.r + (hot.r - cold.r) * t,
				cold.g + (hot.g - cold.g) * t,
				cold.b + (hot.b - cold.b) * t,
				cold.a + (hot.a - cold.a) * t
			};
			const s3d::RectF cell{ heatmapArea.x + gx * cw, heatmapArea.y + gy * ch, cw - 2, ch - 2 };
			cell.draw(col).drawFrame(1, s3d::ColorF(0.85, 0.9, 1.0, 0.25));
			mSmall(U"{}/{}"_fmt(hCount, sCount)).drawAt(cell.center(), s3d::Palette::White);
		}
	}

	mBody(U"CURSOR TRAJECTORY").draw(rightPanel.x + 18, rightPanel.y + 354, s3d::ColorF(0.98, 0.99, 1.0));
	const s3d::RectF traceArea{ rightPanel.x + 18, rightPanel.y + 386, rightPanel.w - 36, 136 };
	traceArea.draw(s3d::ColorF(0.07, 0.09, 0.13, 0.92)).drawFrame(1, s3d::ColorF(0.55, 0.62, 0.75, 0.35));

	if (g.cursorTrace.size() >= 2) {
		s3d::LineString ls;
		size_t step = g.cursorTrace.size() / 500;
		if (step == 0) step = 1;
		ls.reserve(g.cursorTrace.size() / step + 2);
		for (size_t i = 0; i < g.cursorTrace.size(); i += step) {
			const auto& p = g.cursorTrace[i].pos;
			const double px = traceArea.x + s3d::Clamp(p.x / s3d::Scene::Width(), 0.0, 1.0) * traceArea.w;
			const double py = traceArea.y + s3d::Clamp(p.y / s3d::Scene::Height(), 0.0, 1.0) * traceArea.h;
			ls << s3d::Vec2{ px, py };
		}
		ls.draw(1.4, s3d::ColorF(0.52, 0.82, 1.0, 0.65));
	}

	for (const auto& s : g.shots) {
		const double px = traceArea.x + s3d::Clamp(s.pos.x / s3d::Scene::Width(), 0.0, 1.0) * traceArea.w;
		const double py = traceArea.y + s3d::Clamp(s.pos.y / s3d::Scene::Height(), 0.0, 1.0) * traceArea.h;
		s3d::Circle(px, py, 2.2).draw(s.hit ? s3d::ColorF(0.2, 1.0, 0.65, 0.9) : s3d::ColorF(1.0, 0.35, 0.35, 0.8));
	}

	mSmall(U"Blue line: cursor path  Green: hit shot  Red: missed shot")
		.draw(rightPanel.x + 20, rightPanel.y + rightPanel.h - 18, s3d::ColorF(0.75, 0.82, 0.94));

	mLabel(U"[Enter] Home   [A] Achievements   [S] Save Shots CSV").draw(46, s3d::Scene::Height() - 34, s3d::ColorF(0.88, 0.93, 1.0));
}
