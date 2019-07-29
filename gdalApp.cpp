#include "pch.h"
#include <iostream>
#include <gdal.h>
#include "gdal_priv.h"
#include "cpl_conv.h"
#include <Math.h>
using namespace std;
GDALDataset* gdalCreateDataset(const char* path) {
	GDALAllRegister();

	GDALDataset  *dataset = (GDALDataset *)GDALOpen(path, GA_ReadOnly);
	if (dataset == NULL) {
		cout << "Error1" << endl;
		exit(1);
	}
	return dataset;
}

double interpolation(double q11, double q12, double q21, double q22, int C, int R, double c, double r) {
	double y =
		q11 * ((R + 1 - r)*(C + 1 - c)) +
		q12 * ((R + 1 - r)*(c - C)) +
		q21 * ((r - R)*(C + 1 - c)) +
		q22 * ((r - R)*(c - C));
	return y;
}

void getGEoTransform(GDALDataset *dataset,unsigned int &width, unsigned int & height, double & leftUpperX, double &leftUpperY, double &pixelSizeX, double &pixelSizeY)
{
	width = dataset->GetRasterXSize();
	height = dataset->GetRasterYSize();
	
	// Reading image coordinates
	double geoTransform[6];
	if (dataset->GetGeoTransform(geoTransform) == CE_None) {
		leftUpperX = geoTransform[0];
		leftUpperY = geoTransform[3];
		pixelSizeX = geoTransform[1];
		pixelSizeY = geoTransform[5];
	}
	else {
		cout << "Failed read geotransform" << endl;
		exit(1);
	}
}
void readElevationVal(GDALDataset *dataset, double X, double Y, double &rawElevation, double &elevation)
{
	// Get image width and height values in tems of pixels
	unsigned int width, height;// This value cannot be lower than 0 so it's a unsigned int

	double leftUpperX, leftUpperY, pixelSizeX, pixelSizeY;
	getGEoTransform(dataset, width, height, leftUpperX, leftUpperY, pixelSizeX, pixelSizeY);
	
	// Getting the elevation raster band 
	GDALRasterBand  *elevationBand = dataset->GetRasterBand(1);

	double pixelX = 0.0, pixelY = 0.0, pixelX1 = 0.0, pixelY1 = 0.0;
	int intPixelX = 0, intPixelY = 0, intPixelX1 = 0, intPixelY1 = 0;

	pixelX = (X - (pixelSizeX / 2.0) - leftUpperX) / pixelSizeX;
	pixelY = (Y + (pixelSizeX / 2.0) - leftUpperY) / pixelSizeY;
	intPixelX = (int)pixelX;
	intPixelY = (int)pixelY;

	//bilinear interpolation 
	int arr3[4] = { 0,0,0,0 };

	dataset->GetRasterBand(1)->RasterIO(GF_Read, intPixelX, intPixelY, 2, 2, &arr3[0], 2, 2, GDT_Int32, 0, 0);
	int arr1[1];
	int row = (int)((X - leftUpperX) / pixelSizeX);
	int col = (int)((Y - leftUpperY) / pixelSizeY);
	dataset->GetRasterBand(1)->RasterIO(GF_Read, row, col, 1, 1, &arr1[0], 1, 1, GDT_Int32, 0, 0);
	rawElevation = arr1[0];
	elevation = interpolation(arr3[0], arr3[1], arr3[2], arr3[3], intPixelX, intPixelY, pixelX, pixelY);
}
int main() {
	cout << fixed; // for printing exact number
	const char* input = "C:\\Users\\Ensar\\source\\repos\\ConsoleApplication4\\ConsoleApplication4\\vangolu2.tif";
	GDALDataset *dataset = gdalCreateDataset(input);
		
	bool ctrl = true;
	while (ctrl) 
	{
		double rawElev, elev;
		cout << endl;
		double X=0, Y=0;
		cout << "X: ";
		cin >> X;
		cout << "Y: ";
		cin >> Y;
		if (X == 0.0 || Y == 0.0)
		{
			ctrl = false;
			break;
		}
		readElevationVal(dataset, X,Y, rawElev,elev);
		cout << "Raw Elevation: " << rawElev << " Elevation: " << elev << endl;
	}

}