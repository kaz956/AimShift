#pragma once
#include <Siv3D.hpp>

// Return normalized gaze position in [0, 1] x [0, 1].
struct GazeProvider {
	virtual ~GazeProvider() = default;
	virtual bool available() const = 0;
	virtual s3d::Vec2 readRaw01() = 0;
};

// Mouse fallback provider
struct MouseGazeProvider final : public GazeProvider {
	bool available() const override { return true; }

	s3d::Vec2 readRaw01() override {
		const auto p = s3d::Cursor::PosF();
		return {
			s3d::Clamp(p.x / static_cast<double>(s3d::Scene::Width()),  0.0, 1.0),
			s3d::Clamp(p.y / static_cast<double>(s3d::Scene::Height()), 0.0, 1.0)
		};
	}
};

#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>

namespace detail_gaze {
	inline bool tryOpen(cv::VideoCapture& cap, int index, int apiPref /*-1=default*/) {
		if (cap.isOpened()) cap.release();
		if (apiPref >= 0) return cap.open(index, apiPref);
		return cap.open(index);
	}

	inline bool loadCascade(cv::CascadeClassifier& c, const std::string& relPath) {
		if (c.load(relPath)) return true;
		if (c.load("App/" + relPath)) return true;
		if (c.load("./" + relPath)) return true;
		if (c.load("../" + relPath)) return true;
		try {
			const std::string found = cv::samples::findFile(relPath, false, false);
			if (!found.empty() && c.load(found)) return true;
		}
		catch (...) {}
		return false;
	}
}

struct WebcamGazeProvider final : public GazeProvider {
	cv::VideoCapture cap;
	cv::CascadeClassifier faceC, eyeC;
	bool hasEyeCascade = false;
	s3d::Vec2 lastRaw{ 0.5, 0.5 };
	bool hasLastRaw = false;
	s3d::String logLine;

	WebcamGazeProvider() {
		using namespace detail_gaze;

		bool faceOK = loadCascade(faceC, "example/objdetect/haarcascade/frontal_face_alt2.xml");
		bool eyeOK = loadCascade(eyeC, "example/objdetect/haarcascade/eye.xml");
		hasEyeCascade = (faceOK && eyeOK);
		if (!faceOK) logLine += U"[Gaze] face cascade not found. ";
		if (!eyeOK) logLine += U"[Gaze] eye cascade not found (fallback to face center). ";

		const int indices[] = { 0, 1, 2, 3 };
		const int apis[] = {
#ifdef _WIN32
			cv::CAP_MSMF, cv::CAP_DSHOW,
#endif
			-1
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
		if (!cap.isOpened()) return { 0.5, 0.5 };

		cv::Mat frame;
		if (!cap.read(frame) || frame.empty()) {
			return hasLastRaw ? lastRaw : s3d::Vec2{ 0.5, 0.5 };
		}

		cv::Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(gray, gray);

		std::vector<cv::Rect> faces;
		faceC.detectMultiScale(gray, faces, 1.2, 3, 0, cv::Size(80, 80));
		if (faces.empty()) {
			return hasLastRaw ? lastRaw : s3d::Vec2{ 0.5, 0.5 };
		}

		const cv::Rect face = faces[0];
		const double cxFace = (face.x + face.width * 0.5) / frame.cols;
		const double cyFace = (face.y + face.height * 0.5) / frame.rows;

		auto updateSmoothed = [&](const s3d::Vec2& raw, double alpha) {
			lastRaw = hasLastRaw ? s3d::Math::Lerp(lastRaw, raw, alpha) : raw;
			hasLastRaw = true;
			return lastRaw;
		};

		if (!hasEyeCascade) {
			const s3d::Vec2 raw{
				s3d::Clamp(cxFace, 0.0, 1.0),
				s3d::Clamp(cyFace, 0.0, 1.0)
			};
			return updateSmoothed(raw, 0.25);
		}

		cv::Mat roi = gray(face);
		std::vector<cv::Rect> eyes;
		eyeC.detectMultiScale(roi, eyes, 1.2, 2);

		if (eyes.empty()) {
			const s3d::Vec2 raw{
				s3d::Clamp(cxFace, 0.0, 1.0),
				s3d::Clamp(cyFace, 0.0, 1.0)
			};
			return updateSmoothed(raw, 0.25);
		}

		double sx = 0.0, sy = 0.0;
		int n = 0;
		for (const auto& e : eyes) {
			const double ex = (face.x + e.x + e.width * 0.5) / frame.cols;
			const double ey = (face.y + e.y + e.height * 0.5) / frame.rows;
			sx += ex;
			sy += ey;
			++n;
		}

		if (n == 0) {
			return hasLastRaw ? lastRaw : s3d::Vec2{ 0.5, 0.5 };
		}

		const s3d::Vec2 raw{
			s3d::Clamp(sx / n, 0.0, 1.0),
			s3d::Clamp(sy / n, 0.0, 1.0)
		};
		return updateSmoothed(raw, 0.35);
	}
};

#else
struct WebcamGazeProvider final : public GazeProvider {
	bool available() const override { return false; }
	s3d::Vec2 readRaw01() override { return { 0.5, 0.5 }; }
};
#endif
