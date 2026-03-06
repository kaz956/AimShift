#pragma once
#include <Siv3D.hpp>

// 生視線を 0..1（左上=0,0 右下=1,1）で返す共通インタフェース
struct GazeProvider {
	virtual ~GazeProvider() = default;
	virtual bool available() const = 0;    // デバイスが利用可能か
	virtual s3d::Vec2 readRaw01() = 0;     // 0..1 の生視線（未取得時は {0.5,0.5} など）
};

// ===== マウス（デバッグ用） =====
struct MouseGazeProvider final : public GazeProvider {
	bool available() const override { return true; }

	s3d::Vec2 readRaw01() override {
		const auto p = s3d::Cursor::PosF();
		return {
			s3d::Clamp(p.x / (double)s3d::Scene::Width(),  0.0, 1.0),
			s3d::Clamp(p.y / (double)s3d::Scene::Height(), 0.0, 1.0)
		};
	}
};

#ifdef USE_OPENCV
// ===== OpenCV による Web カメラ視線推定 =====
#include <opencv2/opencv.hpp>

namespace detail_gaze {
	inline bool tryOpen(cv::VideoCapture& cap, int index, int apiPref /*-1=default*/) {
		if (cap.isOpened()) cap.release();
		if (apiPref >= 0)  return cap.open(index, apiPref);
		return cap.open(index);
	}
}

struct WebcamGazeProvider final : public GazeProvider {
	cv::VideoCapture      cap;
	cv::CascadeClassifier faceC, eyeC;
	bool hasEyeCascade = false;
	s3d::String logLine;

	WebcamGazeProvider() {
		using namespace detail_gaze;

		// --- 1) カスケード読込 ---
		bool faceOK = false, eyeOK = false;
		{
#ifdef CV_VERSION
			try {
				faceOK = faceC.load(cv::samples::findFile("example/objdetect/haarcascade/frontal_face_alt2.xml", false, false));
				eyeOK = eyeC.load(cv::samples::findFile("example/objdetect/haarcascade/eye.xml", false, false));
			}
			catch (...) { faceOK = eyeOK = false; }
#else
			faceOK = faceC.load("example/objdetect/haarcascade/frontal_face_alt2.xml");
			eyeOK = eyeC.load("example/objdetect/haarcascade/eye.xml");
#endif
			hasEyeCascade = (faceOK && eyeOK);
			if (!faceOK) logLine += U"[Gaze] face cascade not found. ";
			if (!eyeOK)  logLine += U"[Gaze] eye cascade not found (fallback to face center). ";
		}

		// --- 2) カメラ起動を複数パターンで試す ---
		const int indices[] = { 0, 1, 2, 3 };
		const int apis[] = {
		#ifdef _WIN32
			cv::CAP_MSMF, cv::CAP_DSHOW,
		#endif
			- 1  // default
		};
		bool opened = false;
		for (int idx : indices) {
			for (int api : apis) {
				if (tryOpen(cap, idx, api)) { opened = true; break; }
			}
			if (opened) break;
		}
		if (!opened) {
			logLine += U"[Gaze] Cannot open webcam (tried 0..3 with MSMF/DShow/Default). Using mouse fallback.\n";
		}
		else {
			cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
			cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
			cap.set(cv::CAP_PROP_FPS, 30);
			logLine += U"[Gaze] Webcam opened. ";
			if (!hasEyeCascade) logLine += U"(eye cascade missing: face-center mode) ";
			logLine += U"\n";
		}

		s3d::Console.open();
		s3d::Console << logLine;
	}

	bool available() const override {
		return cap.isOpened();
	}

	s3d::Vec2 readRaw01() override {
		if (!cap.isOpened()) {
			return { 0.5, 0.5 };
		}

		cv::Mat frame;
		if (!cap.read(frame) || frame.empty()) {
			return { 0.5, 0.5 };
		}

		// グレースケール化 + ヒストグラム平坦化
		cv::Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(gray, gray);

		// 顔検出
		std::vector<cv::Rect> faces;
		faceC.detectMultiScale(gray, faces, 1.2, 3, 0, cv::Size(80, 80));
		if (faces.empty()) {
			return { 0.5, 0.5 };
		}

		const cv::Rect face = faces[0];
		const double cxFace = (face.x + face.width * 0.5) / frame.cols;
		const double cyFace = (face.y + face.height * 0.5) / frame.rows;

		// 目カスケードがなければ顔中心
		if (!hasEyeCascade) {
			return {
				s3d::Clamp(cxFace, 0.0, 1.0),
				s3d::Clamp(cyFace, 0.0, 1.0)
			};
		}

		// 目検出
		cv::Mat roi = gray(face);
		std::vector<cv::Rect> eyes;
		eyeC.detectMultiScale(roi, eyes, 1.2, 2);

		if (eyes.empty()) {
			return {
				s3d::Clamp(cxFace, 0.0, 1.0),
				s3d::Clamp(cyFace, 0.0, 1.0)
			};
		}

		double sx = 0.0, sy = 0.0; int n = 0;
		for (const auto& e : eyes) {
			const double ex = (face.x + e.x + e.width * 0.5) / frame.cols;
			const double ey = (face.y + e.y + e.height * 0.5) / frame.rows;
			sx += ex;
			sy += ey;
			++n;
		}

		if (n == 0) {
			return {
				s3d::Clamp(cxFace, 0.0, 1.0),
				s3d::Clamp(cyFace, 0.0, 1.0)
			};
		}

		const double gx = s3d::Clamp(sx / n, 0.0, 1.0);
		const double gy = s3d::Clamp(sy / n, 0.0, 1.0);
		return { gx, gy };
	}
};

#else
// ===== OpenCV なしビルド時のダミー Web カメラ版 =====
struct WebcamGazeProvider final : public GazeProvider {
	bool available() const override { return false; }
	s3d::Vec2 readRaw01() override { return { 0.5, 0.5 }; }
};
#endif