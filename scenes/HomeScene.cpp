#include "HomeScene.hpp"

HomeScene::HomeScene(const InitData& init)
	: IScene(init)
	, mTitle(36, s3d::Typeface::Bold)   // ← s3d:: を明示
	, mFont(22)
{
}

void HomeScene::update() {
	auto& g = getData();

	// 装備切り替え（← / →）
	if (s3d::KeyLeft.down()) {
		g.equipped = (g.equipped + (int)g.armory.size() - 1) % (int)g.armory.size();
	}
	if (s3d::KeyRight.down()) {
		g.equipped = (g.equipped + 1) % (int)g.armory.size();
	}

	// ゲーム開始（Enter）→ まずキャリブレーションへ
	if (s3d::KeyEnter.down()) {
		g.initGazeProvider();   // Webカメラが使えれば利用、無ければマウスにフォールバック
		g.resetRun();           // ステージ/スコア/タイマ等を初期化
		changeScene(SceneID::Calibration);  // ★ Home → Calibration →（完了後に）Run
		return;
	}

	if (g.gazeProvider && !g.gazeProvider->available()) {
		s3d::Console.open();
		s3d::Console << U"[Gaze] Webcam not available. Using mouse fallback.";
	}

}

void HomeScene::draw() const {
	const auto& g = getData();
	s3d::Scene::SetBackground(s3d::ColorF(0.05, 0.07, 0.10));

	mTitle(U"AimShift").drawAt(s3d::Scene::CenterF().movedBy(0, -140), s3d::Palette::White);

	// 現在の武器
	const Weapon& w = g.armory[g.equipped];

	// 旧名(projectileSpeed / spread)ではなく、新名(projectileSpeedZ / speed2D / spreadDeg)を表示
	mFont(
		U"Equipped: {}  RoF:{:.1f}/s  DMG:{:.1f}  ZSPD:{:.0f}  2D:{:.0f}  Spread:{:.1f}°"_fmt(
			w.name, w.fireRate, w.damage, w.projectileSpeedZ, w.speed2D, w.spreadDeg
		)
	).drawAt(s3d::Scene::CenterF().movedBy(0, -80));

	// 操作説明
	mFont(U"[←][→] Change Weapon   [Enter] Start").drawAt(s3d::Scene::CenterF().movedBy(0, 40));
	mFont(U"Gaze Input: {}"_fmt(g.usingWebcam ? U"Webcam" : U"Mouse fallback"))
		.drawAt(s3d::Scene::CenterF().movedBy(0, 72), s3d::Palette::Orange);

	// 武器リスト
	const double x0 = s3d::Scene::CenterF().x - 240;
	double y = s3d::Scene::CenterF().y + 120;
	for (size_t i = 0; i < g.armory.size(); ++i) {
		const bool sel = (static_cast<int>(i) == g.equipped);
		const s3d::ColorF col = sel ? s3d::ColorF(0.9, 1.0, 0.95) : s3d::ColorF(0.7);
		mFont(U"{}: {}"_fmt(i, g.armory[i].name)).draw(x0, y, col);
		y += 26;
	}
}
