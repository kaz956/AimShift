#include "RunScene.hpp"

// ===== ローカルヘルパ（Siv3Dバージョン非依存の描画関数など） =====
namespace {
	inline double LerpD(double a, double b, double t) { return a + (b - a) * t; }

	// 厚み付き円弧（円ゲージ用）
	inline void AS_DrawArcLine(const s3d::Vec2& c, double r,
							   double startAngle, double angleLength,
							   double thickness, const s3d::ColorF& col)
	{
		const int N = 48;
		s3d::LineString ls; ls.reserve(N + 1);
		for (int i = 0; i <= N; ++i) {
			const double t = static_cast<double>(i) / N;
			const double ang = startAngle + angleLength * t;
			ls << (c + s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * r);
		}
		ls.draw(thickness, col);
	}

	// 扇形塗り（緑ターゲットHPの円グラフ）
	inline void AS_FillPie(const s3d::Vec2& c, double r,
						   double startAngle, double angleLength,
						   const s3d::ColorF& col)
	{
		const int N = 48;
		for (int i = 0; i < N; ++i) {
			const double t0 = static_cast<double>(i) / N;
			const double t1 = static_cast<double>(i + 1) / N;
			const double a0 = startAngle + angleLength * t0;
			const double a1 = startAngle + angleLength * t1;
			const s3d::Vec2 p0 = c + s3d::Vec2(s3d::Cos(a0), s3d::Sin(a0)) * r;
			const s3d::Vec2 p1 = c + s3d::Vec2(s3d::Cos(a1), s3d::Sin(a1)) * r;
			s3d::Triangle(c, p0, p1).draw(col);
		}
	}

	inline void AS_DrawCuteFace(const s3d::Vec2& c, double r) {
		const double eyeR = r * 0.12;
		const s3d::Vec2 lEye = c + s3d::Vec2(-r * 0.28, -r * 0.12);
		const s3d::Vec2 rEye = c + s3d::Vec2(r * 0.28, -r * 0.12);
		const s3d::Vec2 cheekL = c + s3d::Vec2(-r * 0.38, r * 0.14);
		const s3d::Vec2 cheekR = c + s3d::Vec2(r * 0.38, r * 0.14);

		s3d::Circle(lEye, eyeR).draw(s3d::ColorF(0.95, 1.0, 0.95, 0.95));
		s3d::Circle(rEye, eyeR).draw(s3d::ColorF(0.95, 1.0, 0.95, 0.95));
		s3d::Circle(lEye + s3d::Vec2(eyeR * 0.15, eyeR * 0.1), eyeR * 0.45).draw(s3d::ColorF(0.08, 0.22, 0.12, 0.95));
		s3d::Circle(rEye + s3d::Vec2(eyeR * 0.15, eyeR * 0.1), eyeR * 0.45).draw(s3d::ColorF(0.08, 0.22, 0.12, 0.95));

		s3d::Circle(cheekL, eyeR * 0.75).draw(s3d::ColorF(1.0, 0.68, 0.75, 0.42));
		s3d::Circle(cheekR, eyeR * 0.75).draw(s3d::ColorF(1.0, 0.68, 0.75, 0.42));

		s3d::LineString smile;
		const int N = 18;
		smile.reserve(N + 1);
		for (int i = 0; i <= N; ++i) {
			const double t = static_cast<double>(i) / N;
			const double x = (-r * 0.28) + (r * 0.56) * t;
			const double y = r * 0.26 + s3d::Sin((t - 0.5) * s3d::Math::Pi) * r * 0.07;
			smile << (c + s3d::Vec2(x, y));
		}
		smile.draw(r * 0.08, s3d::ColorF(0.06, 0.24, 0.12, 0.95));
	}

	inline void AS_DrawEvilFace(const s3d::Vec2& c, double r) {
		const double eyeR = r * 0.10;
		const s3d::Vec2 lEye = c + s3d::Vec2(-r * 0.27, -r * 0.10);
		const s3d::Vec2 rEye = c + s3d::Vec2(r * 0.27, -r * 0.10);

		// angry eyebrows
		s3d::Line(c + s3d::Vec2(-r * 0.42, -r * 0.34), c + s3d::Vec2(-r * 0.16, -r * 0.20))
			.draw(r * 0.09, s3d::ColorF(0.12, 0.0, 0.0, 0.95));
		s3d::Line(c + s3d::Vec2(r * 0.42, -r * 0.34), c + s3d::Vec2(r * 0.16, -r * 0.20))
			.draw(r * 0.09, s3d::ColorF(0.12, 0.0, 0.0, 0.95));

		s3d::Circle(lEye, eyeR).draw(s3d::ColorF(1.0, 0.84, 0.72, 0.95));
		s3d::Circle(rEye, eyeR).draw(s3d::ColorF(1.0, 0.84, 0.72, 0.95));
		s3d::Circle(lEye + s3d::Vec2(0, eyeR * 0.1), eyeR * 0.48).draw(s3d::ColorF(0.30, 0.0, 0.0, 0.95));
		s3d::Circle(rEye + s3d::Vec2(0, eyeR * 0.1), eyeR * 0.48).draw(s3d::ColorF(0.30, 0.0, 0.0, 0.95));

		// jagged mouth
		s3d::LineString mouth;
		mouth << (c + s3d::Vec2(-r * 0.30, r * 0.24));
		mouth << (c + s3d::Vec2(-r * 0.18, r * 0.34));
		mouth << (c + s3d::Vec2(-r * 0.06, r * 0.22));
		mouth << (c + s3d::Vec2(r * 0.06, r * 0.34));
		mouth << (c + s3d::Vec2(r * 0.18, r * 0.22));
		mouth << (c + s3d::Vec2(r * 0.30, r * 0.34));
		mouth.draw(r * 0.08, s3d::ColorF(0.18, 0.0, 0.0, 0.95));
	}
}

RunScene::RunScene(const InitData& init) : IScene(init) {
	auto& g = getData();
	m_stageTimeLimit = stageTimeLimitForStage(g.stage);
	m_roundQuota = quotaForStage(g.stage);
	m_roundKills = 0;
	m_bossSpawnedThisRound = false;
}

void RunScene::update() {
	auto& g = getData();
	g.stageTimer.start(); // キャリブレーション中は停止、Runで開始
	const double t = s3d::Scene::Time();

	// カーソルトレース
	g.cursorTrace << CursorSample{ t, s3d::Cursor::PosF() };

	const auto d = DifficultyForStage(g.stage, g.playerSkill);
	spawnTargets(d, t);
	updateTargets(d, t);
	shootIfReady(t);
	updateBullets(t);
	updateEffects();

	// フレーム終端で安全に追加
	if (!m_pendingTargets.isEmpty()) {
		g.targets.insert(g.targets.end(), m_pendingTargets.begin(), m_pendingTargets.end());
		m_pendingTargets.clear();
	}

	// —— ラウンド進行：ライフ切れ > ノルマ達成 > 時間切れ未達 —— //
	const double elapsed = g.stageTimer.s();
	const double remain = s3d::Max(0.0, m_stageTimeLimit - elapsed);
	const bool lifeOut = (g.life <= 0.0);

	if (lifeOut) {
		changeScene(SceneID::Result);
		return;
	}

	// ノルマ達成：残り時間を繰り越して即次ラウンドへ
	if (m_roundKills >= m_roundQuota) {
		g.stage += 1;
		m_roundQuota = quotaForStage(g.stage);
		m_roundKills = 0;
		m_bossSpawnedThisRound = false;

		m_stageTimeLimit = stageTimeLimitForStage(g.stage) + remain; // 繰り越し
		g.stageTimer.restart();

		g.targets.clear();
		g.bullets.clear();
		return;
	}

	// 時間切れ：未達ならゲームオーバー
	if (elapsed >= m_stageTimeLimit) {
		changeScene(SceneID::Result);
		return;
	}
}

void RunScene::draw() const {
	const auto& g = getData();
	s3d::Scene::SetBackground(s3d::ColorF(0.05, 0.07, 0.10));

	const s3d::Vec2 vanish = vanishPoint();
	drawWormhole(vanish);
	drawGameFrame();

	// 擬似投影
	auto projectToScreen = [&](const s3d::Vec2& p0, double z)->s3d::Vec2 {
		const double k = 0.003;
		const double s = 1.0 / (1.0 + k * z);
		return vanish + (p0 - vanish) * s;
		};

	// 1) 遠い弾（当たり判定オフ）→ 背面に描く
	for (const auto& b : g.bullets) if (b.live && b.z > kBulletHitMaxZ) {
		const s3d::Vec2 sp = projectToScreen(b.pos, b.z);
		const double k = 0.003;
		const double s = 1.0 / (1.0 + k * b.z);
		const double r = s3d::Clamp(6.0 * s, 2.0, 6.0);
		const double a = s3d::Clamp(0.2 + 0.8 * s, 0.15, 1.0);
		s3d::Circle(sp + s3d::Vec2(0, 6 * (1.0 - s)), r * 0.9).draw(s3d::ColorF(0, 0, 0, 0.15 * a));
		s3d::Circle(sp, r).draw(s3d::ColorF(1, 1, 1, a));
	}

	// 2) ターゲット
	for (const auto& tg : g.targets) if (tg.alive) {
		if (tg.type == TargetType::Forbidden) {
			s3d::Circle(tg.pos, tg.radius).draw(s3d::ColorF(0.20, 0.05, 0.05));
			s3d::Circle(tg.pos, tg.radius).drawFrame(3, s3d::Palette::Crimson);
			AS_DrawEvilFace(tg.pos, tg.radius);
			if (tg.warnShownAt.has_value()) mWarn(U"!").drawAt(tg.pos, s3d::Palette::Crimson);
		}
		else {
			if (tg.isGolden) {
				s3d::Circle(tg.pos, tg.radius).draw(s3d::ColorF(0.36, 0.26, 0.03));
				s3d::Circle(tg.pos, tg.radius).drawFrame(3, s3d::ColorF(1.0, 0.88, 0.20));
				s3d::Circle(tg.pos + s3d::Vec2(0, -tg.radius * 0.15), tg.radius * 0.10).draw(s3d::ColorF(1.0, 0.95, 0.6));
				s3d::Line(tg.pos + s3d::Vec2(-tg.radius * 0.22, tg.radius * 0.20), tg.pos + s3d::Vec2(tg.radius * 0.22, tg.radius * 0.20))
					.draw(2.6, s3d::ColorF(1.0, 0.95, 0.7));
			}
			else if (tg.isBoss) {
				const s3d::ColorF fill = tg.enraged ? s3d::ColorF(0.22, 0.09, 0.09) : s3d::ColorF(0.08, 0.20, 0.26);
				const s3d::ColorF frame = tg.enraged ? s3d::ColorF(1.0, 0.30, 0.30) : s3d::ColorF(0.45, 0.9, 1.0);
				s3d::Circle(tg.pos, tg.radius).draw(fill);
				s3d::Circle(tg.pos, tg.radius).drawFrame(4, frame);
				AS_DrawEvilFace(tg.pos, tg.radius * 0.95);
				mHUD(U"BOSS").drawAt(tg.pos + s3d::Vec2(0, -tg.radius - 14), s3d::ColorF(1.0, 0.85, 0.4));
			}
			else {
				s3d::Circle(tg.pos, tg.radius).draw(s3d::ColorF(0.06, 0.22, 0.13));
				s3d::Circle(tg.pos, tg.radius).drawFrame(2, s3d::Palette::Limegreen);
				AS_DrawCuteFace(tg.pos, tg.radius);
			}
			const double rate = s3d::Clamp(tg.hp / s3d::Max(1.0, tg.maxHP), 0.0, 1.0);
			const double ang = s3d::Math::TwoPi * rate;
			AS_FillPie(tg.pos, tg.radius - 3, -s3d::Math::HalfPi, ang, s3d::ColorF(0.25, 0.95, 0.6, 0.55));
		}
	}

	// 3) 近い弾（当たり判定オン）→ 手前に描く
	for (const auto& b : g.bullets) if (b.live && b.z <= kBulletHitMaxZ) {
		const s3d::Vec2 sp = projectToScreen(b.pos, b.z);
		const double k = 0.003;
		const double s = 1.0 / (1.0 + k * b.z);
		const double r = s3d::Clamp(6.0 * s, 2.0, 6.0);
		const double a = s3d::Clamp(0.2 + 0.8 * s, 0.15, 1.0);
		s3d::Circle(sp + s3d::Vec2(0, 6 * (1.0 - s)), r * 0.9).draw(s3d::ColorF(0, 0, 0, 0.15 * a));
		s3d::Circle(sp, r).draw(s3d::ColorF(1, 1, 1, a));
	}

	// エフェクト
	drawEffects();

	// UI
	drawLifeMeter();
	drawRoundTimerAndQuota(); // 右上に時間、上部中央にQuota
	mHUD(U"Round: {}   Score: {}"_fmt(g.stage, g.score)).draw(20, 22 + 88);

	if (s3d::KeyEscape.down()) {
		const_cast<RunScene*>(this)->changeScene(SceneID::Result);
	}
}

void RunScene::drawGameFrame() const {
	const s3d::RectF area = playArea();
	area.draw(s3d::ColorF(0.04, 0.06, 0.10, 0.14))
		.drawFrame(2, s3d::ColorF(0.55, 0.62, 0.75, 0.35));

	const s3d::RectF header{ area.x, area.y, area.w, 34.0 };
	header.draw(s3d::ColorF(0.0, 0.0, 0.0, 0.20));

	const s3d::RectF hudBase{ 0.0, area.h, (double)s3d::Scene::Width(), 140.0 };
	hudBase.draw(s3d::ColorF(0.08, 0.10, 0.14, 0.96))
		.drawFrame(1, 0, s3d::ColorF(0.35, 0.43, 0.56, 0.55));

	mHUD(U"HP System (No ammo limit)").draw(24, area.h + 12, s3d::ColorF(0.86, 0.92, 1.0));
	mHUD(U"HP down: hit red target  (! shown: -2.0 / before !: -1.0)")
		.draw(24, area.h + 40, s3d::ColorF(1.0, 0.86, 0.84));
	mHUD(U"Next round: Quota reached.  Game over: HP 0 or TIME 0 before quota.")
		.draw(24, area.h + 68, s3d::ColorF(0.82, 0.90, 1.0));
	mHUD(U"Green counter: on hit -> red burst / damaged -> periodic burst / red bullets bounce once")
		.draw(24, area.h + 96, s3d::ColorF(1.0, 0.86, 0.84));
}

// ===== 背景（ワームホール） =====
void RunScene::drawWormhole(const s3d::Vec2& vanish) const {
	const double t = s3d::Scene::Time();
	const double rotAngle = 0.25 * t;

	for (int i = 0; i < 34; ++i) {
		const double s = i / 33.0;
		const double baseR = 48 + i * 30;
		const double wiggle = 7.0 * s3d::Sin(2.4 * s + 3.0 * t);
		const double r = baseR + wiggle;
		const double alpha = 0.095 * (1.0 - s) * (1.0 - s);
		s3d::Circle(vanish, r).drawFrame(10, s3d::ColorF(0.10, 0.14, 0.22, alpha));
	}
	for (int k = 0; k < 40; ++k) {
		const double ang = (s3d::Math::TwoPi * k / 40.0) + 0.60 * t + rotAngle;
		const s3d::Vec2 dir = { s3d::Cos(ang), s3d::Sin(ang) };
		const s3d::Vec2 a = vanish + dir * 40;
		const s3d::Vec2 b = vanish + dir * 680;
		s3d::Line(a, b).draw(1.0, s3d::ColorF(0.08, 0.12, 0.18, 0.10));
	}
	s3d::Circle(vanish, 42).draw(s3d::ColorF(0.05, 0.08, 0.14, 0.6));
	s3d::Circle(vanish, 22).draw(s3d::ColorF(0.15, 0.25, 0.35, 0.45));
}

// ===== ライフ（円メーター） =====
void RunScene::drawLifeMeter() const {
	const auto& g = getData();
	const double cx = 76.0, cy = 80.0, R = 44.0;
	const double rate = s3d::Clamp(g.life / kMaxLife, 0.0, 1.0);

	s3d::Circle(cx, cy, R).draw(s3d::ColorF(0, 0, 0, 0.35));
	s3d::Circle(cx, cy, R).drawFrame(3, s3d::ColorF(0.2, 0.9, 0.6, 0.9));
	const double theta = s3d::Math::TwoPi * rate;
	AS_DrawArcLine({ cx, cy }, R - 6, -s3d::Math::HalfPi, theta, 12.0, s3d::ColorF(0.2, 0.9, 0.6, 0.35));
	mHUD(U"{:.1f}"_fmt(g.life)).drawAt({ cx, cy }, s3d::Palette::White);
}

// ===== 残り時間 & ノルマ（上部中央） =====
void RunScene::drawRoundTimerAndQuota() const {
	const auto& g = getData();
	const double elapsed = g.stageTimer.s();
	const double remain = s3d::Max(0.0, m_stageTimeLimit - elapsed);
	const double rate = s3d::Clamp(remain / m_stageTimeLimit, 0.0, 1.0);

	// 時間バー（右上）
	const double w = 260.0, h = 16.0;
	const double x = s3d::Scene::Width() - w - 24.0;
	const double y = 22.0;
	s3d::RectF(x, y, w, h).draw(s3d::ColorF(0, 0, 0, 0.30)).drawFrame(2, s3d::ColorF(0.9, 0.9, 1.0, 0.7));
	s3d::RectF(x, y, w * rate, h).draw(s3d::ColorF(0.25, 0.6, 1.0, 0.55));
	const int sec = static_cast<int>(s3d::Floor(remain + 0.5));
	mHUD(U"TIME {:02d}s"_fmt(sec)).drawAt({ x + w * 0.5, y + h + 14.0 }, s3d::Palette::White);

	// ノルマ（上部中央）
	mHUD(U"Quota: {}/{}"_fmt(m_roundKills, m_roundQuota)).drawAt({ s3d::Scene::CenterF().x, 26.0 }, s3d::Palette::White);
}

// ===== エフェクト更新・描画 =====
void RunScene::updateEffects() {
	const double dt = s3d::Scene::DeltaTime();
	const double now = s3d::Scene::Time();
	for (auto& s : m_sparks) if (now - s.bornT <= s.life) { s.pos += s.vel * dt; s.vel *= 0.92; }
	{ s3d::Array<SparkFX> next; for (const auto& s : m_sparks) if (now - s.bornT <= s.life) next << s; m_sparks.swap(next); }
	{ s3d::Array<RingFX>  next; for (const auto& r : m_rings)  if (now - r.bornT <= r.life)  next << r; m_rings.swap(next); }
}
void RunScene::drawEffects() const {
	const double now = s3d::Scene::Time();
	for (const auto& r : m_rings) {
		const double age = s3d::Clamp((now - r.bornT) / r.life, 0.0, 1.0);
		const double radius = LerpD(6.0, 64.0, age);
		const double alpha = (1.0 - age) * 0.6;
		s3d::Circle(r.pos, radius).drawFrame(4, s3d::ColorF(1.0, 1.0, 1.0, alpha));
	}
	for (const auto& s : m_sparks) {
		const double age = s3d::Clamp((now - s.bornT) / s.life, 0.0, 1.0);
		s3d::Circle(s.pos, 2.2).draw(s3d::ColorF(1.0, 0.95, 0.85, (1.0 - age) * 0.9));
	}
}

// ===== スポーン =====
void RunScene::spawnTargets(const Difficulty& d, double now) {
	auto& g = getData();
	const s3d::RectF area = playArea();
	const bool isBossStage = (g.stage % 5 == 0);
	bool bossAlive = false;
	for (const auto& tg : g.targets) {
		if (tg.alive && tg.isBoss) {
			bossAlive = true;
			break;
		}
	}
	if (isBossStage && !m_bossSpawnedThisRound && !bossAlive) {
		spawnBoss(g.stage, now);
		return;
	}
	if (isBossStage && bossAlive) {
		return;
	}

	m_spawnAccum += s3d::Scene::DeltaTime();
	if (m_spawnAccum * d.spawnRate >= 1.0) {
		m_spawnAccum = 0.0;
		const int n = s3d::Random(1, 2);
		for (int i = 0; i < n; ++i) {
			Target t;
			t.pos = {
				s3d::Random(area.x + 40.0, area.x + area.w - 40.0),
				s3d::Random(area.y + 40.0, area.y + area.h - 40.0)
			};
			const double angle = s3d::Random(0.0, s3d::Math::TwoPi);
			t.vel = s3d::Vec2(s3d::Cos(angle), s3d::Sin(angle)) * d.targetSpeed;

			t.radius = d.targetRadius;
			t.type = (s3d::Random(0.0, 1.0) < d.forbiddenRatio ? TargetType::Forbidden : TargetType::Normal);
			t.alive = true;
			t.wallBouncesRemaining = -1;
			t.isBoss = false;
			t.bossKind = BossKind::None;
			t.enraged = false;
			t.isGolden = false;
			t.nextActionAt = s3d::Math::Inf;
			t.spawnTime = now;
			t.warnShownAt = s3d::none;
			t.hoverMs = 0.0;

			if (t.type == TargetType::Normal) {
				t.maxHP = t.hp = 1.0 + 0.2 * (g.stage - 1);
				t.nextBurstAt = s3d::Math::Inf; // 未ダメージ中は周期発射なし
			}
			else {
				t.maxHP = t.hp = 1.0;
			}

			// Stage 5 以降で超低確率の黄色ターゲット
			if (g.stage >= 5 && t.type == TargetType::Normal && s3d::Random(0.0, 1.0) < 0.004) {
				t.isGolden = true;
				t.maxHP = t.hp = 1.0;
				t.radius = d.targetRadius * 0.92;
			}

			++g.runSpawnTotal;
			if (t.type == TargetType::Forbidden) ++g.runSpawnForbidden;
			if (t.isGolden) ++g.runGoldenSpawns;
			g.targets << t;
		}
	}
}

// ===== ターゲット更新（視線＋カーソルで「！」） =====
void RunScene::updateTargets(const Difficulty& /*d*/, double t) {
	auto& g = getData();
	const s3d::RectF area = playArea();

	const s3d::Vec2 gazeScreen = (g.gaze.valid ? g.gaze.map(g.readRawGaze01()) : s3d::Vec2(-1000, -1000));
	const s3d::Vec2 cursor = s3d::Cursor::PosF();

	for (auto& tg : g.targets) if (tg.alive) {
		// 位置とバウンス
		tg.pos += tg.vel * s3d::Scene::DeltaTime();
		const bool hitX = (tg.pos.x < area.x + tg.radius || tg.pos.x > area.x + area.w - tg.radius);
		const bool hitY = (tg.pos.y < area.y + tg.radius || tg.pos.y > area.y + area.h - tg.radius);
		if (hitX || hitY) {
			if (tg.wallBouncesRemaining == 0) {
				tg.alive = false;
				continue;
			}
			if (hitX) tg.vel.x *= -1;
			if (hitY) tg.vel.y *= -1;
			if (tg.wallBouncesRemaining > 0) {
				--tg.wallBouncesRemaining;
			}
		}
		tg.pos.x = s3d::Clamp(tg.pos.x, area.x + tg.radius, area.x + area.w - tg.radius);
		tg.pos.y = s3d::Clamp(tg.pos.y, area.y + tg.radius, area.y + area.h - tg.radius);

		// ボス専用挙動
		if (tg.isBoss) {
			if (!tg.enraged && tg.hp <= tg.maxHP * 0.5) {
				tg.enraged = true;
				tg.vel *= 1.35;
				tg.nextActionAt = s3d::Min(tg.nextActionAt, t + 0.5);
			}
			if (t >= tg.nextActionAt) {
				const double mult = tg.enraged ? 1.45 : 1.0;
				switch (tg.bossKind) {
				case BossKind::Speed: {
					const double ang = s3d::Random(0.0, s3d::Math::TwoPi);
					tg.vel = s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * (180.0 * mult + 45.0);
					if (tg.enraged) {
						emitForbiddenBurst(tg.pos, 4 + g.stage / 3, 200.0 + 30.0 * mult);
					}
					break;
				}
				case BossKind::Arsenal:
					emitForbiddenBurst(tg.pos, 6 + g.stage / 2, 210.0 + 18.0 * g.stage * mult);
					break;
				case BossKind::Warp:
					tg.pos = {
						s3d::Random(area.x + tg.radius + 40.0, area.x + area.w - tg.radius - 40.0),
						s3d::Random(area.y + tg.radius + 40.0, area.y + area.h - tg.radius - 40.0)
					};
					emitForbiddenBurst(tg.pos, tg.enraged ? 8 : 5, 190.0 + 20.0 * mult);
					break;
				default:
					break;
				}
				const double interval = tg.enraged ? 0.65 : 1.2;
				tg.nextActionAt = t + interval;
			}
		}

		// 視認（hover）
		bool gazed = false, cursorOn = false;
		if (g.useGazeAttention)   gazed = s3d::Circle(tg.pos, tg.radius).intersects(gazeScreen);
		if (g.useCursorAttention) cursorOn = s3d::Circle(tg.pos, tg.radius).intersects(cursor);

		const bool hovered = (gazed || cursorOn);
		if (hovered)  tg.hoverMs += s3d::Scene::DeltaTime() * 1000.0;
		else          tg.hoverMs = s3d::Max(0.0, tg.hoverMs - 2.0);

		if (tg.type == TargetType::Forbidden
			&& hovered && !tg.warnShownAt.has_value()
			&& tg.hoverMs >= kWarnHoverThresholdMs) {
			tg.warnShownAt = t; // 「！」表示
		}

		// 緑の周期的な反撃：ダメージ済みの間のみ
		if (tg.type == TargetType::Normal && tg.hp < tg.maxHP) {
			if (t >= tg.nextBurstAt) {
				emitForbiddenBurst(tg.pos, forbiddenBurstCountForStage(g.stage), 180.0 + 20.0 * g.stage);
				tg.nextBurstAt = t + forbiddenBurstIntervalForStage(g.stage);
			}
		}
	}
}

// ===== 発射 =====
void RunScene::shootIfReady(double t) {
	auto& g = getData();
	const auto& wp = g.armory[g.equipped];
	const double interval = (wp.fireRate > 0.0 ? 1.0 / wp.fireRate : 1e9);
	if (!(s3d::MouseL.pressed() && (t - g.lastShotTime) >= interval)) return;

	g.lastShotTime = t;

	const s3d::Vec2 vanish = vanishPoint();
	Bullet b;
	b.pos = s3d::Cursor::PosF();

	s3d::Vec2 dir = (vanish - b.pos).setLength(1.0);
	if (wp.spreadDeg > 0.0) {
		const double ang = s3d::Random(-wp.spreadDeg, wp.spreadDeg) * (s3d::Math::Pi / 180.0);
		dir = dir.rotated(ang);
	}
	b.dir2D = dir;
	b.speed2D = wp.speed2D;
	b.z = 0.0;
	b.zVel = wp.projectileSpeedZ;

	g.bullets << b;

	ShotLog log;
	log.t = t;
	log.pos = b.pos;
	log.preSpeed = ComputePreSpeed(g.cursorTrace, t, kPreSpeedWindowMs);
	log.distToNearest = DistToNearestTarget(g.targets, b.pos);
	g.shots << log;
}

// ===== 弾更新（近距離のみ当たり判定） =====
void RunScene::updateBullets(double /*tNow*/) {
	auto& g = getData();
	const s3d::Vec2 vanish = vanishPoint();
	const s3d::RectF area = playArea();

	auto projectToScreen = [&](const s3d::Vec2& p0, double z)->s3d::Vec2 {
		const double k = 0.003;
		const double s = 1.0 / (1.0 + k * z);
		return vanish + (p0 - vanish) * s;
		};

	for (auto& b : g.bullets) if (b.live) {
		const double dt = s3d::Scene::DeltaTime();
		b.pos += b.dir2D * b.speed2D * dt;
		b.z += b.zVel * dt;

		if (b.z > kBulletMaxZ) { b.live = false; continue; }

		const s3d::Vec2 sp = projectToScreen(b.pos, b.z);
		if (!area.intersects(sp)) {
			b.live = false; continue;
		}

		// 当たり判定は手前の薄い板だけ
		if (b.z > kBulletHitMaxZ) {
			continue; // 描画はdraw()で背面として表示
		}

		// ヒット
		for (auto& tg : g.targets) if (tg.alive) {
			const double effectiveR = tg.radius * kHitRadiusScale;
			if ((sp - tg.pos).length() <= effectiveR) {
				b.live = false;

				// エフェクト
				m_rings << RingFX{ sp, s3d::Scene::Time(), kRingLife };
				const double baseAng = s3d::Random(0.0, s3d::Math::TwoPi);
				for (int i = 0; i < kSparkCount; ++i) {
					const double ang = baseAng + s3d::Math::TwoPi * (i / static_cast<double>(kSparkCount));
					const s3d::Vec2 dir = { s3d::Cos(ang), s3d::Sin(ang) };
					const double speed = s3d::Random(140.0, 280.0);
					m_sparks << SparkFX{ sp, dir * speed, s3d::Scene::Time(), kSparkLife };
				}

				// ログ付与
				if (!g.shots.isEmpty()) {
					auto& last = g.shots.back();
					last.hit = true;
					last.hitType = tg.type;
					last.forbiddenPreAware =
						(tg.type == TargetType::Forbidden) && tg.warnShownAt.has_value();
				}

				if (tg.type == TargetType::Forbidden) {
					if (tg.warnShownAt.has_value()) ++g.runForbiddenAwareHits;
					else ++g.runForbiddenUnawareHits;
					g.life -= (tg.warnShownAt.has_value() ? kForbiddenPrePenalty : kForbiddenPostPenalty);
					g.score += kScoreForbiddenPenalty;
					tg.alive = false;
				}
				else {
					// 緑：HPダメージ
					const double dmg = g.armory[g.equipped].damage;
					const double prevHP = tg.hp;
					tg.hp -= dmg;

					// 即時の反撃（生存時）
					if (tg.hp < prevHP && tg.hp > 0.0) {
						emitForbiddenBurst(tg.pos, forbiddenBurstCountForStage(g.stage), 180.0 + 20.0 * g.stage);
					}
					// 初ダメージで周期反撃開始
					if (prevHP == tg.maxHP && tg.hp < tg.maxHP && tg.hp > 0.0) {
						tg.nextBurstAt = s3d::Scene::Time() + forbiddenBurstIntervalForStage(g.stage);
					}

					if (tg.hp <= 0.0) {
						if (tg.isGolden) {
							++g.runGoldenKills;
							g.score += 600;
							g.unlockAchievement(3);
						}
						if (tg.isBoss) {
							++g.runBossKills;
							if (tg.enraged) ++g.runEnragedBossKills;
							if (tg.bossKind == BossKind::Warp) ++g.runWarpBossKills;
							g.score += 2500;
						}
						tg.alive = false;
						g.life = s3d::Min(kMaxLife, g.life + kHealOnNormal);
						g.score += kScoreNormalKill;
						m_roundKills += 1; // ノルマ用
					}
				}
				break;
			}
		}
	}
}

// ===== Forbidden（赤弾）発射制御 =====
int RunScene::forbiddenBurstCountForStage(int stage) const {
	return 3 + (stage - 1) / 2;
}
double RunScene::forbiddenBurstIntervalForStage(int stage) const {
	return 2.8 / (1.0 + 0.12 * (stage - 1));
}
void RunScene::emitForbiddenBurst(const s3d::Vec2& center, int count, double speed) {
	for (int i = 0; i < count; ++i) {
		const double ang = s3d::Math::TwoPi * (i / static_cast<double>(count)) + s3d::Random(-0.15, 0.15);
		Target t;
		t.type = TargetType::Forbidden;
		t.alive = true;
		t.wallBouncesRemaining = 1; // 緑ターゲット由来の赤は1回だけ反射
		t.isBoss = false;
		t.bossKind = BossKind::None;
		t.enraged = false;
		t.isGolden = false;
		t.nextActionAt = s3d::Math::Inf;
		t.pos = center + s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * 4.0;
		t.vel = s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * speed;
		t.radius = 18.0;
		t.spawnTime = s3d::Scene::Time();
		t.maxHP = t.hp = 1.0;
		m_pendingTargets << t; // ループ安全化のため保留 → フレーム末に反映
		++getData().runSpawnTotal;
		++getData().runSpawnForbidden;
	}
}

BossKind RunScene::bossKindForStage(int stage) const {
	const int idx = ((stage / 5) - 1) % 3;
	if (idx == 0) return BossKind::Speed;
	if (idx == 1) return BossKind::Arsenal;
	return BossKind::Warp;
}

void RunScene::spawnBoss(int stage, double now) {
	auto& g = getData();
	const s3d::RectF area = playArea();
	Target b;
	b.type = TargetType::Normal;
	b.isBoss = true;
	b.bossKind = bossKindForStage(stage);
	b.enraged = false;
	b.isGolden = false;
	b.alive = true;
	b.wallBouncesRemaining = -1;
	b.radius = 44.0;
	b.pos = { area.x + area.w * 0.5, area.y + area.h * 0.5 - 40.0 };
	const double baseSpd = 120.0 + 6.0 * stage;
	const double ang = s3d::Random(0.0, s3d::Math::TwoPi);
	b.vel = s3d::Vec2(s3d::Cos(ang), s3d::Sin(ang)) * baseSpd;
	b.maxHP = b.hp = 12.0 + stage * 1.3;
	b.spawnTime = now;
	b.nextActionAt = now + 1.0;
	b.warnShownAt = s3d::none;
	b.hoverMs = 0.0;
	g.targets << b;
	m_bossSpawnedThisRound = true;
}
