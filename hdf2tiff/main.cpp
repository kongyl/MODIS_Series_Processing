/*
 * main.cpp
 *
 *  Created on: 2014年11月26日
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
		cout << "./hdf2tiff inputPath outputPath datasetNum" << endl;
		cout << "inputPath: the path of input hdf files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		cout << "datasetNum: the number of dataset layer in hdf files" << endl;
		return -1;
	}
	// get args
	string inPath = argv[1];
	if (inPath[inPath.length() - 1] != '/')
		inPath = inPath + "/";
	string outPath = argv[2];
	if (outPath[outPath.length() - 1] != '/')
		outPath = outPath + "/";
	string datasetN = argv[3];
	// test path
	DIR* inD;
	DIR* outD;
	if (!(inD = opendir(inPath.c_str())))
	{
		cout << "Cannot open path: " << inPath << endl;
		cout << "Please input: " << endl;
		cout << "./hdf2tiff inputPath outputPath datasetNum" << endl;
		cout << "inputPath: the path of input hdf files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		cout << "datasetNum: the number of dataset layer in hdf files" << endl;
		return -2;
	}
	if (!(outD = opendir(outPath.c_str())))
	{
		cout << "Cannot open path: " << outPath << endl;
		cout << "Please input: " << endl;
		cout << "./hdf2tiff inputPath outputPath datasetNum" << endl;
		cout << "inputPath: the path of input hdf files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		cout << "datasetNum: the number of dataset layer in hdf files" << endl;
		return -2;
	}
	int dNum = atoi(datasetN.c_str());
	if (dNum == 0 && datasetN != "0")
	{
		cout << "The datasetNum should be an integer" << endl;
		cout << "Please input: " << endl;
		cout << "./hdf2tiff inputPath outputPath datasetNum" << endl;
		cout << "inputPath: the path of input hdf files." << endl;
		cout << "outputPath: the path of output tiff files." << endl;
		cout << "datasetNum: the number of dataset layer in hdf files" << endl;
		return -3;
	}
	// process each file
	struct dirent* inFile;
	GDALAllRegister();
	int i = 1;
	while((inFile = readdir(inD)) != NULL)
	{
		string fileName = inFile->d_name;
		if (fileName.length() < 4)
			continue;
		string fType = fileName.substr(fileName.length() - 4, 4);
		if (fType != ".hdf")
			continue;
		cout << "processing file " << i++ << endl;
		string inFile = inPath + fileName;
		string outFile = outPath + fileName.substr(0, fileName.length() - 4) + ".tif";

		GDALDataset* inDataset = (GDALDataset*)GDALOpen(inFile.c_str(), GA_ReadOnly);
		char** subDatasets = GDALGetMetadata((GDALDatasetH)inDataset, "SUBDATASETS");
		// MCD45A1: 0-burned data
		// MOD13Q1: 0-NDVI 2-EVI 4-VI Quality 6-Red 8-NIR 10-blue 12-MIR 20-day of the year 22-pixel reliability
		// MCD12Q1: 0-IGBP 2-UMD 4-LAI/fPAR 6-NPP/BGC 8-PFT
		// MOD09GA: 2-QA1km 4-SensorZenith 6-SensorAzimuth 10-SolarZenith 12-SolarAzimuth 20-Red 22-NIR 24-Blue 26-Green 28-b5 30-MIR/TM5 32-MIR/TM7 34-QC500m
		// MOD11A1: 0-LST day 2-QC Day 4-view time 6-view angle 8-LST night 10-QC night
		// MOD11A2: 0-LST day
		//cout<<subDatasets[0]<<endl<<subDatasets[2]<<endl<<subDatasets[4]<<endl<<subDatasets[6]<<endl<<subDatasets[8]<<endl<<subDatasets[10]<<endl;
		string tmpStr = subDatasets[dNum];
		tmpStr = tmpStr.substr(tmpStr.find_first_of("=") + 1);
		GDALDataset* subDataset = (GDALDataset*)GDALOpen(tmpStr.c_str(), GA_ReadOnly);

		GDALDriver* outDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		GDALDataset* outDataset = outDriver->CreateCopy(outFile.c_str(), subDataset, FALSE, NULL, NULL, NULL);

		GDALClose(subDataset);
		GDALClose(inDataset);
		GDALClose(outDataset);
	}

	return 0;
}
