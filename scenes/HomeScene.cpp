#include "HomeScene.hpp"
#include <algorithm>

HomeScene::HomeScene(const InitData& init)
	: IScene(init)
	, mTitle(36, s3d::Typeface::Bold)   // ← s3d:: を明示
	, mFont(22)
{
}

void HomeScene::update() {
	auto& g = getData();
	auto moveEquip = [&](int delta) {
		if (g.armory.isEmpty()) return;
		int next = g.equipped;
		for (int i = 0; i < static_cast<int>(g.armory.size()); ++i) {
			next = (next + delta + static_cast<int>(g.armory.size())) % static_cast<int>(g.armory.size());
			if (next < static_cast<int>(g.weaponUnlocked.size()) && g.weaponUnlocked[next]) {
				g.equipped = next;
				return;
			}
		}
	};

	if (!(g.equipped < static_cast<int>(g.weaponUnlocked.size()) && g.weaponUnlocked[g.equipped])) {
		for (int i = 0; i < static_cast<int>(g.weaponUnlocked.size()); ++i) {
			if (g.weaponUnlocked[i]) {
				g.equipped = i;
				break;
			}
		}
	}

	// 装備切り替え（← / →）
	if (s3d::KeyLeft.down()) {
		moveEquip(-1);
	}
	if (s3d::KeyRight.down()) {
		moveEquip(+1);
	}

	if (s3d::KeyA.down()) {
		changeScene(SceneID::Achievements);
		return;
	}
	if (s3d::KeyT.down()) {
		changeScene(SceneID::Tutorial);
		return;
	}

	// 武器リストスクロール
	const int prevEquipped = g.equipped;
	const int maxRow = std::max(0, static_cast<int>(g.armory.size()) - cVisibleRows);
	const double wheel = s3d::Mouse::Wheel();
	if (wheel > 0.0) {
		mWeaponListRow = std::max(0, mWeaponListRow - 1);
	}
	if (wheel < 0.0) {
		mWeaponListRow = std::min(maxRow, mWeaponListRow + 1);
	}
	if (s3d::KeyUp.down() || s3d::KeyW.down()) {
		mWeaponListRow = std::max(0, mWeaponListRow - 1);
	}
	if (s3d::KeyDown.down() || s3d::KeyS.down()) {
		mWeaponListRow = std::min(maxRow, mWeaponListRow + 1);
	}
	// Equip selection moved; keep the selected weapon visible.
	if (g.equipped != prevEquipped) {
		if (g.equipped < mWeaponListRow) {
			mWeaponListRow = g.equipped;
		}
		if (g.equipped >= mWeaponListRow + cVisibleRows) {
			mWeaponListRow = g.equipped - cVisibleRows + 1;
		}
	}
	mWeaponListRow = s3d::Clamp(mWeaponListRow, 0, maxRow);

	// ゲーム開始（Enter）→ まずキャリブレーションへ
	if (s3d::KeyEnter.down()) {
		g.initGazeProvider();   // Webカメラが使えれば利用、無ければマウスにフォールバック
		g.resetRun();           // ステージ/スコア/タイマ等を初期化
		changeScene(SceneID::Run);  // ★ Home → Run
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
	mFont(U"[←][→] Weapon  [Enter] Start  [T] Tutorial  [A] Achievements").drawAt(s3d::Scene::CenterF().movedBy(0, 40));
	mFont(U"Gaze Input: {}"_fmt(g.usingWebcam ? U"Webcam" : U"Mouse fallback"))
		.drawAt(s3d::Scene::CenterF().movedBy(0, 72), s3d::Palette::Orange);

	// 武器リスト
	const auto listRect = weaponListArea();
	listRect.drawFrame(2.0, 0.0, s3d::ColorF(1.0, 0.2));
	const double listHeight = static_cast<double>(g.armory.size()) * cWeaponItemHeight;

	mHint(U"Mouse wheel or ↑↓ to scroll").drawAt(listRect.center().x, listRect.y - 20, s3d::Palette::Lightgray);

	double y = listRect.y;
	for (int row = 0; row < cVisibleRows; ++row) {
		const int i = mWeaponListRow + row;
		if (i >= static_cast<int>(g.armory.size())) {
			y += cWeaponItemHeight;
			continue;
		}

		const bool sel = (i == g.equipped);
		const bool unlocked = (i < static_cast<int>(g.weaponUnlocked.size())) ? g.weaponUnlocked[i] : false;
		const s3d::ColorF col = unlocked
			? (sel ? s3d::ColorF(0.9, 1.0, 0.95) : s3d::ColorF(0.7))
			: s3d::ColorF(0.45, 0.45, 0.5);
		const s3d::String tag = unlocked ? U"" : U" [LOCKED]";
		// Ensure text fits inside the list rectangle (leave room for padding and scrollbar)
		const double scrollbarW = (listHeight > listRect.h) ? 20.0 : 0.0;
		// small extra safety margin to account for glyph overhang/antialiasing
		const double safetyMargin = 6.0;
		const double availableW = listRect.w - 24.0 - scrollbarW - safetyMargin;
		s3d::String text = U"{}: {}{}"_fmt(i, g.armory[i].name, tag);
		// If text is too wide, use binary search to find the longest prefix that fits with ellipsis.
		if (mFont(text).region().w > availableW) {
			const s3d::String full = text;
			const size_t n = full.size();
			size_t lo = 0, hi = n;
			auto fits = [&](size_t len) {
				if (len == 0) return mFont(U"...").region().w <= availableW;
				s3d::String t = full.substr(0, static_cast<int>(len)) + U"...";
				return mFont(t).region().w <= availableW;
			};
			while (lo < hi) {
				size_t mid = (lo + hi + 1) / 2;
				if (fits(mid)) lo = mid; else hi = mid - 1;
			}
			if (lo > 0) {
				text = full.substr(0, static_cast<int>(lo)) + U"...";
			} else {
				text = U"...";
			}
		}
		mFont(text).draw(listRect.x + 12, y + 4, col);
		y += cWeaponItemHeight;
	}

	if (listHeight > listRect.h) {
		const int maxRow = std::max(0, static_cast<int>(g.armory.size()) - cVisibleRows);
		const double thumbHeight = std::max(24.0, listRect.h * listRect.h / listHeight);
		const double trackX = listRect.x + listRect.w - 12;
		const double thumbY = listRect.y + (maxRow > 0 ? (static_cast<double>(mWeaponListRow) / maxRow) * (listRect.h - thumbHeight) : 0.0);
		s3d::RectF{ trackX, listRect.y, 6.0, listRect.h }.draw(s3d::ColorF(1.0, 0.08));
		s3d::RectF{ trackX, thumbY, 6.0, thumbHeight }.draw(s3d::ColorF(1.0, 0.45));
	}
}
