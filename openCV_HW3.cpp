#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <cvaux.h>
#include <fstream>
#include <sstream>

using namespace std;

//#define _CAPTURE_FROM_AVI_FILE_

#define OPENCV2411
#ifdef _MSC_VER
#	ifdef OPENCV210
#		pragma comment(lib, "cv210.lib")
#		pragma comment(lib, "cxcore210.lib")
#		pragma comment(lib, "cvaux210.lib")
#		pragma comment(lib, "highgui210.lib")
#	endif
#	ifdef OPENCV220
#		pragma comment(lib, "opencv_core220.lib")
#		pragma comment(lib, "opencv_highgui220.lib")
#	endif
#	ifdef OPENCV231
#		pragma comment(lib, "opencv_core231.lib")
#		pragma comment(lib, "opencv_highgui231.lib")
#		pragma comment(lib, "opencv_imgproc231.lib")
#		pragma comment(lib, "opencv_objdetect231d.lib")
#	endif
#	ifdef OPENCV2411
#		pragma comment(lib, "opencv_core2411d.lib")
#		pragma comment(lib, "opencv_highgui2411d.lib")
#		pragma comment(lib, "opencv_imgproc2411d.lib")
#	endif
#endif
using namespace std;
using namespace cv;

#define SIZE 100
char test[SIZE];

int main(int argc, char* argv[])
{
	VideoCapture capture;
	int width = 0, height = 0;
	Mat frame,frame1; //兩個source
	bool bCapture;
	string win_in("Input"), win_out("Output");
	int flag=0;	//判斷是否有截圖的flag

	//兩種vector
	vector <Point2d> p2d;
	vector <Point3d> p3d;
	//檔案讀入
	fstream fin;
	string str;
	fin.open("lcalib.p2d",ios::in);
	while(fin.getline(test,sizeof(test)))
	{
		str=test;
		stringstream ss;
		double a,b;
		ss<<str;
		ss>>a>>b;
		cout<<a<<" "<<b<<endl;
		p2d.push_back(Point2d(a,b));	//放入vector
	}
	fin.close();
	fin.open("lcalib.p3d",ios::in);
	while(fin.getline(test,sizeof(test)))
	{
		str=test;
		stringstream ss;
		double a,b,c;
		ss<<str;
		ss>>a>>b>>c;
		cout<<a<<" "<<b<<" "<<c<<endl;
		p3d.push_back(Point3d(a,b,c));
	}
	fin.close();

	//C矩陣
	vector <vector<double>> c;	//2*12
	vector<double> c_tmp;	//1*12
	//PPT第42頁
	for (int i=0;i<p2d.size();i++)
	{
		//第一行
		c_tmp.push_back(p3d[i].x);
		c_tmp.push_back(p3d[i].y);
		c_tmp.push_back(p3d[i].z);
		c_tmp.push_back(1);
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(-(p2d[i].x)*(p3d[i].x));
		c_tmp.push_back(-(p2d[i].x)*(p3d[i].y));
		c_tmp.push_back(-(p2d[i].x)*(p3d[i].z));
		c_tmp.push_back(-(p2d[i].x));
		c.push_back(c_tmp);
		c_tmp.clear();
		//第二行
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(0);
		c_tmp.push_back(p3d[i].x);
		c_tmp.push_back(p3d[i].y);
		c_tmp.push_back(p3d[i].z);
		c_tmp.push_back(0);
		c_tmp.push_back(-(p2d[i].y)*(p3d[i].x));
		c_tmp.push_back(-(p2d[i].y)*(p3d[i].y));
		c_tmp.push_back(-(p2d[i].y)*(p3d[i].z));
		c_tmp.push_back(-(p2d[i].y));
		c.push_back(c_tmp);
		c_tmp.clear();
	}
	//算S矩陣
	double s[12][12];
	for (int i=0;i<12;i++)
	{
		for (int j=0;j<12;j++)
		{
			double tmp(0);
			for (int k=0;k<c.size();k++)
			{tmp+=c[k][j]*c[k][i];}
			s[i][j]=tmp;
		}
	}
	Mat myMat,eigenvalue,eigenvector;
	myMat= Mat(12,12,CV_64F,s);
	eigen(myMat,eigenvalue,eigenvector);	//計算矩陣的eigenvalue和eignvector
	//找最小eigenvalue的eigenvector
	cout<<"========================="<<endl;
	cout<<"Eigenvalue:"<<endl;
	for(int i=0;i<eigenvalue.rows;i++)
	{
		for(int j=0;j<eigenvalue.cols;j++)
		{cout<<i<<"\t"<<eigenvalue.at<double>(i,j);}	//印出全部
		cout<<endl;
	}
	double minv=0.0,maxv=0.0;
	double* minp=&minv;
	double* maxp=&maxv;
	minMaxIdx(eigenvalue,minp,maxp);//取最小(&最大)
	cout<<"========================="<<endl;
	cout<<"MIN:\t"<<minv<<endl;	//印出
	cout<<"MAX:\t"<<maxv<<endl;
	//最小的那一個
	double min;
	int num;
	min=eigenvalue.at<double>(0,0);
	for(int i = 1; i < 12; i++)
	{
		if(min>eigenvalue.at<double>(i,0))
		{
			min=eigenvalue.at<double>(i,0);
			num=i;
		}
	}
	//換成3*4的H矩陣
	double h[3][4];
	for(int i=0;i<12;i++)
	{	h[i/4][i%4]=eigenvector.at<double>(min,i);	}
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<4;j++)
		{	cout<<setw(12)<<h[i][j]<<" ";	}
		cout<<endl;
	}

#ifdef _CAPTURE_FROM_AVI_FILE_
	if (argc != 2) {
		fprintf(stderr, "Usage: %s input-avi-filename\n", *argv);
		exit(1);
	}
	char *AviFileName = * ++argv;
	bCapture = capture.open(AviFileName);
#else
	bCapture = capture.open(0);
#endif
	
	return 0;
}