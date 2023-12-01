#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;


vector<Point2f> selectedPoints; // perspective 변환을 위한 점
vector<Point> linePoints; // 수직선을 그리기 위한 점
string direction;

// 마우스 이벤트 함수
void onMousePerspective(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN && selectedPoints.size() < 4) {
        selectedPoints.push_back(Point2f(x, y));

        // 네 개의 점이 선택되면 로그 출력
        if (selectedPoints.size() == 4) {
            cout << "The choice has been made." << endl<<endl;
        }
    }
}

// 변환된 이미지에서 수직선을 그리는 함수
void onMouseDrawLine(int event, int x, int y, int flags, void* userdata) {
    int num =1;
    if (event == EVENT_LBUTTONDOWN) {
        linePoints.push_back(Point(x, y)); // 수직선을 그리기 위한 점 저장

        if (linePoints.size() == 2) { // 두 점이 선택되면 수직선 그리기
            Mat* img = reinterpret_cast<Mat*>(userdata);
            for (const auto& point : linePoints) {
                if(num==1){
                    line(*img, Point(point.x, 0), Point(point.x, img->rows), Scalar(0, 0, 255)); // 수직선 그리기
                    num++;
                }
                else{
                    line(*img, Point(point.x, 0), Point(point.x, img->rows), Scalar(255, 0, 0)); // 수직선 그리기
                    num=1;                
                }
            }
            if(direction =="left"){
                if(linePoints[0].x <linePoints[1].x){
                    putText(*img, "Offside!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
                }
                else{
                    putText(*img, "No Offside!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
                }

            }
            else{
                if(linePoints[0].x <linePoints[1].x){
                    putText(*img, "No Offside!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
                }
                else{
                    putText(*img, "Offside!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);               
                }

            }
            imshow("Click and Draw!", *img);
            linePoints.clear(); // 점들 초기화
        }
    }
}

int main() {
    string file;
    int type;
    int key =-1;
    Mat src;
    VideoCapture cap;
    while(1){
        cout<< endl<<"Please enter 1 for photos and 2 for videos: ";
        cin>> type;
        cout<<"Please enter the file path and name: ";
        cin >> file;
        if(type ==1){ // 이미지일 경우
            // 이미지 로드
            src = imread(file); // 이미지 경로에 맞게 수정하세요.
            if (src.empty()) {
                cout << "no such file" << endl;
                return -1;
            }
            break;
        }
        else if(type ==2){ // 비디오일 경우
            cout<<"Press the space key when you want to do offside analysis."<<endl;
            if(cap.open(file)==0) {
                cout<< "no such file"<<endl;
                return -1;
            }
            while(true){
                cap >> src;
                if(src.empty()){
                    cout<< "End of video."<<endl;
                    return -1;
                }
                imshow("Source Image", src);
                key = waitKey(33);
                if(key == 32){
                    break;
                }
            }
            break;
        }
        else{
            cout<< "Type is entered incorrectly."<<endl;
        }
    }
    // 이미지 창 생성 및 마우스 이벤트 설정
    namedWindow("Source Image");
    imshow("Source Image", src);
    cout<< "Enter 1 if the direction of the attack is on the left and 2 if it is on the right."<<endl;
    key = waitKey(0);
    while(1){
        if(key==49){
            direction = "left";
            break;
        }
        else if (key ==50){
            direction = "right";
            break;
        }
        else{
            cout<< "You entered it incorrectly. Please enter it again."<<endl;
        }
    }

    cout<< endl<<"Click four points.(In the order of upper left, upper right, lower left, and lower right)"<<endl<<"If you want to reset, please enter esc key."<<endl<<endl;
    setMouseCallback("Source Image", onMousePerspective, nullptr);

    // 사용자가 네 점을 선택할 때까지 대기
    while (selectedPoints.size() < 4) {
        Mat temp = src.clone();
        for (const auto& point : selectedPoints) {
            // 선택된 점들에 빨간색 점 표시
            circle(temp, point, 5, Scalar(0, 0, 255), -1);
        }
        imshow("Source Image", temp);
        key = waitKey(1);
        if (key ==27){
            selectedPoints.clear();
        }
    }

    // 사각형의 너비와 높이 계산
    int width, height;
    float w1 = sqrt(pow(selectedPoints[1].x - selectedPoints[0].x, 2) + pow(selectedPoints[1].y - selectedPoints[0].y, 2));
    float w2 = sqrt(pow(selectedPoints[3].x - selectedPoints[2].x, 2) + pow(selectedPoints[3].y - selectedPoints[2].y, 2));
    width = max(int(w1), int(w2));

    float h1 = sqrt(pow(selectedPoints[2].x - selectedPoints[0].x, 2) + pow(selectedPoints[2].y - selectedPoints[0].y, 2));
    float h2 = sqrt(pow(selectedPoints[3].x - selectedPoints[1].x, 2) + pow(selectedPoints[3].y - selectedPoints[1].y, 2));
    height = max(int(h1), int(h2));

    // 대상 이미지의 네 개의 점
    vector<Point2f> dstPoints;
    dstPoints.push_back(Point2f(0, 0));
    dstPoints.push_back(Point2f(width - 1, 0));
    dstPoints.push_back(Point2f(0, height - 1));
    dstPoints.push_back(Point2f(width - 1, height - 1));

    // 퍼스펙티브 변환 매트릭스 계산
    Mat perspectiveMatrix = getPerspectiveTransform(selectedPoints, dstPoints);

    // 퍼스펙티브 변환 적용
    Mat dst;
    warpPerspective(src, dst, perspectiveMatrix, Size(width, height));

    // 결과 이미지 표시 및 마우스 콜백 설정
    namedWindow("Click and Draw!");
    cout<< "Click two point.(The first is the attacker, the second is the defender.)"<<endl<<"If you want to reset, please enter esc key"<<endl<<endl;
    setMouseCallback("Click and Draw!", onMouseDrawLine, &dst);

    // 사용자가 두 점을 선택할 때까지 대기
    while (linePoints.size() < 2) {
        Mat temp = dst.clone();
        for (const auto& point : linePoints) {
            // 선택된 점들에 빨간색 점 표시
            circle(temp, point, 5, Scalar(0, 0, 255), -1);
        }
        imshow("Click and Draw!", temp);
        key = waitKey(1);
        if (key ==27){
            linePoints.clear();
        }
    }
    waitKey(0);

    return 0;
}