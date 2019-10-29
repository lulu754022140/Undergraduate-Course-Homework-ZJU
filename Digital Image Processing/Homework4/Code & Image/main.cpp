#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

float sigma = 0.002f; // ��˹����������
float K = 0.002f; // ά���˲�ϵ��

// ����Box - Muller�㷨������˹�����
float GenerateGaussianNoise()
{
	static bool flag = false;
	static float rand1, rand2;

	if (flag)
	{
		flag = false;
		return sqrt(-2.0f * log(rand1)) * cos(2.0f * (float)CV_PI * rand2);
	}

	flag = true;

	rand1 = rand() / ((float)RAND_MAX);
	rand2 = rand() / ((float)RAND_MAX);
	if (rand1 < 1e-10)
		rand1 = (float)1e-10;
	if (rand2 < 1e-10)
		rand2 = (float)1e-10;

	return sqrt(-2.0f * log(rand1)) * sin(2.0f * (float)CV_PI * rand2);
}

void AddGaussianNoise(Mat& Image)
{
	for (int i = 0; i < Image.rows; ++i) {
		for (int j = 0; j < Image.cols; ++j) {
			float value = Image.at<float>(i, j) + GenerateGaussianNoise() * sigma;
			if (value > 1.0f)
				value = 1.0f;
			if (value < 0.0f)
				value = 0.0f;
			Image.at<float>(i, j) = value;
		}
	}
}

int main()
{
	Mat image = imread("ԭͼ��.png", CV_8UC1);

	// ��ȡ��Ѹ���Ҷ�任�ߴ�
	int optM = getOptimalDFTSize(image.rows);
	int optN = getOptimalDFTSize(image.cols);
	
	// �߽���չ
	Mat expandedImage;
	copyMakeBorder(image, expandedImage, 0, optM - image.rows, 0, optM - image.cols, BORDER_CONSTANT, Scalar::all(0));

	// ת�ɸ���ͼ��
	expandedImage.convertTo(expandedImage, CV_32FC1);
	normalize(expandedImage, expandedImage, 1, 0, CV_C);
	imshow("ԭͼ��", expandedImage);

	// ʹƵ��ƽ�Ƶ�����
	for (int i = 0; i < expandedImage.rows; i++) {
		for (int j = 0; j < expandedImage.cols; j++) {
			expandedImage.at<float>(i, j) *= (float)pow(-1, i + j);
		}
	}

	// ����˫ͨ����ʵ�����鲿��ͼ��
	Mat planes[] = { Mat_<float>(expandedImage),Mat::zeros(expandedImage.size(),CV_32FC1) };

	// �ϳ�˫ͨ��
	Mat complexImage;
	merge(planes, 2, complexImage);

	// ��˫ͨ��ͼ����и���Ҷ�任
	dft(complexImage, complexImage);

	// �����˶�ģ���任����
	Mat Motion(expandedImage.size(), CV_32FC2);
	Mat DeMotion(expandedImage.size(), CV_32FC2);

	float a = 0.03f;
	float b = 0.03f;
	float T = 1.0f;

	float CutOffBlur = 0.01f;
	float CutOffDeblur = 0.01f;
	for (int i = 0; i < expandedImage.rows; i++) {
		for (int j = 0; j < expandedImage.cols; j++) {
			float temp = (float)CV_PI * ((i - expandedImage.rows / 2) * a + (j - expandedImage.cols / 2) * b);
			if (temp == 0) {
				Motion.at<Vec2f>(i, j)[0] = T;
				Motion.at<Vec2f>(i, j)[1] = T;
				DeMotion.at<Vec2f>(i, j)[0] = T;
				DeMotion.at<Vec2f>(i, j)[1] = T;
			}
			else {
				Motion.at<Vec2f>(i, j)[0] = T / temp * (float)sin(temp)*(float)cos(temp);
				Motion.at<Vec2f>(i, j)[1] = T / temp * (float)sin(temp)*(float)cos(temp);
				DeMotion.at<Vec2f>(i, j)[0] = T / temp * (float)sin(temp)*(float)cos(temp);
				DeMotion.at<Vec2f>(i, j)[1] = T / temp * (float)sin(temp)*(float)cos(temp);
				// �˲���ֵ
				if (Motion.at<Vec2f>(i, j)[0] < CutOffBlur) {
					Motion.at<Vec2f>(i, j)[0] = CutOffBlur;
					Motion.at<Vec2f>(i, j)[1] = CutOffBlur;
				}
				// �˲���ֵ
				if (DeMotion.at<Vec2f>(i, j)[0] < CutOffDeblur) {
					DeMotion.at<Vec2f>(i, j)[0] = CutOffDeblur;
					DeMotion.at<Vec2f>(i, j)[1] = CutOffDeblur;
				}
			}
		}
	}

	Mat Blurred(expandedImage.size(), CV_32FC2);  // ģ��Ƶ��
	Mat BlurredWithNoise(expandedImage.size(), CV_32FC2); // ģ��������Ƶ��
	Mat BlurredImage(expandedImage.size(), CV_32FC1); // ģ��ͼ��
	Mat BlurredWithNoiseImage(expandedImage.size(), CV_32FC1); // ģ��������ͼ��
	Mat Deblurred(expandedImage.size(), CV_32FC2);  // ģ�����˲�Ƶ��
	Mat DeblurredWithNoise(expandedImage.size(), CV_32FC2); // ģ�����������˲�Ƶ��
	Mat DeblurredImage(expandedImage.size(), CV_32FC1); // ģ�����˲�ͼ��
	Mat DeblurredWithNoiseImage(expandedImage.size(), CV_32FC1); // ģ�����������˲�ͼ��
	Mat DeblurredWiener(expandedImage.size(), CV_32FC2);  // ģ��ά���˲�Ƶ��
	Mat DeblurredWienerWithNoise(expandedImage.size(), CV_32FC2); // ģ��������ά���˲�Ƶ��
	Mat DeblurredWienerImage(expandedImage.size(), CV_32FC1); // ģ��ά���˲�ͼ��
	Mat DeblurredWienerWithNoiseImage(expandedImage.size(), CV_32FC1); // ģ��������ά���˲�ͼ��

	split(complexImage, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("ԭͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("ԭͼ��Ƶ��.png", planes[0]);

	multiply(complexImage, Motion, Blurred); // �����˶�ģ��
	
	split(Blurred, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("ģ��ͼ��Ƶ��.png", planes[0]);

	divide(Blurred, DeMotion, Deblurred); // ���˶�ģ��

	split(Deblurred, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("ֱ�ӽ�ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("ֱ�ӽ�ģ��ͼ��Ƶ��.png", planes[0]);

	idft(Deblurred, Deblurred);
	split(Deblurred, planes);
	magnitude(planes[0], planes[1], DeblurredImage);
	normalize(DeblurredImage, DeblurredImage, 1, 0, CV_C);
	imshow("ֱ�ӽ�ģ��ͼ��", DeblurredImage);
	normalize(DeblurredImage, DeblurredImage, 255, 0, CV_C);
	imwrite("ֱ�ӽ�ģ��ͼ��.png", DeblurredImage);

	// ������
	idft(Blurred, Blurred);
	split(Blurred, planes);
	magnitude(planes[0], planes[1], BlurredImage);
	normalize(BlurredImage, BlurredImage, 1, 0, CV_C);
	imshow("ģ��ͼ��", BlurredImage);
	BlurredImage.copyTo(BlurredWithNoiseImage);
	AddGaussianNoise(BlurredWithNoiseImage);
	imshow("������ģ��ͼ��", BlurredWithNoiseImage);
	normalize(BlurredImage, BlurredImage, 255, 0, CV_C);
	imwrite("ģ��ͼ��.png", BlurredImage);
	normalize(BlurredWithNoiseImage, BlurredWithNoiseImage, 255, 0, CV_C);
	imwrite("������ģ��ͼ��.png", BlurredWithNoiseImage);

	for (int i = 0; i < BlurredWithNoiseImage.rows; i++) {
		for (int j = 0; j < BlurredWithNoiseImage.cols; j++) {
			BlurredWithNoiseImage.at<float>(i, j) *= (float)pow(-1, i + j);
		}
	}

	BlurredWithNoiseImage.copyTo(planes[0]);
	planes[1] = Mat::zeros(BlurredWithNoiseImage.size(), CV_32FC1);

	merge(planes, 2, BlurredWithNoise);
	dft(BlurredWithNoise, BlurredWithNoise);

	split(BlurredWithNoise, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("������ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("������ģ��ͼ��Ƶ��.png", planes[0]);
	
	divide(BlurredWithNoise, DeMotion, DeblurredWithNoise);

	split(DeblurredWithNoise, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("�������ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("�������ģ��ͼ��Ƶ��.png", planes[0]);

	idft(DeblurredWithNoise, DeblurredWithNoise);
	split(DeblurredWithNoise, planes);
	magnitude(planes[0], planes[1], DeblurredWithNoiseImage);
	normalize(DeblurredWithNoiseImage, DeblurredWithNoiseImage, 1, 0, CV_C);
	imshow("�������ģ��ͼ��", DeblurredWithNoiseImage);
	normalize(DeblurredWithNoiseImage, DeblurredWithNoiseImage, 255, 0, CV_C);
	imwrite("�������ģ��ͼ��.png", DeblurredWithNoiseImage);

	dft(Blurred, Blurred);
	// ά���˲�
	Mat temp(expandedImage.size(), CV_32FC2);
	Mat MagTransform(expandedImage.size(), CV_32FC2);
	for (int i = 0; i < MagTransform.rows; i++) {
		for (int j = 0; j < MagTransform.cols; j++) {
			MagTransform.at<Vec2f>(i, j)[0] = ((pow(Motion.at<Vec2f>(i, j)[0], 2) + pow(Motion.at<Vec2f>(i, j)[1], 2)) / (pow(Motion.at<Vec2f>(i, j)[0], 2) + pow(Motion.at<Vec2f>(i, j)[1], 2) + K));
			MagTransform.at<Vec2f>(i, j)[1] = ((pow(Motion.at<Vec2f>(i, j)[0], 2) + pow(Motion.at<Vec2f>(i, j)[1], 2)) / (pow(Motion.at<Vec2f>(i, j)[0], 2) + pow(Motion.at<Vec2f>(i, j)[1], 2) + K));
		}
	}
	divide(Blurred, Motion, temp);
	multiply(temp, MagTransform, DeblurredWiener);
	divide(BlurredWithNoise, Motion, temp);
	multiply(temp, MagTransform, DeblurredWienerWithNoise);

	split(DeblurredWiener, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("ά���˲���ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("ά���˲���ģ��ͼ��Ƶ��.png", planes[0]);

	idft(DeblurredWiener, DeblurredWiener);
	split(DeblurredWiener, planes);
	magnitude(planes[0], planes[1], DeblurredWienerImage);
	normalize(DeblurredWienerImage, DeblurredWienerImage, 1, 0, CV_C);
	imshow("ά���˲���ģ��ͼ��", DeblurredWienerImage);
	normalize(DeblurredWienerImage, DeblurredWienerImage, 255, 0, CV_C);
	imwrite("ά���˲���ģ��ͼ��.png", DeblurredWienerImage);

	split(DeblurredWienerWithNoise, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_C);
	imshow("ά���˲��������ģ��ͼ��Ƶ��", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_C);
	imwrite("ά���˲��������ģ��ͼ��Ƶ��.png", planes[0]);

	idft(DeblurredWienerWithNoise, DeblurredWienerWithNoise);
	split(DeblurredWienerWithNoise, planes);
	magnitude(planes[0], planes[1], DeblurredWienerWithNoiseImage);
	normalize(DeblurredWienerWithNoiseImage, DeblurredWienerWithNoiseImage, 1, 0, CV_C);
	imshow("ά���˲��������ģ��ͼ��", DeblurredWienerWithNoiseImage);
	normalize(DeblurredWienerWithNoiseImage, DeblurredWienerWithNoiseImage, 255, 0, CV_C);
	imwrite("ά���˲��������ģ��ͼ��.png", DeblurredWienerWithNoiseImage);

	waitKey();
	return 0;
}