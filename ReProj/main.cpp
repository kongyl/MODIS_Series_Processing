/*
 * main.cpp
 *
 *  Created on: 2014年11月27日
 *      Author: kongyl
 */

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "ogr_spatialref.h"
using namespace std;

int main(int argc, char* argv[])
{
	// test args
	if (argc != 4)
	{
		cout << "Arguments error!" << endl;
		cout << "Please input: " << endl;
		cout << "./ReProj inputPath outputPath Resolution" << endl;
		cout << "inputPath: the path of input geotiff files." << endl;
		cout << "outputPath: the path of output geotiff files." << endl;
		cout << "Resolution: spatial resolution of output data." << endl;
		return -1;
	}
	// get args
	string inPath = argv[1];
	if (inPath[inPath.length() - 1] != '/')
		inPath = inPath + "/";
	string outPath = argv[2];
	if (outPath[outPath.length() - 1] != '/')
		outPath = outPath + "/";
	string resolution = argv[3];
	// test path
	DIR* inD;
	DIR* outD;
	if (!(inD = opendir(inPath.c_str())))
	{
		cout << "Cannot open path: " << inPath << endl;
		cout << "Please input: " << endl;
		cout << "./ReProj inputPath outputPath Resolution" << endl;
		cout << "inputPath: the path of input geotiff files." << endl;
		cout << "outputPath: the path of output geotiff files." << endl;
		cout << "Resolution: spatial resolution of output data." << endl;
		return -2;
	}
	if (!(outD = opendir(outPath.c_str())))
	{
		cout << "Cannot open path: " << outPath << endl;
		cout << "Please input: " << endl;
		cout << "./ReProj inputPath outputPath Resolution" << endl;
		cout << "inputPath: the path of input geotiff files." << endl;
		cout << "outputPath: the path of output geotiff files." << endl;
		cout << "Resolution: spatial resolution of output data." << endl;
		return -2;
	}
	int res = atoi(resolution.c_str());
	if (res == 0)
	{
		cout << "The Resolution should be a positive integer" << endl;
		cout << "Please input: " << endl;
		cout << "./ReProj inputPath outputPath Resolution" << endl;
		cout << "inputPath: the path of input geotiff files." << endl;
		cout << "outputPath: the path of output geotiff files." << endl;
		cout << "Resolution: spatial resolution of output data." << endl;
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
		if (fType != ".tif")
			continue;
		cout << "processing file " << i++ << endl;
		string inFile = inPath + fileName;
		string outFile = outPath + fileName;

		GDALDataset* inDataset = (GDALDataset*)GDALOpen(inFile.c_str(), GA_ReadOnly);
		GDALDataType inDataType = GDALGetRasterDataType(GDALGetRasterBand(inDataset, 1));
		GDALDriver* outDriver = (GDALDriver*)GDALGetDriverByName("GTiff");
		// set proref
		const char* inWKT = GDALGetProjectionRef(inDataset);
		OGRSpatialReference oSR;
		oSR.SetUTM(51, TRUE);
		oSR.SetWellKnownGeogCS("WGS84");
		char* outWKT;
		oSR.exportToWkt(&outWKT);

		void* transArg = GDALCreateGenImgProjTransformer(inDataset, inWKT, NULL, outWKT, FALSE, 0, 1);
		double outGeoTrans[6];
		int width, height;
		GDALSuggestedWarpOutput(inDataset, GDALGenImgProjTransform, transArg, outGeoTrans, &width, &height);
		int newWidth = int(outGeoTrans[1] * width / res + 0.5);
		int newHeight = int(-outGeoTrans[5] * height / res + 0.5);
		outGeoTrans[1] = res;
		outGeoTrans[5] = -res;

		GDALDataset* outDataset = (GDALDataset*)GDALCreate(outDriver, outFile.c_str(), newWidth, newHeight, 1, inDataType, NULL);
		outDataset->SetProjection(outWKT);
		outDataset->SetGeoTransform(outGeoTrans);

		GDALWarpOptions* warpOpt = GDALCreateWarpOptions();
		warpOpt->hSrcDS = inDataset;
		warpOpt->hDstDS = outDataset;
		warpOpt->nBandCount = 1;
		warpOpt->panSrcBands = (int*)CPLMalloc(sizeof(int) * warpOpt->nBandCount);
		warpOpt->panSrcBands[0] = 1;
		warpOpt->panDstBands = (int*)CPLMalloc(sizeof(int) * warpOpt->nBandCount);
		warpOpt->panDstBands[0] = 1;
		warpOpt->pfnProgress = GDALTermProgress;
		warpOpt->pTransformerArg = GDALCreateGenImgProjTransformer(inDataset, inWKT, outDataset, outWKT, FALSE, 0, 1);
		warpOpt->pfnTransformer = GDALGenImgProjTransform;
		warpOpt->eResampleAlg = GRA_NearestNeighbour;

		GDALWarpOperation opera;
		opera.Initialize(warpOpt);
		opera.ChunkAndWarpImage(0, 0, newWidth, newHeight);
		GDALFlushCache(outDataset);

		GDALDestroyGenImgProjTransformer(transArg);
		GDALDestroyGenImgProjTransformer(warpOpt->pTransformerArg);
		GDALDestroyWarpOptions(warpOpt);
		CPLFree(outWKT);
		GDALClose(inDataset);
		GDALClose(outDataset);
	}

	return 0;
}
