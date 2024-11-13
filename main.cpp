#include "opencv2/opencv.hpp"
#include <iostream>
using namespace cv;
using namespace std;
int main() {
    // GStreamer를 사용하여 카메라로부터 영상 스트리밍을 가져옴
    string src = "nvarguscamerasrc sensor-id=0 ! "
                 "video/x-raw(memory:NVMM), width=(int)640, height=(int)360, "
                 "format=(string)NV12, framerate=(fraction)30/1 ! "
                 "nvvidconv flip-method=0 ! video/x-raw, "
                 "width=(int)640, height=(int)360, format=(string)BGRx ! "
                 "videoconvert ! video/x-raw, format=(string)BGR ! appsink";

    VideoCapture source(src, CAP_GSTREAMER);
    if (!source.isOpened()) {
        cout << "Camera error" << endl;
        return -1;
    }

    // 첫 번째 스트림 (컬러 영상)
    string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! "
                  "nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! "
                  "h264parse ! rtph264pay pt=96 ! "
                  "udpsink host=203.234.58.158 port=8001 sync=false";

    VideoWriter writer1(dst1, 0, 30.0, Size(640, 360), true);
    if (!writer1.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    // 두 번째 스트림 (그레이스케일 영상)
    string dst2 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! "
                  "nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! "
                  "h264parse ! rtph264pay pt=96 ! "
                  "udpsink host=203.234.58.158 port=8002 sync=false";

    VideoWriter writer2(dst2, 0, 30.0, Size(640, 360), false);
    if (!writer2.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    // 세 번째 스트림 (이진 영상)
    string dst3 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! "
                  "nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! "
                  "h264parse ! rtph264pay pt=96 ! "
                  "udpsink host=203.234.58.158 port=8003 sync=false";

    VideoWriter writer3(dst3, 0, 30.0, Size(640, 360), false);
    if (!writer3.isOpened()) {
        cerr << "Writer open failed!" << endl;
        return -1;
    }

    Mat frame, gray, th;
    TickMeter tm;  // OpenCV TickMeter 클래스 사용

    while (true) {
        tm.start();  // 실행 시간 측정 시작

        source >> frame;
        if (frame.empty()) {
            cerr << "Frame empty!" << endl;
            break;
        }

        // 그레이스케일 영상으로 변환
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // 이진 영상 (Thresholding 적용)
        threshold(gray, th, 128, 255, THRESH_BINARY);

        writer1 << frame;  // 컬러 영상
        writer2 << gray;   // 그레이스케일 영상
        writer3 << th;     // 이진 영상

        tm.stop();  // 실행 시간 측정 종료
        cout << "Frame processing time: " << tm.getTimeMilli() << " ms" << endl;
        tm.reset();  // 실행 시간 초기화

        // 30ms 대기
        if (waitKey(30) >= 0) {
            break;
        }
    }

    return 0;
}