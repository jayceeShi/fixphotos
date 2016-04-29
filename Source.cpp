  #undef DBG_NEW
#include <math.h>    
//想要写一个东西能够完成把很多照片写成一张的功能
//刚开始打算匹配一下颜色，然后判断那张图片适合哪个区域
//后来果然是嫌烦了……于是就简单用柔光模式混合一下底色和掩膜层.底色是马赛克效果过后的一张照片
//纯粹柔光的话会出现覆盖层的问题，所以可以把背景色改成和需要覆盖的小图层按照比例混合然后再柔光（虽然我没有）
//至于掩膜层怎么做。。。照片大小不一致，但是不想做太多前提准备比如切割照片，所以就转化成了一个如何用一堆长方形拼成一个大长方形的问题
//显然我懒得解┑(￣Д ￣)┍
//于是干脆同意了一下长宽比例。。。果然还是偷懒
//效果其实还可以吧？和正常ps出来的效果差不多。。。
//假如有兴趣的话可以再改一改，比如，一开始可以把底色图片马赛克化然后每一格是一个小图片，可能效果更好；还有根据底膜的深度来分配小照片
//然而，我懒
//sa，撩妹神器1.0
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgcodecs.hpp"
#include <Windows.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <algorithm>
using namespace cv;
using namespace std;
void local(Mat & src, int cols, int rows) {
	long long int**re;
	Mat tmpp = Mat::zeros(src.size(), src.type());
	const int ro = src.rows;
	const int co = src.cols;
	blur(src, tmpp, Size(2 * 10 + 1, 2 * 20 + 1), Point(-1, -1));
	//将src中值滤波
	re = new long long int*[ro];
	int nc = src.channels();
	for (int i = 0; i < ro; i++)
		re[i] = new long long int[co * nc];
	//	Mat del = tmpp(Range(up, dw), Range(lu, ru));
	for (int i = 0; i < ro; i++) {
		const uchar* I1 = src.ptr<uchar>(i);
//		const uchar* I2 = tmpp.ptr<uchar>(i);

		for (int j = 0; j < co; j++) {
			//			if (I1[j * 3] == 128 && I1[j * 3 + 1] == 128 && I1[j * 3 + 2] == 128)continue;
			if (i == 0) {
				if (j == 0) {
					for (int c = 0; c < nc; c++)
						re[i][j * nc + c] = I1[j * nc + c];
				}

				else {
					for (int c = 0; c < nc; c++)
						re[i][j * nc + c] = re[i][(j - 1) * nc + c] + I1[j * nc + c];
				}
			}
			else {
				if (j == 0)
					for (int c = 0; c < nc; c++)
						re[i][j * nc + c] = re[i - 1][j * nc + c] + I1[j * nc + c];
				else
					for (int c = 0; c < nc; c++)
						re[i][j * nc + c] = re[i - 1][j * nc + c] + re[i][(j - 1) * nc + c] - re[i - 1][(j - 1) * nc + c] + I1[j * nc + c];

			}
		}
	}
	//int nc = src.channels();
	for (int i = 0; i < ro - rows; i+=rows) {
//		const uchar* I1 = src.ptr<uchar>(i);
		//		uchar* I2 = tmpp2.ptr<uchar>(i);
//		const uchar* I3 = tmpp.ptr<uchar>(i);

		for (int j = 0; j < co - cols; j+=cols)
			for (int c = 0; c < nc; c++) {
				long long int colo = re[i+rows][j*nc+c+cols*nc] - re[i][j*nc+c+cols*nc] - re[i+rows][j*nc+c] + re[i][j*nc+c];
				colo = colo / (cols * rows);

				for (int ii = i; ii <= i + rows; ii++) {
					uchar * ds = src.ptr<uchar>(ii);
					for (int jj = j*nc + c; jj <= j*nc + c + nc*cols - nc; jj += nc) {
						ds[jj] = colo;
					}
				}

			}
	}

}
//构造一张积分图，用来做成马赛克
//不想要的话就把这个函数注释掉然后下文柔光融合函数的don改成e2

void merge(Mat* src, Mat& dst, int cols, int rows, int x, int y) {
	Mat tmp[16];
	for (int i = 0; i < 16; i++) {
		tmp[i] = src[i].clone();
		resize(src[i], tmp[i], Size(cols, rows), 0, 0);
	}
	RNG rng;
	for (int i = 0; i < dst.cols - cols; i += cols)
		for (int j = 0; j < dst.rows - rows; j += rows) {

			tmp[rng.uniform(0,16)].copyTo(dst(Rect(Point(i, j), Point(i + cols, j + rows))));
		}
}
int main() {
	Mat e[18];
	e[0] = imread("e1.jpg");
	e[1]=imread("e3.jpg");
	e[2] = imread("e4.jpg");
	e[3] = imread("e5.jpg");
	e[4] = imread("e6.jpg");
	e[5] = imread("e7.jpg");
	e[6] = imread("e8.jpg");
	e[7] = imread("e9.jpg");
	e[8] = imread("e10.jpg");
	e[9] = imread("e11.jpg");
	e[10] = imread("e12.jpg");
	e[11] = imread("e13.jpg");
	e[12] = imread("e14.jpg");
	e[13] = imread("e15.jpg");
	e[14] = imread("e16.jpg");
	e[15] = imread("e17.jpg");

	Mat e2 = imread("e2.jpg");
	Mat don = e2.clone();
	double rate = e[0].cols * 1.0 / e[0].rows;
	for (int i = 1; i < 16; i++) {
  		if (e[i].rows < e[i].cols) {
			transpose(e[i], e[i]);
			flip(e[i], e[i], 1);
		}

		resize(e[i], e[i], e[0].size());
	}
	resize(e[1], e[1], e[0].size());
	resize(e[2], e[2], e[0].size());
	resize(e[3], e[3], e[0].size());
	int x = 200;//每一列图片的个数，讲道理的话基本上越多越好
	int y = x / rate;
	Mat out = Mat::zeros(e2.size(), e2.type());

	int cols2 = e2.cols / x;
	int rows2 = cols2 / rate;

	merge(e, out, cols2, rows2, x, y);

	local(don, cols2, rows2);
	//out = out * 0.8 + don * 0.2可以这样先稍微混合一下
	Mat dst = Mat::zeros(e2.size(), e2.type());
	for (int i = 0; i < e2.rows; ++i)
	{
		const uchar* srcc = (const uchar*)(don.data + don.step*i);
		const uchar* srcc2 = (const uchar*)(out.data + out.step * i);
		uchar* dstt = (uchar*)(dst.data + dst.step*i);
		for (int j = 0; j < e2.cols * 3; ++j)
		{

			dstt[j] = saturate_cast<uchar>(srcc2[j] < 128 ? ((srcc[j] * srcc2[j]*1.0)/128 + (srcc[j] *1.0/ 255) *(srcc[j]*1.0/255) *(255 - 2*srcc2[j])) : ((srcc[j]*(255 - srcc2[j])) *1.0/ 128 + sqrt(srcc[j]*1.0 / 255)*(2*srcc2[j] - 255)));
		}
	}
	namedWindow("haveatry");
	imshow("haveatry", dst);
	waitKey();
	imwrite("16.jpg",dst);
	return 0;
}