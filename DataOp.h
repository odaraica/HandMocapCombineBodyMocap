#pragma once
#ifndef DATAOP_H
#define DATAOP_H

#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <vector>
#include <string>
#include <fbsdk/fbsdk.h>
#include <cmath>


using namespace std;

#define PI 3.14159265358979323846
// 
// typedef enum BODYNODES
// {
// 	BN_Hips = 0, ///< Hips
// 	BN_RightUpperLeg, ///< Right Upper Leg
// 	BN_RightLowerLeg, ///< Right Lower Leg
// 	BN_RightFoot, ///< Right Foot
// 	BN_RightToe, ///< Right Toe
// 	BN_LeftUpperLeg, ///< Left Upper Leg
// 	BN_LeftLowerLeg, ///< Left Lower Leg
// 	BN_LeftFoot, ///< Left Foot
// 	BN_LeftToe, ///< Left Toe
// 	BN_Spine, ///< Spine
// 	BN_Spine1, ///< Spine1
// 	BN_Spine2, ///< Spine2
// 	BN_Spine3, ///< Spine3 -- Back
// 	BN_Neck, ///< Neck
// 	BN_Head, ///< Head
// 	BN_RightShoulder, ///< Right Shoulder
// 	BN_RightUpperArm, ///< Right Upper Arm
// 	BN_RightLowerArm, ///< Right Lower Arm
// 	BN_RightHand, ///< Right Hand
// 	BN_LeftShoulder, ///< Left Shoulder
// 	BN_LeftUpperArm, ///< Left Upper Arm
// 	BN_LeftLowerArm, ///< Left Lower Arm
// 	BN_LeftHand, ///< Left Hand
// }_BodyNodes_;

template<class T>
static void saveVec(vector<T> InData,const char path[])
{
	ofstream ofs;
	ofs.open(path, ios::out|ios::app);
	for (size_t i=0;i<InData.size();i++)
	{
		ofs<<InData[i]<<endl;
	}
	ofs.close();
}
static void saveStr(string InData,const char path[])
{
	ofstream ofs;
	ofs.open(path, ios::out|ios::app);

	ofs<<InData<<endl;

	ofs.close();
}


typedef struct UDPMsg
{
	int frameHead;
	int frameIndex;
	int frameLength[2];
	int frameOther[3];
	int frameUpdate;
	int frame;
	int frameOther1;
	int frameRate;
	int rootPos[6];
	int sensorState[23];
	int bodyRot[184];
	int RHandState[20];
	int RHandRot[160];
	int LHandState[20];
	int LHandRot[160];
	int frameEndOther[3];
	int frameTail;
	int frameCor;
}UDPData;



class DataFilter
{
public:
	DataFilter();
	~DataFilter();
public:
	UDPData* outputData;
	bool getData();
	bool transData();
	bool dataOpt();
	void checkData();
	int strToInt(char* InData);
	void quatToEuler(double w, double x, double y, double z, double* rotList);
	void quatToEuler1(double w, double x, double y, double z, double* rotList);
	void quatToEuler2(double w, double x, double y, double z, double* rotList);
	int sign(double x);
	double LRotList[16][3];
	double RRotList[16][3];
	vector<int> UDPFrameDec;
	vector<int> UDPFrameDecSave;
	char UDPOriFrame[1024];
};


#endif