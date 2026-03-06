#pragma once
#include <Siv3D.hpp>
#include <cstring>            // std::memcpy
#include "../GameData.hpp"    // GameData / gaze provider

// 画面上の 3x3 点を順に見てもらい、raw(0..1)→screen(px) の 2x3 アフィンを学習
class CalibrationScene : public s3d::SceneManager<SceneID, GameData>::Scene {
public:
	using MyApp = s3d::SceneManager<SceneID, GameData>;

	CalibrationScene(const InitData& init)
		: IScene(init)
	{
		// 視線プロバイダ初期化（Webカメラが使えなければマウスにフォールバック）
		getData().initGazeProvider();

		buildTargetsGrid();
		getData().gaze.valid = false;
		m_hold.restart();
	}

	void update() override {
		auto& g = getData();

		// 参照点をすべて回り切ったらフィットして Run へ
		if (m_idx >= static_cast<int>(m_targets.size())) {
			s3d::Mat3x2 A;
			if (fitAffine2x3(m_raw, m_targets, A)) {
				g.gaze.A = A;
				g.gaze.valid = true;
			}
			else {
				g.gaze.A = s3d::Mat3x2::Identity();
				g.gaze.valid = false;
			}
			changeScene(SceneID::Run);
			return;
		}

		// 1点あたり 0.8 秒ホールドして 1 サンプル採取
		if (m_hold.sF() >= 0.8) {
			const s3d::Vec2 r01 = g.readRawGaze01(); // 生視線 0..1
			m_raw << r01;
			++m_idx;
			m_hold.restart();
		}
	}

	void draw() const override {
		s3d::Scene::SetBackground(s3d::ColorF(0.03, 0.03, 0.05));

		// 参照点表示
		for (int i = 0; i < static_cast<int>(m_targets.size()); ++i) {
			const auto& p = m_targets[i];
			const bool current = (i == m_idx);
			s3d::Circle(p, current ? 18 : 10)
				.draw(current ? s3d::ColorF(0.60, 0.85, 1.00)
							  : s3d::ColorF(0.20, 0.40, 0.70));
		}

		// W100 回避のため Font はメンバ（毎フレーム生成しない）
		mMsg(U"Look at the highlighted dots to calibrate gaze.\n"
			 U"(Webcam if available, otherwise mouse)")
			.draw(20, 20, s3d::Palette::White);
	}

private:
	// ====== メンバ ======
	s3d::Array<s3d::Vec2> m_targets;   // 画面座標の参照点（3x3）
	s3d::Array<s3d::Vec2> m_raw;       // 参照点に対応する raw 視線（0..1）
	int m_idx = 0;                     // いま注視して欲しい点のインデックス
	s3d::Stopwatch m_hold{ s3d::StartImmediately::Yes };
	s3d::Font mMsg{ 20 };              // ガイドメッセージ用フォント

	// ====== 実装（inline） ======
	void buildTargetsGrid() {
		// 3x3 グリッド（上下左右に余白）
		const double margin = 80.0;
		const s3d::RectF area{
			margin, margin,
			(double)s3d::Scene::Width() - margin * 2.0,
			(double)s3d::Scene::Height() - margin * 2.0
		};

		m_targets.clear();
		for (int gy = 0; gy < 3; ++gy) {
			for (int gx = 0; gx < 3; ++gx) {
				const s3d::Vec2 p{
					area.x + area.w * (gx / 2.0),   // 0, 0.5, 1.0
					area.y + area.h * (gy / 2.0)
				};
				m_targets << p;
			}
		}
	}

	// 3x3 連立の単純ガウス消去
	static void solve3x3(double M[3][3], double b[3], double x[3]) {
		for (int i = 0; i < 3; ++i) {
			int piv = i; double mv = std::abs(M[i][i]);
			for (int r = i + 1; r < 3; ++r) {
				if (std::abs(M[r][i]) > mv) { mv = std::abs(M[r][i]); piv = r; }
			}
			if (piv != i) { std::swap(M[i], M[piv]); std::swap(b[i], b[piv]); }
			const double diag = M[i][i];
			if (std::abs(diag) < 1e-8) continue;
			for (int c = i; c < 3; ++c) M[i][c] /= diag;
			b[i] /= diag;
			for (int r = 0; r < 3; ++r) if (r != i) {
				const double f = M[r][i];
				for (int c = i; c < 3; ++c) M[r][c] -= f * M[i][c];
				b[r] -= f * b[i];
			}
		}
		for (int i = 0; i < 3; ++i) x[i] = b[i];
	}

	// raw(0..1) → screen(px) の 2x3 アフィン最小二乗
	bool fitAffine2x3(const s3d::Array<s3d::Vec2>& raw01,
					  const s3d::Array<s3d::Vec2>& screen,
					  s3d::Mat3x2& outA)
	{
		if (raw01.size() != screen.size() || raw01.size() < 3) return false;

		double XtX[3][3] = { 0 }, XtYx[3] = { 0 }, XtYy[3] = { 0 };
		for (size_t i = 0; i < raw01.size(); ++i) {
			const double rx = raw01[i].x, ry = raw01[i].y;
			const double sx = screen[i].x, sy = screen[i].y;
			const double v[3] = { rx, ry, 1.0 };
			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 3; ++c) XtX[r][c] += v[r] * v[c];
			}
			for (int r = 0; r < 3; ++r) {
				XtYx[r] += v[r] * sx;
				XtYy[r] += v[r] * sy;
			}
		}

		double wx[3], wy[3];
		{
			double A1[3][3]; double b1[3];
			std::memcpy(A1, XtX, sizeof(XtX));
			std::memcpy(b1, XtYx, sizeof(b1));
			solve3x3(A1, b1, wx);
		}
		{
			double A2[3][3]; double b2[3];
			std::memcpy(A2, XtX, sizeof(XtX));
			std::memcpy(b2, XtYy, sizeof(b2));
			solve3x3(A2, b2, wy);
		}

		// Mat3x2 の並び: [a b 0; d e 0; c f 1]
		outA = s3d::Mat3x2(
			wx[0], wy[0],
			wx[1], wy[1],
			wx[2], wy[2]
		);
		return true;
	}
};
