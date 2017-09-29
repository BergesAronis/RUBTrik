// RedBallTracker.cpp

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include<iostream>

using namespace std;
using namespace cv;

char charCheckForEscKey;

Scalar BGRtoHSV(Scalar BGRColor)
{
	double hue;
	double saturation;
	double value;

	double bPrime = BGRColor[0] / 255.0;
	double gPrime = BGRColor[1] / 255.0;
	double rPrime = BGRColor[2] / 255.0;
	double cMin = min(min(bPrime, gPrime), rPrime);
	double cMax = max(max(bPrime, gPrime), rPrime);
	double delta = cMax - cMin;

	if (delta == 0)
		hue = 0;
	else if (cMax == rPrime)
	{
		hue = (gPrime - bPrime) / delta;
		if (gPrime < bPrime)
			hue += 6;
	}
	else if (cMax == gPrime)
		hue = ((bPrime - rPrime) / delta) + 2;
	else
		hue = ((rPrime - gPrime) / delta) + 4;
	hue *= 60;
	if (cMax == 0)
		saturation = 0;
	else
		saturation = delta / cMax;

	value = cMax;

	return Scalar(hue, saturation, value);
}

Scalar determineColor(Scalar inputColor)
{
	Scalar HSV = BGRtoHSV(inputColor);
	double H = HSV[0];
	double S = HSV[1];
	double V = HSV[2];

	if (S < .30)
	{
		if (V > .50)
			return Scalar(255, 255, 255);
		else
			return Scalar(0, 0, 0);
	}
	else
	{
		if ((H > 0 && H < 10) || (H > 330 && H < 360))
		{
			cout << "red";
			return Scalar(0, 0, 255);
		}
		else if (H > 174 && H < 262)
			return Scalar(255, 0, 0);
		else if (H > 83 && H < 139)
			return Scalar(0, 255, 0);
		else if (H > 10 && H < 44)
			return Scalar(0, 128, 255);
		else if (H > 44 && H < 66)
			return Scalar(0, 255, 255);
		else
			return Scalar(0, 0, 0);
	}
}

class CubeSquare
{ 
	public:
		Scalar color;
		Point topLeft;
		Point bottomRight;
		int side;
		int position;

	CubeSquare()
	{
		color = NULL;
		topLeft = Point(0, 0);
		bottomRight = Point(0, 0);
		position = 0;
	}

	CubeSquare(int position0, int squareSize, int imageHeight, int imageWidth)
	{
		color = NULL;
		position = position0;
		int k = position % 9;
		int j = k / 3;
		int i = k % 3;

		topLeft = Point(squareSize*i + imageWidth / 2.0 - (3 / 2.0)*squareSize, -squareSize*j + imageHeight / 2.0 + (3 / 2.0)*squareSize);
		bottomRight = Point(squareSize*i + imageWidth / 2.0 - (1 / 2.0)*squareSize, -squareSize*j + imageHeight / 2.0 + (1 / 2.0)*squareSize);
	}

};
class Cube
{
	CubeSquare* squareHolder;
	int currentSide;
	int squareSize;
	
	public:
	Cube()
	{
		squareHolder = new CubeSquare[54];
		for (int i = 0; i < 54; i++)
		{
			squareHolder[i] = CubeSquare();
		}
		currentSide = 0;
	}

	Cube(int squareSize0, int imageHeight, int imageWidth)
	{
		squareSize = squareSize0;
		squareHolder = new CubeSquare[54];
		for (int i = 0; i < 54; i++)
		{
			squareHolder[i] = CubeSquare(i, squareSize, imageHeight, imageWidth);
		}
		currentSide = 0;
	}

	void drawInputSquares(Mat frame)
	{
		int fill = 1;
		Scalar fillColor = Scalar(0, 0, 255);
		for (int i = currentSide * 9; i < (currentSide+1) * 9; i++)
		{
			if (squareHolder[i].color != Scalar(0, 0, 0))
			{
				fill = -1;
				fillColor = squareHolder[i].color;
			}
			
			rectangle(frame, squareHolder[i].topLeft, squareHolder[i].bottomRight, fillColor, fill);
		}
	}

	void getColors(Mat frame)
	{
		Point centre;
		int totalB;
		int totalG;
		int totalR;

		for (int i = currentSide * 9; i < (currentSide + 1) * 9; i++)
		{
			totalB = 0;
			totalG = 0;
			totalR = 0;

			centre = Point((squareHolder[i].topLeft.x + squareHolder[i].bottomRight.x) / 2.0, (squareHolder[i].topLeft.y + squareHolder[i].bottomRight.y) / 2.0);
			for (int j = -1; j <= 1; j++)
			{
				for (int k = -1; k <= 1; k++)
				{
					Vec3b intensity = frame.at<Vec3b>(centre.y + (squareSize / 4.0)*j, centre.x + (squareSize/4.0)*k);
					totalB += intensity.val[0];
					totalG += intensity.val[1];
					totalR += intensity.val[2];
				}
			}

			//if (i % 9 == 2)
			//{
			//	//string output = to_string(totalB / 9) + "," + to_string(totalG / 9) + "," + to_string(totalR / 9);

			//	//string output = to_string(HSV[0]) + "," + to_string(HSV[1]) + "," + to_string(HSV[2]);
			//	//cout << output << endl;
			//}

			squareHolder[i].color = determineColor(Scalar(totalB / 9, totalG / 9, totalR / 9));
		}
	}
};

int main() {
	VideoCapture capture(0);
	int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);

	/* Create the window */
	namedWindow("Rubix Solver", CV_WINDOW_AUTOSIZE);

	/* Frame container */
	Mat frame;

	Cube cube = Cube(75, height, width);

	/* Global loop */
	while (charCheckForEscKey != 27)
	{
		/* Capture the frame from the webcam */
		capture >> frame;
		if (frame.empty())
			break;

		cube.getColors(frame);
		cube.drawInputSquares(frame);

		/* Show the result */
		imshow("Rubix Solver", frame);

		/* Wait some milliseconds */
		charCheckForEscKey = cv::waitKey(1);
	}
	return(0);
}
