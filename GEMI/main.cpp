/*
 * main.cpp
 *
 *  Created on: 2014年11月28日
 *      Author: kongyl
 */

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include "gdal_priv.h"
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	// test args
	if (argc != 4)
	{
		cout << "Arguments error!" << endl;
		cout << "Please input: " << endl;
		cout << "./GEMI redPath nirPath outputPath" << endl;
		cout << "redPath: the path of input red band files." << endl;
		cout << "nirPath: the path of input nir band files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		return -1;
	}
	// get args
	string redPath = argv[1];
	if (redPath[redPath.length() - 1] != '/')
		redPath = redPath + "/";
	string nirPath = argv[2];
	if (nirPath[nirPath.length() - 1] != '/')
		nirPath = nirPath + "/";
	string outPath = argv[3];
	if (outPath[outPath.length() - 1] != '/')
		outPath = outPath + "/";
	// test path
	DIR* redD;
	DIR* nirD;
	DIR* outD;
	if (!(redD = opendir(redPath.c_str())))
	{
		cout << "Cannot open path: " << redPath << endl;
		cout << "Please input: " << endl;
		cout << "./GEMI redPath nirPath outputPath" << endl;
		cout << "redPath: the path of input red band files." << endl;
		cout << "nirPath: the path of input nir band files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		return -2;
	}
	if (!(nirD = opendir(nirPath.c_str())))
	{
		cout << "Cannot open path: " << nirPath << endl;
		cout << "Please input: " << endl;
		cout << "./GEMI redPath nirPath outputPath" << endl;
		cout << "redPath: the path of input red band files." << endl;
		cout << "nirPath: the path of input nir band files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		return -2;
	}
	if (!(outD = opendir(outPath.c_str())))
	{
		cout << "Cannot open path: " << outPath << endl;
		cout << "Please input: " << endl;
		cout << "./GEMI redPath nirPath outputPath" << endl;
		cout << "redPath: the path of input red band files." << endl;
		cout << "nirPath: the path of input nir band files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		return -2;
	}
	// process each file
	struct dirent* redFile;
	GDALAllRegister();
	int i = 1;
	while((redFile = readdir(redD)) != NULL)
	{
		string fileName = redFile->d_name;
		if (fileName.length() < 4)
			continue;
		string fType = fileName.substr(fileName.length() - 4, 4);
		if (fType != ".tif")
			continue;
		cout << "processing file " << i++ << endl;
		string redPathFile = redPath + fileName;
		string nirPathFile = nirPath + fileName; // every file corresponds to the red band
		string outFile = outPath + fileName;

		GDALDataset* redDataset = (GDALDataset*)GDALOpen(redPathFile.c_str(), GA_ReadOnly);
		GDALDataset* nirDataset = (GDALDataset*)GDALOpen(nirPathFile.c_str(), GA_ReadOnly);
		const char* inWKT = redDataset->GetProjectionRef();
		double geoTrans[6];
		redDataset->GetGeoTransform(geoTrans);

		GDALRasterBand* redBand = redDataset->GetRasterBand(1);
		GDALRasterBand* nirBand = nirDataset->GetRasterBand(1);
		int xSize = redBand->GetXSize();
		int ySize = redBand->GetYSize();
		float* dataRed = (float*)CPLMalloc(sizeof(float) * xSize * ySize);
		float* dataNir = (float*)CPLMalloc(sizeof(float) * xSize * ySize);
		float* data = (float*)CPLMalloc(sizeof(float) * xSize * ySize);
		redBand->RasterIO(GF_Read, 0, 0, xSize, ySize, dataRed, xSize, ySize, GDT_Float32, 0, 0);
		nirBand->RasterIO(GF_Read, 0, 0, xSize, ySize, dataNir, xSize, ySize, GDT_Float32, 0, 0);
		cout<<"processing..."<<endl;
		for (int i = 0; i < xSize; i++)
		{
			for (int j = 0; j < ySize; j++)
			{
				// NDVI
				//data[i + j * xSize] = 1.0 * (dataNir[i + j * xSize] - dataRed[i + j * xSize]) / (dataNir[i + j * xSize] + dataRed[i + j * xSize]);
				// GEMI
				float nir = dataNir[i + j * xSize] * 0.0001;
				float red = dataRed[i + j * xSize] * 0.0001;
				float nu = 2 * (nir * nir - red * red) + 1.5 * nir + 0.5 * red;
				float nd = nir + red + 0.5;
				float tn = nu / nd;
				float gemi = tn * (1 - 0.25 * tn) - (red - 0.125) / (1 - red);
				data[i + j * xSize] = gemi;
			}
		}

		cout<<"writing..."<<endl;
		GDALDriver* outDriver = (GDALDriver*)GDALGetDriverByName("GTiff");
		GDALDataset* outDataset = (GDALDataset*)GDALCreate(outDriver, outFile.c_str(), xSize, ySize, 1, GDT_Float32, NULL);
		outDataset->SetProjection(inWKT);
		outDataset->SetGeoTransform(geoTrans);
		GDALRasterBand* outBand = outDataset->GetRasterBand(1);
		outBand->RasterIO(GF_Write, 0, 0, xSize, ySize, data, xSize, ySize, GDT_Float32, 0, 0);

		CPLFree(data);
		CPLFree(dataRed);
		CPLFree(dataNir);
		GDALClose(redDataset);
		GDALClose(nirDataset);
		GDALClose(outDataset);
	}

	return 0;
}


