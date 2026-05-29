#include "TutorialScene.hpp"

namespace {
	inline void FillPie(const s3d::Vec2& c, double r,
		double startAngle, double angleLength,
		const s3d::ColorF& col) {
		const int segments = 48;
		for (int i = 0; i < segments; ++i) {
			const double t0 = static_cast<double>(i) / segments;
			const double t1 = static_cast<double>(i + 1) / segments;
			const double a0 = startAngle + angleLength * t0;
			const double a1 = startAngle + angleLength * t1;
			const s3d::Vec2 p0 = c + s3d::Vec2(s3d::Cos(a0), s3d::Sin(a0)) * r;
			const s3d::Vec2 p1 = c + s3d::Vec2(s3d::Cos(a1), s3d::Sin(a1)) * r;
			s3d::Triangle(c, p0, p1).draw(col);
		}
	}

	inline void DrawCuteFace(const s3d::Vec2& c, double r) {
		const double eyeR = r * 0.12;
		const s3d::Vec2 lEye = c + s3d::Vec2(-r * 0.28, -r * 0.12);
		const s3d::Vec2 rEye = c + s3d::Vec2(r * 0.28, -r * 0.12);
		s3d::Circle(lEye, eyeR).draw(s3d::ColorF(0.95, 1.0, 0.95, 0.95));
		s3d::Circle(rEye, eyeR).draw(s3d::ColorF(0.95, 1.0, 0.95, 0.95));
		s3d::Circle(lEye + s3d::Vec2(eyeR * 0.15, eyeR * 0.1), eyeR * 0.45).draw(s3d::ColorF(0.08, 0.22, 0.12, 0.95));
		s3d::Circle(rEye + s3d::Vec2(eyeR * 0.15, eyeR * 0.1), eyeR * 0.45).draw(s3d::ColorF(0.08, 0.22, 0.12, 0.95));
		s3d::LineString smile;
		for (int i = 0; i <= 16; ++i) {
			const double t = i / 16.0;
			const double x = (-r * 0.28) + (r * 0.56) * t;
			const double y = r * 0.26 + s3d::Sin((t - 0.5) * s3d::Math::Pi) * r * 0.07;
			smile << (c + s3d::Vec2(x, y));
		}
		smile.draw(r * 0.08, s3d::ColorF(0.06, 0.24, 0.12, 0.95));
	}

	inline void DrawEvilFace(const s3d::Vec2& c, double r) {
		const double eyeR = r * 0.10;
		s3d::Line(c + s3d::Vec2(-r * 0.42, -r * 0.34), c + s3d::Vec2(-r * 0.16, -r * 0.20))
			.draw(r * 0.09, s3d::ColorF(0.12, 0.0, 0.0, 0.95));
		s3d::Line(c + s3d::Vec2(r * 0.42, -r * 0.34), c + s3d::Vec2(r * 0.16, -r * 0.20))
			.draw(r * 0.09, s3d::ColorF(0.12, 0.0, 0.0, 0.95));
		s3d::Circle(c + s3d::Vec2(-r * 0.27, -r * 0.10), eyeR).draw(s3d::ColorF(1.0, 0.84, 0.72, 0.95));
		s3d::Circle(c + s3d::Vec2(r * 0.27, -r * 0.10), eyeR).draw(s3d::ColorF(1.0, 0.84, 0.72, 0.95));
		s3d::LineString mouth;
		mouth << (c + s3d::Vec2(-r * 0.30, r * 0.24))
			<< (c + s3d::Vec2(-r * 0.18, r * 0.34))
			<< (c + s3d::Vec2(-r * 0.06, r * 0.22))
			<< (c + s3d::Vec2(r * 0.06, r * 0.34))
			<< (c + s3d::Vec2(r * 0.18, r * 0.22))
			<< (c + s3d::Vec2(r * 0.30, r * 0.34));
		mouth.draw(r * 0.08, s3d::ColorF(0.18, 0.0, 0.0, 0.95));
	}
}

TutorialScene::TutorialScene(const InitData& init)
	: IScene(init) {
	mStep = 0;
	mMode = TutorialMode::Explain;
	setupStep();
}

s3d::RectF TutorialScene::playArea() const {
	return { 0.0, 0.0, (double)s3d::Scene::Width(), (double)s3d::Scene::Height() };
}

bool TutorialScene::canAcceptConfirm(double now) const {
	return (now >= mInputUnlockAt);
}

bool TutorialScene::requiresAction(int step) const {
	return (step == 1 || step == 5 || step == 6 || step == 7 || step == 8 || step == 9);
}

void TutorialScene::enterExplain(double now, const s3d::String& text) {
	mMode = TutorialMode::Explain;
	mStepText = text;
	mStepStartedAt = now;
	mInputUnlockAt = now + 2.0;
}

void TutorialScene::enterAction(double now) {
	mMode = TutorialMode::Action;
	mStepStartedAt = now;
}

void TutorialScene::enterAwaitNext(double now, const s3d::String& text) {
	mMode = TutorialMode::AwaitNext;
	mStepText = text;
	mStepStartedAt = now;
	mInputUnlockAt = now + 2.0;
}

void TutorialScene::setupStep() {
	const double now = s3d::Scene::Time();
	mTargets.clear();
	mPendingTargets.clear();
	mBullets.clear();
	mStatus.clear();
	mLifeBeforeStep = mLife;
	mCounterTriggered = false;
	mCounterHalfTriggered = false;
	mLastForbiddenWarned = false;

	DemoTarget t;
	switch (mStep) {
	case 0:
		enterExplain(now, U"このゲームはマウスで狙い、左クリックで撃つゲームです。");
		break;
	case 1:
		enterExplain(now, U"弾道確認: 弾は中央上へ進みます。10発撃つと次へ。");
		mStepShots = 0;
		break;
	case 2:
		enterExplain(now, U"左上は体力ゲージです。赤に当てると減少します。");
		break;
	case 3:
		enterExplain(now, U"中央上は Quota。目標数を達成すると次ラウンドです。");
		break;
	case 4:
		enterExplain(now, U"右上はタイマー。0になる前にQuota達成が必要です。");
		break;
	case 5:
		enterExplain(now, U"緑ターゲット: 通常ターゲットです。倒してください。");
		t.pos = s3d::Scene::CenterF().movedBy(0, -30);
		t.radius = 34.0;
		t.type = TargetType::Normal;
		t.hp = t.maxHP = 1.0;
		t.vel = { 95, 75 };
		mTargets << t;
		break;
	case 6:
		enterExplain(now, U"緑の反撃: 1発当てるとライフ半分。以後は一定周期で反撃します。");
		t.pos = s3d::Scene::CenterF().movedBy(0, -30);
		t.radius = 34.0;
		t.type = TargetType::Normal;
		t.hp = t.maxHP = 2.0;
		t.vel = { 90, 70 };
		mTargets << t;
		break;
	case 7:
		enterExplain(now, U"赤ターゲット: 当てるとHPが減ります。命中後に左上の体力を確認します。");
		t.pos = s3d::Scene::CenterF().movedBy(0, -30);
		t.radius = 30.0;
		t.type = TargetType::Forbidden;
		t.hp = t.maxHP = 1.0;
		t.vel = { 80, 62 };
		mTargets << t;
		break;
	case 8:
		enterExplain(now, U"赤(!): 赤にカーソルを一度重ねると ! が出ます。! 後に撃つとより大きくHPが減ります。");
		t.pos = s3d::Scene::CenterF().movedBy(0, -30);
		t.radius = 30.0;
		t.type = TargetType::Forbidden;
		t.hp = t.maxHP = 1.0;
		t.vel = { 85, 66 };
		mTargets << t;
		break;
	case 9:
		enterExplain(now, U"黄色ターゲット: 高HP・高速。3バウンドで消えるか、倒せば次へ。");
		t.pos = s3d::Scene::CenterF().movedBy(0, -30);
		t.radius = 26.0;
		t.type = TargetType::Normal;
		t.isGolden = true;
		t.hp = t.maxHP = 4.0;
		t.vel = { 280, 210 };
		t.wallBouncesRemaining = 3;
		mTargets << t;
		break;
	default:
		enterAwaitNext(now, U"チュートリアル完了。クリックでホームへ戻ります。");
		break;
	}
}

void TutorialScene::advanceStep(double now) {
	++mStep;
	if (mStep > 10) {
		mStep = 10;
	}
	setupStep();
	mStepStartedAt = now;
}

bool TutorialScene::hasAliveTarget(TargetType type) const {
	for (const auto& t : mTargets) {
		if (t.alive && t.type == type) return true;
	}
	return false;
}

bool TutorialScene::hasAliveGreen() const {
	for (const auto& t : mTargets) {
		if (t.alive && t.type == TargetType::Normal && !t.isGolden) return true;
	}
	return false;
}

bool TutorialScene::hasAliveYellow() const {
	for (const auto& t : mTargets) {
		if (t.alive && t.isGolden) return true;
	}
	return false;
}

void TutorialScene::updateTargets(double now) {
	const s3d::RectF area = playArea();
	for (auto& t : mTargets) if (t.alive) {
		t.pos += t.vel * s3d::Scene::DeltaTime();
		const bool hitX = (t.pos.x < area.x + t.radius || t.pos.x > area.x + area.w - t.radius);
		const bool hitY = (t.pos.y < area.y + t.radius || t.pos.y > area.y + area.h - t.radius);
		if (hitX || hitY) {
			if (t.wallBouncesRemaining == 0) {
				t.alive = false;
				continue;
			}
			if (hitX) t.vel.x *= -1;
			if (hitY) t.vel.y *= -1;
			if (t.wallBouncesRemaining > 0) {
				--t.wallBouncesRemaining;
				if (t.isGolden && t.wallBouncesRemaining == 0) {
					t.alive = false;
					continue;
				}
			}
		}
		t.pos.x = s3d::Clamp(t.pos.x, area.x + t.radius, area.x + area.w - t.radius);
		t.pos.y = s3d::Clamp(t.pos.y, area.y + t.radius, area.y + area.h - t.radius);

		if (t.type == TargetType::Forbidden) {
			const bool hover = s3d::Circle(t.pos, t.radius).intersects(s3d::Cursor::PosF());
			if (hover) t.hoverMs += s3d::Scene::DeltaTime() * 1000.0;
			else t.hoverMs = s3d::Max(0.0, t.hoverMs - 2.0);
			if (!t.warnShownAt && hover) t.warnShownAt = now;
		}

		if (mStep == 6 && mMode == TutorialMode::Action && t.type == TargetType::Normal && !t.isGolden
			&& mCounterHalfTriggered && t.hp > 0.0 && now >= t.nextActionAt) {
			spawnGreenCounter(t.pos, now);
			t.nextActionAt = now + 1.2;
		}
	}
}

void TutorialScene::spawnGreenCounter(const s3d::Vec2& center, double now) {
	for (int i = 0; i < 4; ++i) {
		const double ang = s3d::Math::TwoPi * (i / 4.0) + s3d::Random(-0.18, 0.18);
		DemoTarget r;
		r.type = TargetType::Forbidden;
		r.pos = center + s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * 8.0;
		r.vel = s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * 210.0;
		r.radius = 16.0;
		r.fromCounter = true;
		r.wallBouncesRemaining = 1;
		r.expireAt = now + 7.0;
		mPendingTargets << r;
	}
}

void TutorialScene::processShoot(double now) {
	if (!s3d::MouseL.pressed()) return;
	const double interval = (mDemoFireRate > 0.0) ? (1.0 / mDemoFireRate) : 1e9;
	if (now - mLastShotAt < interval) return;
	mLastShotAt = now;
	++mStepShots;

	Bullet b;
	b.pos = s3d::Cursor::PosF();
	b.dir2D = (vanishPoint() - b.pos).setLength(1.0);
	b.speed2D = mDemoBulletSpeed2D;
	b.z = 0.0;
	b.zVel = mDemoBulletZVel;
	b.live = true;
	mBullets << b;
}

void TutorialScene::updateBullets() {
	const double now = s3d::Scene::Time();
	bool beginGreenCounterExplain = false;
	for (auto& b : mBullets) if (b.live) {
		const double dt = s3d::Scene::DeltaTime();
		b.pos += b.dir2D * b.speed2D * dt;
		b.z += b.zVel * dt;
		if (b.z > mDemoBulletMaxZ) { b.live = false; continue; }

		const s3d::Vec2 sp = projectToScreen(b.pos, b.z);
		if (!playArea().intersects(sp)) {
			b.live = false;
			continue;
		}
		if (b.z > mDemoBulletHitMaxZ) continue;

		for (auto& t : mTargets) if (t.alive) {
			if ((sp - t.pos).length() > t.radius * 0.7) continue;
			b.live = false;

			if (t.type == TargetType::Forbidden) {
				mLastForbiddenWarned = t.warnShownAt.has_value();
				mLife -= (mLastForbiddenWarned ? 2.0 : 1.0);
				mLife = s3d::Max(0.0, mLife);
				t.alive = false;
				break;
			}

			const double before = t.hp;
			t.hp -= 1.0;

			if (mStep == 6 && !mCounterHalfTriggered && before == t.maxHP && t.hp < t.maxHP) {
				mCounterHalfTriggered = true;
				mLife = 4.0;
				t.nextActionAt = now + 0.6;
				beginGreenCounterExplain = true;
			}

			if (t.hp <= 0.0) {
				t.alive = false;
				if (!t.isGolden) ++mKills;
			}
			break;
		}
	}
	mBullets.remove_if([](const Bullet& b) { return !b.live; });
	if (beginGreenCounterExplain) {
		enterExplain(now, U"ライフを半分に設定。ここから緑は一定周期で反撃します。");
	}
}

s3d::Vec2 TutorialScene::vanishPoint() const {
	return { s3d::Scene::CenterF().x, 220.0 };
}

s3d::Vec2 TutorialScene::projectToScreen(const s3d::Vec2& p, double z) const {
	const s3d::Vec2 v = vanishPoint();
	const double k = 0.003;
	const double s = 1.0 / (1.0 + k * z);
	return v + (p - v) * s;
}

void TutorialScene::drawBullets() const {
	for (const auto& b : mBullets) if (b.live) {
		const s3d::Vec2 sp = projectToScreen(b.pos, b.z);
		const double s = 1.0 / (1.0 + 0.003 * b.z);
		const double r = s3d::Clamp(6.0 * s, 2.0, 6.0);
		const double a = s3d::Clamp(0.2 + 0.8 * s, 0.15, 1.0);
		s3d::Circle(sp + s3d::Vec2(0, 6 * (1.0 - s)), r * 0.9).draw(s3d::ColorF(0, 0, 0, 0.12 * a));
		s3d::Circle(sp, r).draw(s3d::ColorF(1, 1, 1, a));
	}
}

void TutorialScene::update() {
	const double now = s3d::Scene::Time();
	const bool confirmPressed = (s3d::KeyEnter.down() || s3d::MouseL.down());

	if (mMode == TutorialMode::Explain) {
		if (canAcceptConfirm(now) && confirmPressed) {
			if (requiresAction(mStep)) enterAction(now);
			else advanceStep(now);
		}
	}
	else if (mMode == TutorialMode::AwaitNext) {
		if (canAcceptConfirm(now) && confirmPressed) {
			if (mStep >= 10) changeScene(SceneID::Home);
			else advanceStep(now);
		}
	}
	else {
		updateTargets(now);
		processShoot(now);
		updateBullets();
		if (!mPendingTargets.isEmpty()) {
			mTargets.insert(mTargets.end(), mPendingTargets.begin(), mPendingTargets.end());
			mPendingTargets.clear();
		}

		switch (mStep) {
		case 1:
			if (mStepShots >= 10) enterAwaitNext(now, U"10発完了。弾が中央方向へ進むことを確認しました。クリックで次へ。");
			break;
		case 5:
			if (!hasAliveGreen() && !hasAliveTarget(TargetType::Forbidden) && mBullets.isEmpty()) {
				enterAwaitNext(now, U"緑ターゲットの撃破を確認。クリックで次へ。");
			}
			break;
		case 6:
			if (mCounterHalfTriggered && !hasAliveGreen() && !hasAliveTarget(TargetType::Forbidden) && mBullets.isEmpty()) {
				enterAwaitNext(now, U"緑の反撃フェーズ完了。クリックで次へ。");
			}
			break;
		case 7:
			if (mLife < mLifeBeforeStep && !hasAliveTarget(TargetType::Forbidden) && mBullets.isEmpty()) {
				enterAwaitNext(now, U"赤ターゲット命中でHPが減少したことを確認。クリックで次へ。");
			}
			break;
		case 8:
			if (mLife < mLifeBeforeStep && mLastForbiddenWarned && !hasAliveTarget(TargetType::Forbidden) && mBullets.isEmpty()) {
				enterAwaitNext(now, U"赤(!)命中で大きくHP減少したことを確認。クリックで次へ。");
			}
			else if (!hasAliveTarget(TargetType::Forbidden) && !mLastForbiddenWarned && mBullets.isEmpty()) {
				DemoTarget t;
				t.pos = s3d::Scene::CenterF().movedBy(0, -30);
				t.radius = 30.0;
				t.type = TargetType::Forbidden;
				t.hp = t.maxHP = 1.0;
				t.vel = { 85, 66 };
				mPendingTargets << t;
				enterExplain(now, U"! は赤にカーソルを一度重ねると表示されます。! 表示後に撃ってください。");
			}
			break;
		case 9:
			if (!hasAliveYellow() && mBullets.isEmpty()) {
				enterAwaitNext(now, U"黄色ターゲットフェーズ完了。クリックで次へ。");
			}
			break;
		default:
			break;
		}
	}

	if (s3d::KeyEscape.down()) {
		changeScene(SceneID::Home);
	}
}

void TutorialScene::drawTarget(const DemoTarget& t) const {
	if (!t.alive) return;
	if (t.type == TargetType::Forbidden) {
		s3d::Circle(t.pos, t.radius).draw(s3d::ColorF(0.20, 0.05, 0.05));
		s3d::Circle(t.pos, t.radius).drawFrame(3, s3d::Palette::Crimson);
		DrawEvilFace(t.pos, t.radius);
		if (t.warnShownAt) mBody(U"!").drawAt(t.pos, s3d::Palette::Crimson);
		return;
	}
	if (t.isGolden) {
		s3d::Circle(t.pos, t.radius).draw(s3d::ColorF(0.36, 0.26, 0.03));
		s3d::Circle(t.pos, t.radius).drawFrame(3, s3d::ColorF(1.0, 0.88, 0.20));
		s3d::Circle(t.pos + s3d::Vec2(0, -t.radius * 0.14), t.radius * 0.10).draw(s3d::ColorF(1.0, 0.95, 0.6));
		s3d::Line(t.pos + s3d::Vec2(-t.radius * 0.24, t.radius * 0.20), t.pos + s3d::Vec2(t.radius * 0.24, t.radius * 0.20))
			.draw(2.6, s3d::ColorF(1.0, 0.95, 0.7));
		const double rate = s3d::Clamp(t.hp / s3d::Max(1.0, t.maxHP), 0.0, 1.0);
		FillPie(t.pos, t.radius - 3.0, -s3d::Math::HalfPi, s3d::Math::TwoPi * rate, s3d::ColorF(1.0, 0.90, 0.35, 0.55));
		return;
	}
	s3d::Circle(t.pos, t.radius).draw(s3d::ColorF(0.06, 0.22, 0.13));
	s3d::Circle(t.pos, t.radius).drawFrame(2, s3d::Palette::Limegreen);
	DrawCuteFace(t.pos, t.radius);
	const double rate = s3d::Clamp(t.hp / s3d::Max(1.0, t.maxHP), 0.0, 1.0);
	FillPie(t.pos, t.radius - 3.0, -s3d::Math::HalfPi, s3d::Math::TwoPi * rate, s3d::ColorF(0.25, 0.95, 0.6, 0.55));
}

s3d::RectF TutorialScene::currentHighlightRect() const {
	const s3d::RectF area = playArea();
	if ((mStep == 7 || mStep == 8) && (mMode != TutorialMode::Action)) {
		return s3d::RectF(20, 24, 120, 120);
	}
	switch (mStep) {
	case 1: return s3d::RectF(area.center().x - 180, area.center().y - 60, 360, 160);
	case 2: return s3d::RectF(20, 24, 120, 120);
	case 3: return s3d::RectF(s3d::Scene::CenterF().x - 150, 6, 300, 48);
	case 4: return s3d::RectF(s3d::Scene::Width() - 300, 14, 280, 56);
	default:
		for (const auto& t : mTargets) if (t.alive) {
			return s3d::RectF(t.pos.x - t.radius - 18, t.pos.y - t.radius - 18, (t.radius + 18) * 2, (t.radius + 18) * 2);
		}
		return s3d::RectF(area.center().x - 160, area.center().y - 60, 320, 120);
	}
}

s3d::String TutorialScene::currentOverlayText() const {
	return mStepText;
}

void TutorialScene::drawGuideOverlay() const {
	if (mMode == TutorialMode::Action) return;
	const s3d::RectF hi = currentHighlightRect();
	s3d::RectF{ 0, 0, (double)s3d::Scene::Width(), (double)s3d::Scene::Height() }.draw(s3d::ColorF(0, 0, 0, 0.58));
	hi.drawFrame(4, s3d::ColorF(1.0, 0.95, 0.55, 0.95));

	const s3d::String text = currentOverlayText();
	const double boxW = 560.0;
	const double boxH = 48.0;
	const double margin = 20.0;
	double boxX = hi.center().x - (boxW * 0.5);
	boxX = s3d::Clamp(boxX, margin, (double)s3d::Scene::Width() - boxW - margin);
	double boxY = hi.y + hi.h + 14.0;
	if ((boxY + boxH) > ((double)s3d::Scene::Height() - margin)) {
		boxY = hi.y - boxH - 14.0;
	}
	boxY = s3d::Clamp(boxY, margin, (double)s3d::Scene::Height() - boxH - margin);
	const s3d::RectF box{ boxX, boxY, boxW, boxH };
	box.rounded(8).draw(s3d::ColorF(0, 0, 0, 0.78)).drawFrame(1, s3d::ColorF(1, 1, 1, 0.35));
	mSmall(text).drawAt(box.center(), s3d::Palette::White);
}

void TutorialScene::draw() const {
	const double now = s3d::Scene::Time();
	const s3d::RectF area = playArea();
	s3d::Scene::SetBackground(s3d::ColorF(0.04, 0.06, 0.09));

	for (int i = 0; i < 14; ++i) {
		const double r = 120 + i * 48 + s3d::Sin(now * 0.8 + i * 0.35) * 8;
		s3d::Circle(vanishPoint(), r).drawFrame(2, s3d::ColorF(0.11, 0.15, 0.23, 0.06));
	}

	area.draw(s3d::ColorF(0.04, 0.06, 0.10, 0.14))
		.drawFrame(2, s3d::ColorF(0.55, 0.62, 0.75, 0.35));

	for (const auto& t : mTargets) {
		drawTarget(t);
	}
	drawBullets();

	const double lifeRate = s3d::Clamp(mLife / 8.0, 0.0, 1.0);
	const s3d::Vec2 lifeC{ 76, 80 };
	s3d::Circle(lifeC, 44).draw(s3d::ColorF(0, 0, 0, 0.35));
	s3d::Circle(lifeC, 44).drawFrame(3, s3d::ColorF(0.2, 0.9, 0.6, 0.9));
	s3d::LineString ring;
	for (int i = 0; i <= 48; ++i) {
		const double t = i / 48.0;
		const double ang = -s3d::Math::HalfPi + (s3d::Math::TwoPi * lifeRate * t);
		ring << (lifeC + s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * 34.0);
	}
	ring.draw(10.0, s3d::ColorF(0.2, 0.9, 0.6, 0.35));
	mBody(U"{:.1f}"_fmt(mLife)).drawAt(lifeC, s3d::Palette::White);

	mBody(U"Quota: {}/{}"_fmt(mKills, mQuota)).drawAt({ s3d::Scene::CenterF().x, 26.0 }, s3d::Palette::White);

	const double remain = s3d::Max(0.0, mTimeLimit - now);
	const double rate = s3d::Clamp(remain / mTimeLimit, 0.0, 1.0);
	const double w = 260.0, h = 16.0;
	const double x = s3d::Scene::Width() - w - 24.0;
	const double y = 22.0;
	s3d::RectF(x, y, w, h).draw(s3d::ColorF(0, 0, 0, 0.30)).drawFrame(2, s3d::ColorF(0.9, 0.9, 1.0, 0.7));
	s3d::RectF(x, y, w * rate, h).draw(s3d::ColorF(0.25, 0.6, 1.0, 0.55));
	mBody(U"TIME {:02d}s"_fmt((int)s3d::Floor(remain + 0.5))).drawAt({ x + w * 0.5, y + h + 14.0 }, s3d::Palette::White);

	drawGuideOverlay();
}
