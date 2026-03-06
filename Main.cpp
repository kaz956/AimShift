#include <Siv3D.hpp>

#include "GameData.hpp"
#include "scenes/HomeScene.hpp"
#include "scenes/CalibrationScene.hpp"
#include "scenes/RunScene.hpp"
#include "scenes/ResultScene.hpp"

using App = s3d::SceneManager<SceneID, GameData>;

void Main() {
	s3d::Window::Resize(1000, 680);
	s3d::Scene::SetBackground(s3d::ColorF(0.05, 0.07, 0.10));

	App app;
	app.add<HomeScene>(SceneID::Home);
	app.add<CalibrationScene>(SceneID::Calibration);
	app.add<RunScene>(SceneID::Run);
	app.add<ResultScene>(SceneID::Result);

	// ★ ホームから開始
	app.init(SceneID::Home);

	while (s3d::System::Update()) {
		if (!app.update()) break;
	}
}
