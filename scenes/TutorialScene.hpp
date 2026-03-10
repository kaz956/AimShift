#pragma once
#include <Siv3D.hpp>
#include "../GameData.hpp"

class TutorialScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;
	TutorialScene(const InitData& init);
	void update() override;
	void draw() const override;

private:
	enum class TutorialMode {
		Explain,
		Action,
		AwaitNext,
	};

	struct DemoTarget {
		s3d::Vec2 pos{ 0, 0 };
		s3d::Vec2 vel{ 0, 0 };
		double radius = 24.0;
		double hp = 1.0;
		double maxHP = 1.0;
		bool alive = true;
		TargetType type = TargetType::Normal;
		bool isBoss = false;
		bool isGolden = false;
		bool enraged = false;
		bool fromCounter = false;
		int wallBouncesRemaining = -1;
		double hoverMs = 0.0;
		s3d::Optional<double> warnShownAt = s3d::none;
		double nextActionAt = s3d::Math::Inf;
		double expireAt = s3d::Math::Inf;
	};

	s3d::Font mTitle{ 34, s3d::Typeface::Bold };
	s3d::Font mBody{ 22 };
	s3d::Font mSmall{ 16 };
	s3d::Array<DemoTarget> mTargets;
	s3d::Array<DemoTarget> mPendingTargets;
	s3d::Array<Bullet> mBullets;
	int mStep = 0;
	TutorialMode mMode = TutorialMode::Explain;
	double mStepStartedAt = 0.0;
	double mInputUnlockAt = 0.0;
	double mLife = 8.0;
	double mTimeLimit = 60.0;
	int mQuota = 6;
	int mKills = 0;
	double mLastShotAt = -1e9;
	int mStepShots = 0;
	s3d::String mStatus;
	s3d::String mStepText;
	bool mCounterTriggered = false;
	bool mCounterHalfTriggered = false;
	double mLifeBeforeStep = 8.0;
	bool mLastForbiddenWarned = false;
	double mDemoFireRate = 4.0;
	double mDemoBulletSpeed2D = 240.0;
	double mDemoBulletZVel = 980.0;
	double mDemoBulletHitMaxZ = 120.0;
	double mDemoBulletMaxZ = 1200.0;

	void setupStep();
	bool canAcceptConfirm(double now) const;
	bool requiresAction(int step) const;
	void updateTargets(double now);
	void spawnGreenCounter(const s3d::Vec2& center, double now);
	void processShoot(double now);
	void updateBullets();
	void drawBullets() const;
	void drawTarget(const DemoTarget& t) const;
	void advanceStep(double now);
	void enterExplain(double now, const s3d::String& text);
	void enterAction(double now);
	void enterAwaitNext(double now, const s3d::String& text);
	s3d::RectF playArea() const;
	s3d::RectF currentHighlightRect() const;
	s3d::String currentOverlayText() const;
	void drawGuideOverlay() const;
	bool hasAliveTarget(TargetType type) const;
	bool hasAliveGreen() const;
	bool hasAliveYellow() const;
	s3d::Vec2 vanishPoint() const;
	s3d::Vec2 projectToScreen(const s3d::Vec2& p, double z) const;
};
