#pragma once
#include <Siv3D.hpp>
#include <limits>
#include <cmath>
#include "GameData.hpp"

// 直前 windowMs の平均マウス速度(px/s)
inline double ComputePreSpeed(const s3d::Array<CursorSample>& trace, double t, double windowMs = 120.0) {
	const double from = t - windowMs / 1000.0;
	s3d::Vec2 last{ std::numeric_limits<double>::quiet_NaN(),
					std::numeric_limits<double>::quiet_NaN() };
	double dist = 0.0;
	double prevT = std::numeric_limits<double>::quiet_NaN();

	for (const auto& s : trace) {
		if (s.t < from || s.t > t) continue;
		if (std::isnan(prevT)) { prevT = s.t; last = s.pos; continue; }
		dist += last.distanceFrom(s.pos);
		last = s.pos;
		prevT = s.t;
	}
	if (std::isnan(prevT)) return 0.0;
	return dist / (windowMs / 1000.0);
}

// 点 p と生存ターゲット中心の最近傍距離
inline double DistToNearestTarget(const s3d::Array<Target>& tgts, const s3d::Vec2& p) {
	double d = s3d::Math::Inf;
	for (const auto& g : tgts) if (g.alive) d = s3d::Min(d, p.distanceFrom(g.pos));
	return d;
}

// CSV保存（shots）
inline void SaveShotsCSV(const s3d::Array<ShotLog>& shots) {
	const auto folder = U"logs/";
	s3d::FileSystem::CreateDirectories(folder);
	const auto name = s3d::DateTime::Now().format(U"run_%Y%m%d_%H%M%S_shots.csv");
	s3d::TextWriter w(folder + name);
	w.write(U"t,x,y,hit,hitType,forbiddenPreAware,preSpeed,distToNearest");
	for (const auto& s : shots) {
		w.write(U"{:.3f},{:.1f},{:.1f},{},{},{},{:.1f},{:.1f}"_fmt(
			s.t, s.pos.x, s.pos.y,
			(s.hit ? U"1" : U"0"),
			(s.hitType == TargetType::Forbidden ? U"Forbidden" : U"Normal"),
			(s.forbiddenPreAware ? U"1" : U"0"),
			s.preSpeed, s.distToNearest
		));
	}
	s3d::Print(U"Saved: {}"_fmt(folder + name));
}
