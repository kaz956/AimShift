// GameData.hpp
#pragma once
#include <Siv3D.hpp>
#include "input/Gaze.hpp"
#include "Entities.hpp"   // ★ 追加：唯一のDifficulty定義

// ===== シーン / エンティティ種別 =====
enum class SceneID { Home, Tutorial, Calibration, Run, Result, Achievements };
enum class TargetType { Normal, Forbidden };
enum class BossKind { None, Speed, Arsenal, Warp };

// ===== 視線キャリブ結果（生→画面の2×3アフィン） =====
struct GazeCalib {
	s3d::Mat3x2 A = s3d::Mat3x2::Identity();
	bool valid = false;
	// raw(0..1) → screen(px)
	s3d::Vec2 map(const s3d::Vec2& raw01) const {
		const double x = (A._11 * raw01.x + A._12 * raw01.y + A._31);
		const double y = (A._21 * raw01.x + A._22 * raw01.y + A._32);
		return { x, y };
	}
};

// ===== 武器 =====
struct Weapon {
	s3d::String name;
	double fireRate;        // 連射[発/秒]
	double damage;          // 攻撃力
	double projectileSpeedZ;// 弾のZ方向速度（擬似3Dの奥行き）
	double speed2D;         // 弾の2D平面速度
	double spreadDeg;       // ばらつき（度）
	double recoil;          // 反動（未使用なら0）
};

// ===== ターゲット / 弾 / ログ =====
struct Target {
	s3d::Vec2 pos{ 0,0 }, vel{ 0,0 };
	double radius = 22.0;
	TargetType type = TargetType::Normal;
	bool alive = true;
	int wallBouncesRemaining = -1; // -1: 無制限, 0以上: 残り反射回数
	bool isBoss = false;
	BossKind bossKind = BossKind::None;
	bool enraged = false;
	bool isGolden = false;
	double nextActionAt = s3d::Math::Inf;

	double spawnTime = 0.0;
	s3d::Optional<double> warnShownAt = s3d::none; // 「！」表示時刻
	double hoverMs = 0.0;                          // 視線/カーソルホバー累積

	double hp = 1.0, maxHP = 1.0;                  // 緑ターゲットのHP
	double nextBurstAt = s3d::Math::Inf;           // 緑が周期的に赤弾を出す時刻
};

struct Bullet {
	s3d::Vec2 pos{ 0,0 }, dir2D{ 0,0 };
	double z = 0.0;         // 擬似奥行き
	double speed2D = 220.0; // 平面速度
	double zVel = 900.0;    // Z速度
	bool live = true;
};

struct CursorSample { double t = 0.0; s3d::Vec2 pos{ 0,0 }; };

struct ShotLog {
	double t = 0.0;
	s3d::Vec2 pos{ 0,0 };
	bool hit = false;
	TargetType hitType = TargetType::Normal;
	bool forbiddenPreAware = false; // 「！」出てるのに撃ったか（事前気付き）
	double preSpeed = 0.0;          // 直前カーソル速度
	double distToNearest = s3d::Math::Inf;
};

struct AchievementState {
	s3d::String name;
	s3d::String hint;
	bool unlocked = false;
};

// ===== ゲーム共有データ =====
struct GameData {
	// 視線
	GazeCalib gaze;
	std::unique_ptr<GazeProvider> gazeProvider; // マウス / Webカメラ
	bool usingWebcam = false;
	bool useCursorAttention = true;             // カーソルでも「！」判定
	bool useGazeAttention = true;             // 視線でも「！」判定

	// 武器
	s3d::Array<Weapon> armory{
		Weapon{ U"PeaShooter", 6.0, 1.0,  900.0, 220.0, 1.0, 0.0 },
		Weapon{ U"Rifle",      3.5, 2.0, 1100.0, 220.0, 0.6, 0.0 },
		Weapon{ U"SMG",       10.0, 0.7,  850.0, 220.0, 2.0, 0.0 },
		Weapon{ U"Vanguard Cannon", 5.8, 2.4, 1200.0, 250.0, 0.5, 0.0 }, // Boss Hunter I
		Weapon{ U"Overdrive Buster", 4.8, 3.0, 1320.0, 280.0, 0.35, 0.0 }, // Overclock Breaker
		Weapon{ U"Anchor Rifle", 6.8, 2.1, 1250.0, 300.0, 0.25, 0.0 }, // Warp Takedown
		Weapon{ U"Auric Beam", 8.8, 1.9, 1400.0, 320.0, 0.9, 0.0 }, // Golden Ticket
		Weapon{ U"Decimator-10", 6.2, 2.6, 1380.0, 285.0, 0.15, 0.0 }, // Ten Percent Rule
		Weapon{ U"Sentinel Eye", 7.4, 2.2, 1300.0, 295.0, 0.2, 0.0 }, // Aware Only
		Weapon{ U"Tempest Rail", 5.2, 3.4, 1500.0, 330.0, 0.10, 0.0 }, // Stage Master 10
		Weapon{ U"Eclipse Nova", 9.4, 2.0, 1550.0, 340.0, 0.8, 0.0 }, // Stage Master 15
	};
	s3d::Array<bool> weaponUnlocked{ true, true, true, false, false, false, false, false, false, false, false };
	int equipped = 0;

	// 実績
	s3d::Array<AchievementState> achievements{
		{ U"Boss Hunter I", U"Stage 5, 10, 15... で出現するボスを1体撃破する", false },
		{ U"Overclock Breaker", U"ボスのHP半分以下の強化状態を撃破する", false },
		{ U"Warp Takedown", U"ワープするボスを撃破する", false },
		{ U"Golden Ticket", U"Stage 5以降で超低確率の黄色ターゲットを撃破する", false },
		{ U"Ten Percent Rule", U"1ラン中のForbidden出現率をちょうど10%にする", false },
		{ U"Aware Only", U"Forbiddenを撃つ時、警告表示後のみ命中させる", false },
		{ U"Stage Master 10", U"Stage 10 以上に到達する", false },
		{ U"Stage Master 15", U"Stage 15 以上に到達する", false },
	};

	// ラン時の状態
	double playerSkill = 1.0;
	int    stage = 1;
	double life = 6.0;
	int    score = 0;
	s3d::Stopwatch stageTimer{ s3d::StartImmediately::No };

	s3d::Array<Target> targets;
	s3d::Array<Bullet> bullets;
	s3d::Array<CursorSample> cursorTrace;
	s3d::Array<ShotLog> shots;
	double lastShotTime = -1e9;

	// ラン中統計（実績判定用）
	int runSpawnTotal = 0;
	int runSpawnForbidden = 0;
	int runForbiddenAwareHits = 0;
	int runForbiddenUnawareHits = 0;
	int runBossKills = 0;
	int runWarpBossKills = 0;
	int runEnragedBossKills = 0;
	int runGoldenSpawns = 0;
	int runGoldenKills = 0;

	// 初期化
	void initGazeProvider() {
		usingWebcam = false;
#ifdef USE_OPENCV
		auto cam = std::make_unique<WebcamGazeProvider>();
		if (cam->available()) {
			gazeProvider = std::move(cam);
			usingWebcam = true;
			return;
		}
#endif
		gazeProvider = std::make_unique<MouseGazeProvider>();
	}
	// 生視線(0..1)
	s3d::Vec2 readRawGaze01() {
		if (!gazeProvider) return { 0.5, 0.5 };
		return gazeProvider->readRaw01();
	}

	// 新規ラン開始
	void resetRun() {
		stage = 1; life = 6.0; score = 0;
		targets.clear(); bullets.clear(); cursorTrace.clear(); shots.clear();
		stageTimer.reset(); // RunScene に入るまで開始しない
		lastShotTime = -1e9;
		runSpawnTotal = 0;
		runSpawnForbidden = 0;
		runForbiddenAwareHits = 0;
		runForbiddenUnawareHits = 0;
		runBossKills = 0;
		runWarpBossKills = 0;
		runEnragedBossKills = 0;
		runGoldenSpawns = 0;
		runGoldenKills = 0;
	}

	void unlockWeapon(const size_t idx) {
		if (idx < weaponUnlocked.size()) {
			weaponUnlocked[idx] = true;
		}
	}

	void unlockAchievement(const size_t idx) {
		if (idx < achievements.size()) {
			if (!achievements[idx].unlocked) {
				achievements[idx].unlocked = true;
				// 1実績解除につき1武器開放（基礎3武器の次から対応付け）
				const size_t weaponIdx = 3 + idx;
				unlockWeapon(weaponIdx);
			}
		}
	}

	void evaluateRunAchievements() {
		if (runBossKills >= 1) unlockAchievement(0);
		if (runEnragedBossKills >= 1) unlockAchievement(1);
		if (runWarpBossKills >= 1) unlockAchievement(2);
		if (runGoldenKills >= 1) unlockAchievement(3);
		if (runSpawnTotal >= 20 && (runSpawnForbidden * 10 == runSpawnTotal)) unlockAchievement(4);
		if (runForbiddenAwareHits > 0 && runForbiddenUnawareHits == 0) unlockAchievement(5);
		if (stage >= 10) unlockAchievement(6);
		if (stage >= 15) unlockAchievement(7);
	}
};
