#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

int main()
{
	Mat image = imread("lena.png", CV_8UC1);
	imshow("origin", image);

	// ��ȡ��Ѹ���Ҷ�任�ߴ�
	int optM = getOptimalDFTSize(image.rows);
	int optN = getOptimalDFTSize(image.cols);

	// �߽���չ
	Mat expandedImage;
	copyMakeBorder(image, expandedImage, 0, optM - image.rows, 0, optM - image.cols, BORDER_CONSTANT, Scalar::all(0));
	
	// ת�ɸ���ͼ��
	expandedImage.convertTo(expandedImage, CV_32FC1);

	// ʹƵ��ƽ�Ƶ�����
	for (int i = 0; i < expandedImage.rows; i++) {
		for (int j = 0; j < expandedImage.cols; j++) {
			expandedImage.at<float>(i,j) *= (float)pow(-1, i + j);
		}
	}

	// ����˫ͨ����ʵ�����鲿��ͼ��
	Mat planes[] = { Mat_<float>(expandedImage),Mat::zeros(expandedImage.size(),CV_32FC1) };

	// �ϳ�˫ͨ��
	Mat complexImage;
	merge(planes, 2, complexImage);

	// ��˫ͨ��ͼ����и���Ҷ�任
	dft(complexImage, complexImage);

	// ����Gauss�任����
	Mat gaussianBlur(expandedImage.size(), CV_32FC2);
	Mat gaussianSharpen(expandedImage.size(), CV_32FC2);

	float D01 = 64; // ��˹��ͨ�˲�����ֹƵ��
	float D02 = 64; // ��˹��ͨ�˲�����ֹƵ��
	for (int i = 0; i < expandedImage.rows; i++) {
		for (int j = 0; j < expandedImage.cols; j++) {
			float D = (float)pow(i - expandedImage.rows / 2, 2) + (float)pow(j - expandedImage.cols / 2, 2);
			gaussianBlur.at<Vec2f>(i, j)[0] = expf(-D / (2 * D01*D01));
			gaussianBlur.at<Vec2f>(i, j)[1] = expf(-D / (2 * D01*D01));
			gaussianSharpen.at<Vec2f>(i, j)[0] = 1 - expf(-D / (2 * D02*D02));
			gaussianSharpen.at<Vec2f>(i, j)[1] = 1 - expf(-D / (2 * D02*D02));
		}
	}

	multiply(complexImage, gaussianBlur, gaussianBlur); // ��˹��ͨ�˲�ģ��ͼ��
	multiply(gaussianBlur, gaussianSharpen, gaussianSharpen); //��˹��ͨ�˲���ģ��ͼ��ԭ

	// ԭͼ��Ƶ��
	split(complexImage, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_MINMAX);
	imshow("spect origin", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_MINMAX);
	imwrite("spect origin.png", planes[0]);

	// ģ��ͼ��Ƶ��
	split(gaussianBlur, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_MINMAX);
	imshow("spect gaussianBlur", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_MINMAX);
	imwrite("spect gaussianBlur.png", planes[0]);

	// ģ����ԭ��ͼ��Ƶ��
	split(gaussianSharpen, planes);
	magnitude(planes[0], planes[1], planes[0]);
	planes[0] += Scalar::all(1);
	log(planes[0], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_MINMAX);
	imshow("spect gaussianSharpen", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_MINMAX);
	imwrite("spect gaussianSharpen.png", planes[0]);

	// ģ����ͼ��
	idft(gaussianBlur, gaussianBlur);
	split(gaussianBlur, planes);
	magnitude(planes[0], planes[1], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_MINMAX);
	imshow("gaussian Blurred", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_MINMAX);
	imwrite("gaussian Blurred.png", planes[0]);

	// ģ����ԭ��ͼ��
	idft(gaussianSharpen, gaussianSharpen);
	split(gaussianSharpen, planes);
	magnitude(planes[0], planes[1], planes[0]);
	normalize(planes[0], planes[0], 1, 0, CV_MINMAX);
	imshow("gaussian Sharpened", planes[0]);
	normalize(planes[0], planes[0], 255, 0, CV_MINMAX);
	imwrite("gaussian Sharpened.png", planes[0]);

	waitKey();
	return 0;
}