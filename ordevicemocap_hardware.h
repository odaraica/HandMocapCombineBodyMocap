//#ifndef __ORDEVICE_SKELETON_KINECT_H__
//#define __ORDEVICE_SKELETON_KINECT_H__
#ifndef ORDEVICEMOCAP_HARDWARE_H

#define ORDEVICEMOCAP_HARDWARE_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS

#include "DataOp.h"
//#include <fbsdk/fbsdk.h>
#include "statushandle.h"

struct SkeletonNodeInfo
{
	const char* mName;
	int mParentChannel;
	double mDefaultT[3];
	double mDefaultR[3];
	double mT[3];
	double mR[3];
};

DWORD WINAPI ReceiveData(LPVOID lpParam);
void neterror(int typ);
struct ReceiveDataPram
{
	SOCKADDR_IN              addr;
	SOCKET                   sendSocket;
	char*                    recvBufferP;
	SkeletonNodeInfo*        writeBodyChannel;
	bool                     isWhile;
};

class ORDeviceMocapGVP
{
public:

	//! Constructor & destructor
	ORDeviceMocapGVP();
	~ORDeviceMocapGVP();

	//--- Opens and closes Mocap Hardware. returns true if successful
	bool    Init();
	bool    Open();//!< Open Mocap Hardware
	bool    Close();//!< Close Mocap Hardware
	bool    setAddrPort(const char* addr,int size,int p);
	bool    GetSetupInfo();//!< Get the setup information.

	//--- Hardware communication
	bool    StartDataStream();//!< Put the device in streaming mode.
	bool    StopDataStream();//!< Take the device out of streaming mode.


	DataFilter* currentFrame;
	DataFilter* nextFrame;

	bool      FetchMocapData(FBTime& pTime);//!< Fetch a data from hardware.
	//bool    FetchBodyJointData();
	void      GetBodySkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder);
	// void    GetRHandSkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder);
	// void    GetLHandSkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder);

	FBString   GetHardWareName();
	//double     GetAverageSensorFloorOffset();
	//void       SetSensorFloorOffsetSet();

	//--- Channel & Channel data management
	int         GetChannelCount();
	const char* GetChannelName(int pChannel);
	int         GetChannelParent(int pChannel);
	FBVector3d  GetDefaultT(int pChannel);
	FBVector3d  GetDefaultR(int pChannel);
	void        SetDefaultT(int pChannel,double x,double y,double z);
	void        SetDefaultR(int pChannel,double x,double y,double z);
	void        SetDataT(int pChannel,double x,double y,double z);
	void        SetDataR(int pChannel,double x,double y,double z);
	vector<double>GetDataT(int pChannel);
	vector<double>GetDataR(int pChannel);
	
	void        MRtoMT();
	
	//bool        AdjustAngle();
	//inline void CalculateAverageSensorFloorOffset();


	// Speech
	//bool           SetupSpeechConnection();
	//SpeechCommands ProcessSpeech();

private:


	SkeletonNodeInfo        mBodyChannel[53];
	SkeletonNodeInfo        mRHandChannel[20];
	SkeletonNodeInfo        mLHandChannel[20];
	int                     mBodyChannelCount;
	int                     mRHandChannelCount;
	int                     mLHandChannelCount;

	bool                    mCharactorized;
	bool                    mInitSuccessful;
	bool                    mOpened;
	bool                    sensorState;
	bool                    mDataStreamStart;
	bool                    mDataStreamStop;
	int                     wsaSig;
	WSADATA                 wsaData;
	HANDLE                  m_hNextSkeletonEvent;
	SOCKET                  sendSocket;
	SOCKADDR_IN             Addr;
	
	//multiThread
	HANDLE                  ReceiveTh[2];
	DWORD                   dwThreadID[2];
	ReceiveDataPram         RDpram[2];
	char                    addrInput[32];
	int                     port;
	char                    sendBuffer[1024];
	char                    recvBuffer[1024];
	int                     socketSig;
	int                     sendSig;
	int                     recvSig;
	bool                    loopRecv;
	double                  mAverageSensorFloorOffset;
	bool                    mSensorFloorOffsetSet;

	// Speech
	//ISpeech*                mSpeech;
	//bool                    mHasVoiceControl;

public:
	FBMocapJointsState*    MocapJointsState;
};







#endif /* __ORDEVICE_SKELETON_KINECT_H__ */
