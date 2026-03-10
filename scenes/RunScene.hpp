#pragma once
#include <Siv3D.hpp>
#include "../GameData.hpp"
#include "../Entities.hpp"
#include "../Util.hpp"

class RunScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;
	RunScene(const InitData& init);
	void update() override;
	void draw() const override;

private:
	void spawnTargets(const Difficulty& d, double now);
	void spawnBoss(int stage, double now);
	void updateTargets(const Difficulty& d, double t);
	void shootIfReady(double t);
	void updateBullets(double t);

	// --- ヒットエフェクト ---
	struct RingFX { s3d::Vec2 pos; double bornT; double life; };
	struct SparkFX { s3d::Vec2 pos, vel; double bornT; double life; };
	void updateEffects();
	void drawEffects() const;

	// 当たり判定が有効な奥行きの上限（これより深い弾はヒットしない）
	const double kBulletHitMaxZ = 120.0;  // ★ 追加

	// --- UI ---
	void drawWormhole(const s3d::Vec2& vanish) const;
	void drawGameFrame() const;
	void drawLifeMeter() const;
	void drawRoundTimerAndQuota() const;   // ★ 残り時間 & ノルマ表示

	// --- Forbidden 発射制御（緑の反撃） ---
	void emitForbiddenBurst(const s3d::Vec2& center, int count, double speed);
	int  forbiddenBurstCountForStage(int stage) const;
	double forbiddenBurstIntervalForStage(int stage) const;
	BossKind bossKindForStage(int stage) const;

	// ラウンド時間（ステージごとに設定）
	double stageTimeLimitForStage(int stage) const {
		return s3d::Clamp(25.0 - 1.5 * (stage - 1), 12.0, 25.0);
	}
	// ★ ラウンド撃破ノルマ（ステージごと）
	int quotaForStage(int stage) const {
		return 10 + 2 * (stage - 1); // 例：Stage1=10, 2=12, …
	}

	// しきい値
	const double kWarnHoverThresholdMs = 120.0;
	const double kPreSpeedWindowMs = 120.0;
	const double kForbiddenPrePenalty = 2.0;
	const double kForbiddenPostPenalty = 1.0;

	// スコア
	const int kScoreNormalKill = 100;
	const int kScoreForbiddenPenalty = -50;

	// 緑ターゲットの回復
	const double kHealOnNormal = 0.35;
	const double kMaxLife = 8.0;

	// 当たり判定
	const double kHitRadiusScale = 0.65;
	const double kBulletMaxZ = 1200.0;

	// エフェクト設定
	const double kRingLife = 0.35;
	const double kSparkLife = 0.55;
	const int    kSparkCount = 14;

	// スポーン管理
	double m_spawnAccum = 0.0;

	// 現在ラウンドの制限時間（秒）とノルマ
	double m_stageTimeLimit = 25.0;
	int    m_roundQuota = 10;   // ★ ノルマ
	int    m_roundKills = 0;    // ★ 今ラウンド撃破数
	bool   m_bossSpawnedThisRound = false;

	// 消失点（“間くらい”）
	s3d::Vec2 vanishPoint() const {
		const double H = static_cast<double>(s3d::Scene::Height());
		const double yTop = 80.0, yBottom = H - 120.0, blend = 0.5;
		return { s3d::Scene::CenterF().x, yTop + (yBottom - yTop) * blend };
	}
	s3d::RectF playArea() const {
		return { 0.0, 0.0, (double)s3d::Scene::Width(), (double)s3d::Scene::Height() - 140.0 };
	}

	// エフェクト蓄積
	s3d::Array<RingFX>  m_rings;
	s3d::Array<SparkFX> m_sparks;

	// このフレームの追加予定ターゲット（安全に後で反映）
	s3d::Array<Target> m_pendingTargets;

	// フォント
	s3d::Font mHUD{ 20 };
	s3d::Font mWarn{ 28 };
};
