// GameData.hpp
#pragma once
#include <Siv3D.hpp>
#include "input/Gaze.hpp"
#include "Entities.hpp"   // ★ 追加：唯一のDifficulty定義

// ===== シーン / エンティティ種別 =====
enum class SceneID { Home, Calibration, Run, Result };
enum class TargetType { Normal, Forbidden };

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
	};
	int equipped = 0;

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
		stageTimer.reset(); stageTimer.start();
		lastShotTime = -1e9;
	}
};
