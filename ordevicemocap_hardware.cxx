

/**    \file    ordevicemocap_hardware.cxx
*    Definition of a virtual hardware class.
*    Contains the definition of the functions for the
*    ORDeviceMocapKinect class.
*/

//--- Class declaration
#include "ordevicemocap_hardware.h"
#include "speech/speech.h"


static FBMatrix RotationMatrix4ToFBMatrix(Matrix4 pMatrix4);
static FBTVector PositionVector4ToFBTVector(Vector4 pVector4);

/************************************************
 *    Constructor.
 ************************************************/
ORDeviceMocapGVP::ORDeviceMocapGVP() :
    mBodyChannelCount(0),
    sensorState(NULL),
    mDataStreamStart(false),
    m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
    mOpened(false),
    loopRecv(true),
    MocapJointsState(NULL),
    mAverageSensorFloorOffset(0.0),
    mSensorFloorOffsetSet(false),
    wsaSig(0),
    mInitSuccessful(false),
	port(7000),
	socketSig(0),
	sendSig(0),
	recvSig(0),
	mCharactorized(false),
	mDataStreamStop(true)
	//     mHasVoiceControl(false),
	//     mSpeech(NULL)
	{
	memset(addrInput,0,32);
	memset(sendBuffer,0,1024);
	memset(recvBuffer,0,1024);

	}


/************************************************
 *    Destructor.
 ************************************************/
ORDeviceMocapGVP::~ORDeviceMocapGVP()
{
//     if (mSpeech)
//     {
//         mSpeech->Stop();
//         delete mSpeech;
//     }

//     if (mNuiSensor)
//     {
//         // There is a unresolved issue, Freezing due to that sensor won't shut down properly by NuiShutdown.
//         // This possibly be resolved by Register dummy device status callback by NuiSetDeviceStatusCallback, But didn't work.
//         //mNuiSensor->NuiShutdown();
//     }
    if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(m_hNextSkeletonEvent);
    }
    if (sensorState)
    {
        sensorState = NULL;
    }
    if (MocapJointsState)
    {
        delete MocapJointsState;
    }
    if (mOpened)
    {
        mOpened = false;
    }
    if (mDataStreamStart)
    {
        mDataStreamStart = false;
    }
    if (loopRecv)
    {
        loopRecv = false;
    }
	if (mDataStreamStop)
	{
		mDataStreamStop = true;
	}
}


bool ORDeviceMocapGVP::setAddrPort(const char* addr,int size,int p)
{
	__try
	{
		memcpy(addrInput,addr,size);
		port = p;
		mCharactorized = true;

		return true;	
	}
	__except(puts("error"), EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

bool ORDeviceMocapGVP::Init()
{
	if (mCharactorized)
	{
		__try
		{
			if (sendBuffer>0)
			{
				memset(sendBuffer, 0, 1024);
			}
			if (recvBuffer>0)
			{
				memset(recvBuffer, 0, 1024);
			}
			if (sendSig!=0)
			{
				sendSig = 0;
			}
			if (recvSig!=0)
			{
				recvSig = 0;
			}
			if (!loopRecv)
			{
				loopRecv = true;
			}
			mInitSuccessful = true;
		}
		__except (puts("error"), EXCEPTION_EXECUTE_HANDLER)
		{
			StatusHandle::One()->PopMessage(eSTATUS_MOCAP_HARDWARE_INIT_ERROR);
			mInitSuccessful = false;
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
    
}

/************************************************
 *    Open device communications.
 ************************************************/
bool ORDeviceMocapGVP::Open()
{
    if (mInitSuccessful)
    {
		socketSig = WSAStartup(MAKEWORD(2, 1), &wsaData);
		if ( socketSig !=0)
		{
			return false;
		}
		else
		{
			//StatusHandle::One()->PopMessage(eSTATUS_MOCAP_HARDWARE_SOCKET_CREATE_SUCCESS);
			mOpened = true;
			return true;
		}
		
    }

}


/************************************************
 *    Start data streaming from device.
 ************************************************/
bool ORDeviceMocapGVP::StartDataStream()
{
	mDataStreamStop = false;

    if (!mOpened)
    {
        return false;
    }
	sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	Addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.239");
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(7000);
    unsigned char firstBuffer[17] = { 0xfa,0x00,0x00,0x0b,0x04,0x03,0xa2,0x53,0x23,0x52,0xce,0x32,0x99,0xf4,0x32,0xfb,0x30 };
    //memcpy(sendBuffer, firstBuffer, sizeof(firstBuffer));
    sendSig = sendto(sendSocket, (char*)firstBuffer, sizeof(firstBuffer), 0, (SOCKADDR*)&Addr, sizeof(SOCKADDR));
    if (sendSig>0)
    {
		StatusHandle::One()->PopMessage(eSTATUS_MOCAP_HARDWARE_REQUEST_SEND_SUCCESS);

		mDataStreamStart = true;
    }
    else
    {
		int err = WSAGetLastError();
		neterror(err);
		//StatusHandle::One()->PopMessage(eSTATUS_MOCAP_DEBUG);
       return false;
    }
	//multiThread parm set
	RDpram[0].addr = Addr;
	RDpram[0].isWhile = false;
	RDpram[0].recvBufferP = recvBuffer;
	RDpram[0].sendSocket = sendSocket;
	RDpram[0].writeBodyChannel = mBodyChannel;

	//multiThread
	
	if (ReceiveTh[0] = CreateThread(NULL,0,ReceiveData,&RDpram,0,&dwThreadID[0]))
	{
		return true;
	}
	else
	{
		return false;
	}
	return true;
}

//start receive Data
void neterror(int typ)
{
	switch (typ)
	{
	case 0:
		FBMessageBox("Error:","Directly send error","OK");
		break;
	case 10004:
		FBMessageBox("Error:","10004","OK");
		break;
	case 10013:
		FBMessageBox("Error:","10013","OK");
		break;
	case 10014:
		FBMessageBox("Error:","10014","OK");
		break;
	case 10022:
		FBMessageBox("Error:","10022","OK");
		break;
	case 10024:
		FBMessageBox("Error:","10024","OK");
		break;
	case 10035:
		FBMessageBox("Error:","10035","OK");
		break;
	case 10036:
		FBMessageBox("Error:","10036","OK");
		break;
	case 10037:
		FBMessageBox("Error:","10037","OK");
		break;
	case 10038 :
		FBMessageBox("Error:","10038","OK");
		break;
	case 10039:
		FBMessageBox("Error:","10039","OK");
		break;
	case 10040:
		FBMessageBox("Error:","10040","OK");
		break;
	case 10041:
		FBMessageBox("Error:","10041","OK");
		break;
	case 10042:
		FBMessageBox("Error:","10042","OK");
		break;
	case 10043:
		FBMessageBox("Error:","10043","OK");
		break;
	case 10044:
		FBMessageBox("Error:","10044","OK");
		break;
	case 10045:
		FBMessageBox("Error:","10045","OK");
		break;
	case 10046:
		FBMessageBox("Error:","10046","OK");
		break;
	case 10047:
		FBMessageBox("Error:","10047","OK");
		break;
	case 10048:
		FBMessageBox("Error:","10048","OK");
		break;
	case 10049:
		FBMessageBox("Error:","10049","OK");
		break;
	case 10050:
		FBMessageBox("Error:","10050","OK");
		break;
	case 10051:
		FBMessageBox("Error:","10051","OK");
		break;
	case 10052:
		FBMessageBox("Error:","10052","OK");
		break;
	case 10053:
		FBMessageBox("Error:","10053","OK");
		break;
	case 10054:
		FBMessageBox("Error:","10054","OK");
		break;
	case 10055:
		FBMessageBox("Error:","10055","OK");
		break;
	case 10056:
		FBMessageBox("Error:","10056","OK");
		break;
	case 10057:
		FBMessageBox("Error:","10057","OK");
		break;
	case 10058:
		FBMessageBox("Error:","10058","OK");
		break;
	case 10060:
		FBMessageBox("Error:","10060","OK");
		break;
	case 10061:
		FBMessageBox("Error:","10061","OK");
		break;
	case 10064:
		FBMessageBox("Error:","10064","OK");
		break;
	case 10065:
		FBMessageBox("Error:","10065","OK");
		break;
	case 10067:
		FBMessageBox("Error:","10067","OK");
		break;
	case 10091:
		FBMessageBox("Error:","10091","OK");
		break;
	case 10092:
		FBMessageBox("Error:","10092","OK");
		break;
	case 10093:
		FBMessageBox("Error:","10093","OK");
		break;
	case 10094:
		FBMessageBox("Error:","10094","OK");
		break;
	case 11001:
		FBMessageBox("Error:","11001","OK");
		break;
	case 11002:
		FBMessageBox("Error:","11002","OK");
		break;
	case 11003:
		FBMessageBox("Error:","11003","OK");
		break;
	case 11004:
		FBMessageBox("Error:","11004","OK");
		break;
	default:
		break;
	}
}

/************************************************
 *    Close device communications.
 ************************************************/
bool ORDeviceMocapGVP::Close()
{
    if (mOpened)
    {
		closesocket(sendSocket);
        WSACleanup();
        mOpened = false;
        if (mDataStreamStart)
        {
            mDataStreamStart = false;
        }
    }
    return true;
}


/************************************************
 *    Stop data streaming from device.
 ************************************************/
bool ORDeviceMocapGVP::StopDataStream()
{
	mDataStreamStop = true;


    if (mOpened&&mDataStreamStart)
    {
		CloseHandle(ReceiveTh[0]);
        unsigned char closeBuffer[9] = {0xfa,0x00,0x00,0x03,0x04,0x0b,0xa1,0xfb,0xa8};
        memcpy(sendBuffer, closeBuffer, 9);
        sendSig = sendto(sendSocket, (char*)sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&Addr, sizeof(SOCKADDR));
		if (sendSig < 0)
		{
			return false;
		}
		else
		{
			StatusHandle::One()->PopMessage(eSTATUS_MOCAP_HARDWARE_CLOSE_SEND_SUCCESS);
			mOpened = false;
			mDataStreamStart = false;
			return true;
		}
    }
    return false;
}


/************************************************
 *    Get device setup information.
 ************************************************/
bool ORDeviceMocapGVP::GetSetupInfo()
{
    mBodyChannelCount = 53;

    //0:Hips
    mBodyChannel[0].mName = "Hips";
    mBodyChannel[0].mParentChannel = -1;
    mBodyChannel[0].mT[0] = 0.0;
    mBodyChannel[0].mT[1] = 111.0;
    mBodyChannel[0].mT[2] = 0.0;
    mBodyChannel[0].mR[0] = 0.0;
    mBodyChannel[0].mR[1] = 0.0;
    mBodyChannel[0].mR[2] = 0.0;

    //1:Right Upper Leg
    mBodyChannel[1].mName = "RightUpLeg";
    mBodyChannel[1].mParentChannel = 0;
    mBodyChannel[1].mT[0] = -10.55;
    mBodyChannel[1].mT[1] = 102.19;
    mBodyChannel[1].mT[2] = -0.86;
    mBodyChannel[1].mR[0] = 0.0;
    mBodyChannel[1].mR[1] = 0.0;
    mBodyChannel[1].mR[2] = 0.0;

    //2:Right Lower Leg
	mBodyChannel[2].mName = "RightLeg";
	mBodyChannel[2].mParentChannel = 1;
	mBodyChannel[2].mT[0] = -11.75;
	mBodyChannel[2].mT[1] = 56.54;
	mBodyChannel[2].mT[2] = -2.37;
	mBodyChannel[2].mR[0] = 0.0;
	mBodyChannel[2].mR[1] = 0.0;
	mBodyChannel[2].mR[2] = 0.0;

    //3:Right Foot
    mBodyChannel[3].mName = "RightFoot";
    mBodyChannel[3].mParentChannel = 2;
    mBodyChannel[3].mT[0] = -11.31;
    mBodyChannel[3].mT[1] = 9.83;
    mBodyChannel[3].mT[2] = -2.63;
    mBodyChannel[3].mR[0] = 0.0;
    mBodyChannel[3].mR[1] = 0.0;
    mBodyChannel[3].mR[2] = 0.0;

    //4:Right Toe
    mBodyChannel[4].mName = "RightToeBase";
    mBodyChannel[4].mParentChannel = 3;
    mBodyChannel[4].mT[0] = -11.31;
    mBodyChannel[4].mT[1] = 1.81;
    mBodyChannel[4].mT[2] = 9.53;
    mBodyChannel[4].mR[0] = 0.0;
    mBodyChannel[4].mR[1] = 0.0;
    mBodyChannel[4].mR[2] = 0.0;

    //5:Left Upper Leg
    mBodyChannel[5].mName = "LeftUpLeg";
    mBodyChannel[5].mParentChannel = 0;
    mBodyChannel[5].mT[0] = 10.55;
    mBodyChannel[5].mT[1] = 102.19;
    mBodyChannel[5].mT[2] = -0.86;
    mBodyChannel[5].mR[0] = 0.0;
    mBodyChannel[5].mR[1] = 0.0;
    mBodyChannel[5].mR[2] = 0.0;

    //6:Left Lower Leg
    mBodyChannel[6].mName = "LeftLeg";
    mBodyChannel[6].mParentChannel = 5;
    mBodyChannel[6].mT[0] = 11.75;
    mBodyChannel[6].mT[1] = 56.54;
    mBodyChannel[6].mT[2] = -2.37;
    mBodyChannel[6].mR[0] = 0.0;
    mBodyChannel[6].mR[1] = 0.0;
    mBodyChannel[6].mR[2] = 0.0;

    //7:Left Foot
    mBodyChannel[7].mName = "LeftFoot";
    mBodyChannel[7].mParentChannel = 6;
    mBodyChannel[7].mT[0] = 11.31;
    mBodyChannel[7].mT[1] = 9.83;
    mBodyChannel[7].mT[2] = -2.63;
    mBodyChannel[7].mR[0] = 0.0;
    mBodyChannel[7].mR[1] = 0.0;
    mBodyChannel[7].mR[2] = 0.0;

    //8:Left Toe
    mBodyChannel[8].mName = "LeftToeBase";
    mBodyChannel[8].mParentChannel = 7;
    mBodyChannel[8].mT[0] = 11.31;
    mBodyChannel[8].mT[1] = 1.81;
    mBodyChannel[8].mT[2] = 9.53;
    mBodyChannel[8].mR[0] = 0.0;
    mBodyChannel[8].mR[1] = 0.0;
    mBodyChannel[8].mR[2] = 0.0;

    //9:Spine
    mBodyChannel[9].mName = "Spine";
    mBodyChannel[9].mParentChannel = 0;
    mBodyChannel[9].mT[0] = 0.0;
    mBodyChannel[9].mT[1] = 120.60;
    mBodyChannel[9].mT[2] = 0.0;
    mBodyChannel[9].mR[0] = 0.0;
    mBodyChannel[9].mR[1] = 0.0;
    mBodyChannel[9].mR[2] = 0.0;

    //10:Spine1
    mBodyChannel[10].mName = "Spine1";
    mBodyChannel[10].mParentChannel = 9;
    mBodyChannel[10].mT[0] = 0.0;
    mBodyChannel[10].mT[1] = 131.87;
    mBodyChannel[10].mT[2] = 0.0;
    mBodyChannel[10].mR[0] = 0.0;
    mBodyChannel[10].mR[1] = 0.0;
    mBodyChannel[10].mR[2] = 0.0;

    //11:Spine2
    mBodyChannel[11].mName = "Spine2";
    mBodyChannel[11].mParentChannel = 10;
    mBodyChannel[11].mT[0] = 0.0;
    mBodyChannel[11].mT[1] = 143.90;
    mBodyChannel[11].mT[2] = 0.0;
    mBodyChannel[11].mR[0] = 0.0;
    mBodyChannel[11].mR[1] = 0.0;
    mBodyChannel[11].mR[2] = 0.0;

    //12:Spine3
    mBodyChannel[12].mName = "Spine3";
    mBodyChannel[12].mParentChannel = 11;
    mBodyChannel[12].mT[0] = 0.0;
    mBodyChannel[12].mT[1] = 155.62;
    mBodyChannel[12].mT[2] = 1.25;
    mBodyChannel[12].mR[0] = 0.0;
    mBodyChannel[12].mR[1] = 0.0;
    mBodyChannel[12].mR[2] = 0.0;

    //13:Neck
    mBodyChannel[13].mName = "Neck";
    mBodyChannel[13].mParentChannel = 12;
    mBodyChannel[13].mT[0] = 0.0;
    mBodyChannel[13].mT[1] = 167.69;
    mBodyChannel[13].mT[2] = 0.0;
    mBodyChannel[13].mR[0] = 0.0;
    mBodyChannel[13].mR[1] = 0.0;
    mBodyChannel[13].mR[2] = 0.0;

    //14:Head
    mBodyChannel[14].mName = "Head";
    mBodyChannel[14].mParentChannel = 13;
    mBodyChannel[14].mT[0] = 0.0;
    mBodyChannel[14].mT[1] = 177.70;
    mBodyChannel[14].mT[2] = 0.0;
    mBodyChannel[14].mR[0] = 0.0;
    mBodyChannel[14].mR[1] = 0.0;
    mBodyChannel[14].mR[2] = 0.0;

    //15:Right Shoulder
    mBodyChannel[15].mName = "RightShoulder";
    mBodyChannel[15].mParentChannel = 12;
    mBodyChannel[15].mT[0] = -4.90;
    mBodyChannel[15].mT[1] = 159.69;
    mBodyChannel[15].mT[2] = 0.0;
    mBodyChannel[15].mR[0] = 0.0;
    mBodyChannel[15].mR[1] = 0.0;
    mBodyChannel[15].mR[2] = 0.0;

    //16:Right Upper Arm
    mBodyChannel[16].mName = "RightArm";
    mBodyChannel[16].mParentChannel = 15;
    mBodyChannel[16].mT[0] = -20.55;
    mBodyChannel[16].mT[1] = 159.69;
    mBodyChannel[16].mT[2] = 0.0;
    mBodyChannel[16].mR[0] = 0.0;
    mBodyChannel[16].mR[1] = 0.0;
    mBodyChannel[16].mR[2] = 0.0;

    //17:Right Lower Arm
    mBodyChannel[17].mName = "RightForeArm";
    mBodyChannel[17].mParentChannel = 16;
    mBodyChannel[17].mT[0] = -46.43;
    mBodyChannel[17].mT[1] = 159.69;
    mBodyChannel[17].mT[2] = 0.0;
    mBodyChannel[17].mR[0] = 0.0;
    mBodyChannel[17].mR[1] = 0.0;
    mBodyChannel[17].mR[2] = 0.0;

    //18:Right Hand
    mBodyChannel[18].mName = "RightHand";
    mBodyChannel[18].mParentChannel = 17;
    mBodyChannel[18].mT[0] = -74.83;
    mBodyChannel[18].mT[1] = 159.69;
    mBodyChannel[18].mT[2] = 0.0;
    mBodyChannel[18].mR[0] = 0.0;
    mBodyChannel[18].mR[1] = 0.0;
    mBodyChannel[18].mR[2] = 0.0;

    //19: Left Shoulder
    mBodyChannel[19].mName = " LeftShoulder";
    mBodyChannel[19].mParentChannel = 12;
    mBodyChannel[19].mT[0] = 4.90;
    mBodyChannel[19].mT[1] = 159.69;
    mBodyChannel[19].mT[2] = 0.0;
    mBodyChannel[19].mR[0] = 0.0;
    mBodyChannel[19].mR[1] = 0.0;
    mBodyChannel[19].mR[2] = 0.0;

    //20:Left Upper Arm
    mBodyChannel[20].mName = "LeftArm";
    mBodyChannel[20].mParentChannel = 19;
    mBodyChannel[20].mT[0] = 20.55;
    mBodyChannel[20].mT[1] = 159.69;
    mBodyChannel[20].mT[2] = 0.0;
    mBodyChannel[20].mR[0] = 0.0;
    mBodyChannel[20].mR[1] = 0.0;
    mBodyChannel[20].mR[2] = 0.0;

    //21:Left Lower Arm
    mBodyChannel[21].mName = "LeftForeArm";
    mBodyChannel[21].mParentChannel = 20;
    mBodyChannel[21].mT[0] = 46.43;
    mBodyChannel[21].mT[1] = 159.69;
    mBodyChannel[21].mT[2] = 0.0;
    mBodyChannel[21].mR[0] = 0.0;
    mBodyChannel[21].mR[1] = 0.0;
    mBodyChannel[21].mR[2] = 0.0;

    //22:Left Hand
    mBodyChannel[22].mName = "LeftHand";
    mBodyChannel[22].mParentChannel = 21;
    mBodyChannel[22].mT[0] = 74.83;
    mBodyChannel[22].mT[1] = 159.69;
    mBodyChannel[22].mT[2] = 0.0;
    mBodyChannel[22].mR[0] = 0.0;
    mBodyChannel[22].mR[1] = 0.0;
    mBodyChannel[22].mR[2] = 0.0;

    //23:Left ThumbFinger
	mBodyChannel[23].mName = "LeftHandThumb1";
	mBodyChannel[23].mParentChannel = 22;
	mBodyChannel[23].mT[0] = 78.21;
	mBodyChannel[23].mT[1] = 159.94;
	mBodyChannel[23].mT[2] = 4.23;
	mBodyChannel[23].mR[0] = 0.0;
	mBodyChannel[23].mR[1] = 0.0;
	mBodyChannel[23].mR[2] = 0.0;

	//24:Left ThumbFinger1
	mBodyChannel[24].mName = "LeftHandThumb2";
	mBodyChannel[24].mParentChannel = 23;
	mBodyChannel[24].mT[0] = 81.75;
	mBodyChannel[24].mT[1] = 159.96;
	mBodyChannel[24].mT[2] = 7.76;
	mBodyChannel[24].mR[0] = 0.0;
	mBodyChannel[24].mR[1] = 0.0;
	mBodyChannel[24].mR[2] = 0.0;

	//25:Left ThumbFinger2
	mBodyChannel[25].mName = "LeftHandThumb3";
	mBodyChannel[25].mParentChannel = 24;
	mBodyChannel[25].mT[0] = 84.20;
	mBodyChannel[25].mT[1] = 159.95;
	mBodyChannel[25].mT[2] = 10.22;
	mBodyChannel[25].mR[0] = 0.0;
	mBodyChannel[25].mR[1] = 0.0;
	mBodyChannel[25].mR[2] = 0.0;

	//26:Left IndexFinger1
	mBodyChannel[26].mName = "LeftHandIndex1";
	mBodyChannel[26].mParentChannel = 22;
	mBodyChannel[26].mT[0] = 86.29;
	mBodyChannel[26].mT[1] = 160.25;
	mBodyChannel[26].mT[2] = 4.04;
	mBodyChannel[26].mR[0] = 0.0;
	mBodyChannel[26].mR[1] = 0.0;
	mBodyChannel[26].mR[2] = 0.0;

	//27:Left IndexFinger2
	mBodyChannel[27].mName = "LeftHandIndex2";
	mBodyChannel[27].mParentChannel = 26;
	mBodyChannel[27].mT[0] = 91.20;
	mBodyChannel[27].mT[1] = 160.01;
	mBodyChannel[27].mT[2] = 4.03;
	mBodyChannel[27].mR[0] = 0.0;
	mBodyChannel[27].mR[1] = 0.0;
	mBodyChannel[27].mR[2] = 0.0;

	//28:Left IndexFinger3
	mBodyChannel[28].mName = "LeftHandIndex3";
	mBodyChannel[28].mParentChannel = 27;
	mBodyChannel[28].mT[0] = 93.98;
	mBodyChannel[28].mT[1] = 159.84;
	mBodyChannel[28].mT[2] = 4.04;
	mBodyChannel[28].mR[0] = 0.0;
	mBodyChannel[28].mR[1] = 0.0;
	mBodyChannel[28].mR[2] = 0.0;

	//29:Left MiddleFinger1
	mBodyChannel[29].mName = "LeftHandMiddle1";
	mBodyChannel[29].mParentChannel = 22;
	mBodyChannel[29].mT[0] = 86.44;
	mBodyChannel[29].mT[1] = 160.27;
	mBodyChannel[29].mT[2] = 1.45;
	mBodyChannel[29].mR[0] = 0.0;
	mBodyChannel[29].mR[1] = 0.0;
	mBodyChannel[29].mR[2] = 0.0;

	//30:Left MiddleFinger2
	mBodyChannel[30].mName = "LeftHandMiddle2";
	mBodyChannel[30].mParentChannel = 29;
	mBodyChannel[30].mT[0] = 91.79;
	mBodyChannel[30].mT[1] = 159.91;
	mBodyChannel[30].mT[2] = 1.43;
	mBodyChannel[30].mR[0] = 0.0;
	mBodyChannel[30].mR[1] = 0.0;
	mBodyChannel[30].mR[2] = 0.0;

	//31:Left MiddleFinger3
	mBodyChannel[31].mName = "LeftHandMiddle3";
	mBodyChannel[31].mParentChannel = 30;
	mBodyChannel[31].mT[0] = 95.14;
	mBodyChannel[31].mT[1] = 159.65;
	mBodyChannel[31].mT[2] = 1.45;
	mBodyChannel[31].mR[0] = 0.0;
	mBodyChannel[31].mR[1] = 0.0;
	mBodyChannel[31].mR[2] = 0.0;

	//32:Left RingFinger1
	mBodyChannel[32].mName = "LeftHandRing1";
	mBodyChannel[32].mParentChannel = 22;
	mBodyChannel[32].mT[0] = 85.69;
	mBodyChannel[32].mT[1] = 160.39;
	mBodyChannel[32].mT[2] = -0.83;
	mBodyChannel[32].mR[0] = 0.0;
	mBodyChannel[32].mR[1] = 0.0;
	mBodyChannel[32].mR[2] = 0.0;

	//33:Left RingFinger2
	mBodyChannel[33].mName = "LeftHandRing2";
	mBodyChannel[33].mParentChannel = 32;
	mBodyChannel[33].mT[0] = 90.35;
	mBodyChannel[33].mT[1] = 160.03;
	mBodyChannel[33].mT[2] = -0.83;
	mBodyChannel[33].mR[0] = 0.0;
	mBodyChannel[33].mR[1] = 0.0;
	mBodyChannel[33].mR[2] = 0.0;

	//34:Left RingFinger3
	mBodyChannel[34].mName = "LeftHandRing3";
	mBodyChannel[34].mParentChannel = 33;
	mBodyChannel[34].mT[0] = 93.58;
	mBodyChannel[34].mT[1] = 159.80;
	mBodyChannel[34].mT[2] = -0.82;
	mBodyChannel[34].mR[0] = 0.0;
	mBodyChannel[34].mR[1] = 0.0;
	mBodyChannel[34].mR[2] = 0.0;

	//35:Left PinkyFinger1
	mBodyChannel[35].mName = "LeftHandPinky1";
	mBodyChannel[35].mParentChannel = 22;
	mBodyChannel[35].mT[0] = 84.74;
	mBodyChannel[35].mT[1] = 160.29;
	mBodyChannel[35].mT[2] = -3.11;
	mBodyChannel[35].mR[0] = 0.0;
	mBodyChannel[35].mR[1] = 0.0;
	mBodyChannel[35].mR[2] = 0.0;

	//36:LeftPinkyFinger2
	mBodyChannel[36].mName = "LeftHandPinky2";
	mBodyChannel[36].mParentChannel = 35;
	mBodyChannel[36].mT[0] = 88.48;
	mBodyChannel[36].mT[1] = 160.08;
	mBodyChannel[36].mT[2] = -3.10;
	mBodyChannel[36].mR[0] = 0.0;
	mBodyChannel[36].mR[1] = 0.0;
	mBodyChannel[36].mR[2] = 0.0;

	//37:LeftPinkyFinger3
	mBodyChannel[37].mName = "LeftHandPinky3";
	mBodyChannel[37].mParentChannel = 36;
	mBodyChannel[37].mT[0] = 90.83;
	mBodyChannel[37].mT[1] = 159.90;
	mBodyChannel[37].mT[2] = -3.11;
	mBodyChannel[37].mR[0] = 0.0;
	mBodyChannel[37].mR[1] = 0.0;
	mBodyChannel[37].mR[2] = 0.0;

	//38:Right ThumbFinger
	mBodyChannel[38].mName = "RightHandThumb1";
	mBodyChannel[38].mParentChannel = 18;
	mBodyChannel[38].mT[0] = -78.21;
	mBodyChannel[38].mT[1] = 159.94;
	mBodyChannel[38].mT[2] = 4.23;
	mBodyChannel[38].mR[0] = 0.0;
	mBodyChannel[38].mR[1] = 0.0;
	mBodyChannel[38].mR[2] = 0.0;

	//39:Right ThumbFinger1
	mBodyChannel[39].mName = "RightHandThumb2";
	mBodyChannel[39].mParentChannel = 38;
	mBodyChannel[39].mT[0] = -81.76;
	mBodyChannel[39].mT[1] = 159.94;
	mBodyChannel[39].mT[2] = 7.75;
	mBodyChannel[39].mR[0] = 0.0;
	mBodyChannel[39].mR[1] = 0.0;
	mBodyChannel[39].mR[2] = 0.0;

	//40:RightThumbFinger2
	mBodyChannel[40].mName = "RightHandThumb3";
	mBodyChannel[40].mParentChannel = 39;
	mBodyChannel[40].mT[0] = -84.23;
	mBodyChannel[40].mT[1] = 159.94;
	mBodyChannel[40].mT[2] = 10.19;
	mBodyChannel[40].mR[0] = 0.0;
	mBodyChannel[40].mR[1] = 0.0;
	mBodyChannel[40].mR[2] = 0.0;

	//41:Right IndexFinger1
	mBodyChannel[41].mName = "RightHandIndex1";
	mBodyChannel[41].mParentChannel = 18;
	mBodyChannel[41].mT[0] = -86.29;
	mBodyChannel[41].mT[1] = 160.25;
	mBodyChannel[41].mT[2] = 4.04;
	mBodyChannel[41].mR[0] = 0.0;
	mBodyChannel[41].mR[1] = 0.0;
	mBodyChannel[41].mR[2] = 0.0;

	//42:Right IndexFinger2
	mBodyChannel[42].mName = "RightHandIndex2";
	mBodyChannel[42].mParentChannel = 41;
	mBodyChannel[42].mT[0] = -91.20;
	mBodyChannel[42].mT[1] = 160.01;
	mBodyChannel[42].mT[2] = 4.05;
	mBodyChannel[42].mR[0] = 0.0;
	mBodyChannel[42].mR[1] = 0.0;
	mBodyChannel[42].mR[2] = 0.0;

	//43:RightIndexFinger3
	mBodyChannel[43].mName = "RightHandIndex3";
	mBodyChannel[43].mParentChannel = 42;
	mBodyChannel[43].mT[0] = -93.98;
	mBodyChannel[43].mT[1] = 159.84;
	mBodyChannel[43].mT[2] = 4.05;
	mBodyChannel[43].mR[0] = 0.0;
	mBodyChannel[43].mR[1] = 0.0;
	mBodyChannel[43].mR[2] = 0.0;

	//44:Right MiddleFinger1
	mBodyChannel[44].mName = "RightHandMiddle1";
	mBodyChannel[44].mParentChannel = 18;
	mBodyChannel[44].mT[0] = -86.44;
	mBodyChannel[44].mT[1] = 160.27;
	mBodyChannel[44].mT[2] = 1.45;
	mBodyChannel[44].mR[0] = 0.0;
	mBodyChannel[44].mR[1] = 0.0;
	mBodyChannel[44].mR[2] = 0.0;

	//45:Right MiddleFinger2
	mBodyChannel[45].mName = "RightHandMiddle2";
	mBodyChannel[45].mParentChannel = 44;
	mBodyChannel[45].mT[0] = -91.79;
	mBodyChannel[45].mT[1] = 159.91;
	mBodyChannel[45].mT[2] = 1.44;
	mBodyChannel[45].mR[0] = 0.0;
	mBodyChannel[45].mR[1] = 0.0;
	mBodyChannel[45].mR[2] = 0.0;

	//46:Right MiddleFinger3
	mBodyChannel[46].mName = "RightHandMiddle3";
	mBodyChannel[46].mParentChannel = 45;
	mBodyChannel[46].mT[0] = -95.14;
	mBodyChannel[46].mT[1] = 159.65;
	mBodyChannel[46].mT[2] = 1.45;
	mBodyChannel[46].mR[0] = 0.0;
	mBodyChannel[46].mR[1] = 0.0;
	mBodyChannel[46].mR[2] = 0.0;

	//47:Right RingFinger1
	mBodyChannel[47].mName = "RightHandRing1";
	mBodyChannel[47].mParentChannel = 18;
	mBodyChannel[47].mT[0] = -85.69;
	mBodyChannel[47].mT[1] = 160.39;
	mBodyChannel[47].mT[2] = -0.83;
	mBodyChannel[47].mR[0] = 0.0;
	mBodyChannel[47].mR[1] = 0.0;
	mBodyChannel[47].mR[2] = 0.0;

	//48:Right RingFinger2
	mBodyChannel[48].mName = "RightHandRing2";
	mBodyChannel[48].mParentChannel = 47;
	mBodyChannel[48].mT[0] = -90.34;
	mBodyChannel[48].mT[1] = 159.95;
	mBodyChannel[48].mT[2] = -0.81;
	mBodyChannel[48].mR[0] = 0.0;
	mBodyChannel[48].mR[1] = 0.0;
	mBodyChannel[48].mR[2] = 0.0;

	//49:Right RingFinger3
	mBodyChannel[49].mName = "RightHandRing3";
	mBodyChannel[49].mParentChannel = 48;
	mBodyChannel[49].mT[0] = -93.57;
	mBodyChannel[49].mT[1] = 159.67;
	mBodyChannel[49].mT[2] = -0.83;
	mBodyChannel[49].mR[0] = 0.0;
	mBodyChannel[49].mR[1] = 0.0;
	mBodyChannel[49].mR[2] = 0.0;

	//50:Right PinkyFinger1
	mBodyChannel[50].mName = "RightHandPinky1";
	mBodyChannel[50].mParentChannel = 18;
	mBodyChannel[50].mT[0] = -84.74;
	mBodyChannel[50].mT[1] = 160.29;
	mBodyChannel[50].mT[2] = -3.11;
	mBodyChannel[50].mR[0] = 0.0;
	mBodyChannel[50].mR[1] = 0.0;
	mBodyChannel[50].mR[2] = 0.0;

	//51:Right PinkyFinger2
	mBodyChannel[51].mName = "RightHandPinky2";
	mBodyChannel[51].mParentChannel = 50;
	mBodyChannel[51].mT[0] = -88.48;
	mBodyChannel[51].mT[1] = 160.08;
	mBodyChannel[51].mT[2] = -3.13;
	mBodyChannel[51].mR[0] = 0.0;
	mBodyChannel[51].mR[1] = 0.0;
	mBodyChannel[51].mR[2] = 0.0;

	//52:Right PinkyFinger3
	mBodyChannel[52].mName = "RightHandPinky3";
	mBodyChannel[52].mParentChannel = 51;
	mBodyChannel[52].mT[0] = -90.83;
	mBodyChannel[52].mT[1] = 159.90;
	mBodyChannel[52].mT[2] = -3.12;
	mBodyChannel[52].mR[0] = 0.0;
	mBodyChannel[52].mR[1] = 0.0;
	mBodyChannel[52].mR[2] = 0.0;

    for (int i = 0; i < mBodyChannelCount; i++)
    {
        memcpy(mBodyChannel[i].mDefaultT, mBodyChannel[i].mT, sizeof(double) * 3);
        memcpy(mBodyChannel[i].mDefaultR, mBodyChannel[i].mR, sizeof(double) * 3);
    }

    MocapJointsState = new FBMocapJointsState(53);
    

    return true;
}

void ORDeviceMocapGVP::GetBodySkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder)
{
    pSkeletonJointsOrder = new FBBodyNodeId[53];

    pSkeletonJointsOrder[0] = kFBHipsNodeId;

    pSkeletonJointsOrder[1] = kFBRightHipNodeId;
    pSkeletonJointsOrder[2] = kFBRightKneeNodeId;
    pSkeletonJointsOrder[3] = kFBRightAnkleNodeId;
    pSkeletonJointsOrder[4] = kFBRightFootNodeId;

    pSkeletonJointsOrder[5] = kFBLeftHipNodeId;
    pSkeletonJointsOrder[6] = kFBLeftKneeNodeId;
    pSkeletonJointsOrder[7] = kFBLeftAnkleNodeId;
    pSkeletonJointsOrder[8] = kFBLeftFootNodeId;

    pSkeletonJointsOrder[9] = kFBWaistNodeId;
    pSkeletonJointsOrder[10] = kFBSpine2NodeId;
    pSkeletonJointsOrder[11] = kFBSpine3NodeId;

    pSkeletonJointsOrder[12] = kFBChestNodeId;
    pSkeletonJointsOrder[13] = kFBNeckNodeId;
    pSkeletonJointsOrder[14] = kFBHeadNodeId;

    pSkeletonJointsOrder[15] = kFBRightCollarNodeId;
    pSkeletonJointsOrder[16] = kFBRightShoulderNodeId;
    pSkeletonJointsOrder[17] = kFBRightElbowNodeId;
    pSkeletonJointsOrder[18] = kFBRightWristNodeId;

    pSkeletonJointsOrder[19] = kFBLeftCollarNodeId;
    pSkeletonJointsOrder[20] = kFBLeftShoulderNodeId;
    pSkeletonJointsOrder[21] = kFBLeftElbowNodeId;
    pSkeletonJointsOrder[22] = kFBLeftWristNodeId;

    pSkeletonJointsOrder[23] = kFBLeftThumbANodeId;
    pSkeletonJointsOrder[24] = kFBLeftThumbBNodeId;
    pSkeletonJointsOrder[25] = kFBLeftThumbCNodeId;

    pSkeletonJointsOrder[26] = kFBLeftIndexANodeId;
    pSkeletonJointsOrder[27] = kFBLeftIndexBNodeId;
    pSkeletonJointsOrder[28] = kFBLeftIndexCNodeId;

    pSkeletonJointsOrder[29] = kFBLeftMiddleANodeId;
    pSkeletonJointsOrder[30] = kFBLeftMiddleBNodeId;
    pSkeletonJointsOrder[31] = kFBLeftMiddleCNodeId;

    pSkeletonJointsOrder[32] = kFBLeftRingANodeId;
    pSkeletonJointsOrder[33] = kFBLeftRingBNodeId;
    pSkeletonJointsOrder[34] = kFBLeftRingCNodeId;

    pSkeletonJointsOrder[35] = kFBLeftPinkyANodeId;
    pSkeletonJointsOrder[36] = kFBLeftPinkyBNodeId;
    pSkeletonJointsOrder[37] = kFBLeftPinkyCNodeId;

    pSkeletonJointsOrder[38] = kFBRightThumbANodeId;
    pSkeletonJointsOrder[39] = kFBRightThumbBNodeId;
    pSkeletonJointsOrder[40] = kFBRightThumbCNodeId;

    pSkeletonJointsOrder[41] = kFBRightIndexANodeId;
    pSkeletonJointsOrder[42] = kFBRightIndexBNodeId;
    pSkeletonJointsOrder[43] = kFBRightIndexCNodeId;

    pSkeletonJointsOrder[44] = kFBRightMiddleANodeId;
    pSkeletonJointsOrder[45] = kFBRightMiddleBNodeId;
    pSkeletonJointsOrder[46] = kFBRightMiddleCNodeId;

    pSkeletonJointsOrder[47] = kFBRightRingANodeId;
    pSkeletonJointsOrder[48] = kFBRightRingBNodeId;
    pSkeletonJointsOrder[49] = kFBRightRingCNodeId;

    pSkeletonJointsOrder[50] = kFBRightPinkyANodeId;
    pSkeletonJointsOrder[51] = kFBRightPinkyBNodeId;
    pSkeletonJointsOrder[52] = kFBRightPinkyCNodeId;
}

// void ORDeviceMocapGVP::GetRHandSkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder)
// {
// pSkeletonJointsOrder = new FBBodyNodeId[20];
// 
// pSkeletonJointsOrder[0] = kFBRightWristNodeId;
// 
// pSkeletonJointsOrder[1] = kFBRightThumbANodeId;
// pSkeletonJointsOrder[2] = kFBRightThumbBNodeId;
// pSkeletonJointsOrder[3] = kFBRightThumbCNodeId;
// 
// //pSkeletonJointsOrder[4]
// pSkeletonJointsOrder[5] = kFBRightIndexANodeId;
// pSkeletonJointsOrder[6] = kFBRightIndexBNodeId;
// pSkeletonJointsOrder[7] = kFBRightIndexCNodeId;
// 
// //pSkeletonJointsOrder[8]
// pSkeletonJointsOrder[9] = kFBRightMiddleANodeId;
// pSkeletonJointsOrder[10] = kFBRightMiddleBNodeId;
// pSkeletonJointsOrder[11] = kFBRightMiddleCNodeId;
// 
// //pSkeletonJointsOrder[12]
// pSkeletonJointsOrder[13] = kFBRightRingANodeId;
// pSkeletonJointsOrder[14] = kFBRightRingBNodeId;
// pSkeletonJointsOrder[15] = kFBRightRingCNodeId;
// 
// //pSkeletonJointsOrder[16]
// pSkeletonJointsOrder[17] = kFBRightPinkyANodeId;
// pSkeletonJointsOrder[18] = kFBRightPinkyBNodeId;
// pSkeletonJointsOrder[19] = kFBRightPinkyCNodeId;
// }
// 
// void ORDeviceMocapGVP::GetLHandSkeletonJointsOrder(FBBodyNodeId*& pSkeletonJointsOrder)
// {
// pSkeletonJointsOrder = new FBBodyNodeId[20];
// 
// pSkeletonJointsOrder[0] = kFBLeftWristNodeId;
// 
// pSkeletonJointsOrder[1] = kFBLeftThumbANodeId;
// pSkeletonJointsOrder[2] = kFBLeftThumbBNodeId;
// pSkeletonJointsOrder[3] = kFBLeftThumbCNodeId;
// 
// //pSkeletonJointsOrder[4]
// pSkeletonJointsOrder[5] = kFBLeftIndexANodeId;
// pSkeletonJointsOrder[6] = kFBLeftIndexBNodeId;
// pSkeletonJointsOrder[7] = kFBLeftIndexCNodeId;
// 
// //pSkeletonJointsOrder[8]
// pSkeletonJointsOrder[9] = kFBLeftMiddleANodeId;
// pSkeletonJointsOrder[10] = kFBLeftMiddleBNodeId;
// pSkeletonJointsOrder[11] = kFBLeftMiddleCNodeId;
// 
// //pSkeletonJointsOrder[12]
// pSkeletonJointsOrder[13] = kFBLeftRingANodeId;
// pSkeletonJointsOrder[14] = kFBLeftRingBNodeId;
// pSkeletonJointsOrder[15] = kFBLeftRingCNodeId;
// 
// //pSkeletonJointsOrder[16]
// pSkeletonJointsOrder[17] = kFBLeftPinkyANodeId;
// pSkeletonJointsOrder[18] = kFBLeftPinkyBNodeId;
// pSkeletonJointsOrder[19] = kFBLeftPinkyCNodeId;
// }

FBString ORDeviceMocapGVP::GetHardWareName()
{
    return FBString("Virtual Pictures HandMocap");
}


int ORDeviceMocapGVP::GetChannelCount()
{
    return mBodyChannelCount;
}

const char* ORDeviceMocapGVP::GetChannelName(int pChannel)
{
    return mBodyChannel[pChannel].mName;
}

int ORDeviceMocapGVP::GetChannelParent(int pChannel)
{
    return mBodyChannel[pChannel].mParentChannel;
}

FBSDKNamespace::FBVector3d ORDeviceMocapGVP::GetDefaultT(int pChannel)
{
    return mBodyChannel[pChannel].mDefaultT;
}

FBSDKNamespace::FBVector3d ORDeviceMocapGVP::GetDefaultR(int pChannel)
{
    return mBodyChannel[pChannel].mDefaultR;
}



std::vector<double> ORDeviceMocapGVP::GetDataT(int pChannel)
{
    vector<double> tempT;
    tempT.push_back(mBodyChannel[pChannel].mT[0]);
    tempT.push_back(mBodyChannel[pChannel].mT[1]);
    tempT.push_back(mBodyChannel[pChannel].mT[2]);

    return tempT;
}

std::vector<double> ORDeviceMocapGVP::GetDataR(int pChannel)
{
vector<double> tempR;
tempR.push_back(mBodyChannel[pChannel].mR[0]);
tempR.push_back(mBodyChannel[pChannel].mR[1]);
tempR.push_back(mBodyChannel[pChannel].mR[2]);

return tempR;
}

/************************************************
 *    Fetch one frame skeleton data from the HardWare.
 ************************************************/
bool ORDeviceMocapGVP::FetchMocapData(FBTime& pTime)
{
   

//     if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0))
//     {
//         //get frameData
//         
//         // smooth out the skeleton data
//         
// 
//         for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
//         {
//             NUI_SKELETON_DATA lSkeletonData = skeletonFrame.SkeletonData[i];
//             NUI_SKELETON_TRACKING_STATE lTrackingState = skeletonFrame.SkeletonData[i].eTrackingState;
// 
//             if (NUI_SKELETON_TRACKED == lTrackingState)
//             {
// 
//                 mKinectMocapJointsState->mSkeletonTrackingState = (FBSkeletonTrackingState)lTrackingState;
//                 mKinectMocapJointsState->mSkeletonPosition = PositionVector4ToFBTVector(lSkeletonData.Position);
// 
//                 NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
//                 NuiSkeletonCalculateBoneOrientations(&lSkeletonData, boneOrientations);
// 
//                 // Copy data
//                 SkeletonNodeInfo* lIter;
//                 for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//                 {
//                     lIter = &mBodyChannel[j];
//                     if (lSkeletonData.eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)
//                     {
//                         lIter->mT[0] = lSkeletonData.SkeletonPositions[j].x * 100;
//                         lIter->mT[1] = lSkeletonData.SkeletonPositions[j].y * 100;
//                         lIter->mT[2] = lSkeletonData.SkeletonPositions[j].z * 100;
// 
//                         mKinectMocapJointsState->mSkeletonJointTrackingState[j] = (FBSkeletonJointTrackingState)lSkeletonData.eSkeletonPositionTrackingState[j];
//                         mKinectMocapJointsState->mSkeletonJointsPositions[j] = PositionVector4ToFBTVector(lSkeletonData.SkeletonPositions[j]);
// 
//                         NUI_SKELETON_BONE_ORIENTATION& orientation = boneOrientations[j];
//                         Matrix4 lLocalRotationMatrix4 = orientation.absoluteRotation.rotationMatrix;
//                         FBMatrix lGlobalRotationMatrix = RotationMatrix4ToFBMatrix(lLocalRotationMatrix4);
// 
//                         mKinectMocapJointsState->mSkeletonJointsGlobalOrientations[j] = lGlobalRotationMatrix;
// 
//                         FBRVector lRVector;
//                         FBMatrixToRotation(lRVector, lGlobalRotationMatrix);
// 
//                         lIter->mR[0] = lRVector[0];
//                         lIter->mR[1] = lRVector[1];
//                         lIter->mR[2] = lRVector[2];
//                     }
//                     else
//                     {
//                         mKinectMocapJointsState->mSkeletonJointTrackingState[j] = (FBSkeletonJointTrackingState)lSkeletonData.eSkeletonPositionTrackingState[j];
//                         lIter->mT[0] = mKinectMocapJointsState->mSkeletonJointsPositions[j][0];
//                         lIter->mT[1] = mKinectMocapJointsState->mSkeletonJointsPositions[j][1];
//                         lIter->mT[2] = mKinectMocapJointsState->mSkeletonJointsPositions[j][2];
// 
//                         FBRVector lRVector;
//                         FBMatrixToRotation(lRVector, mKinectMocapJointsState->mSkeletonJointsGlobalOrientations[j]);
// 
//                         lIter->mR[0] = lRVector[0];
//                         lIter->mR[1] = lRVector[1];
//                         lIter->mR[2] = lRVector[2];
//                     }
//                 }
//             }
//             else if (NUI_SKELETON_POSITION_ONLY == lTrackingState)
//             {
//             }
//         }
//     }
//     else
//     {
//         return false;
//     }
	return true;

}

void ORDeviceMocapGVP::SetDataT(int pChannel,double x,double y,double z)
{
	mBodyChannel[pChannel].mT[0] = x;
	mBodyChannel[pChannel].mT[1] = x;
	mBodyChannel[pChannel].mT[2] = x;
}




DWORD WINAPI ReceiveData(LPVOID lpParam)
{
	//param get
	bool recvSig = false;
	ReceiveDataPram* Pram = (ReceiveDataPram*)lpParam;
	SOCKET sendSocket = Pram->sendSocket;
	char* recvBuffer = Pram->recvBufferP;
	SkeletonNodeInfo* mBodyChannel = Pram->writeBodyChannel;
	SOCKADDR_IN Addr = Pram->addr;
	bool        mDataStreamStop = Pram->isWhile;
	
	
	DataFilter*  currentFrame;
	//receive data
	while (true)
	{
		currentFrame = new DataFilter;

		int len = sizeof(SOCKADDR);

		if ((recvSig = recvfrom(sendSocket, recvBuffer, 1024, 0, (SOCKADDR*)&Addr, &len)) > 0)
		{
			cout << "receive success" << endl;
			memcpy(currentFrame->UDPOriFrame, recvBuffer, 1024);
			memset(recvBuffer, 0, 1024);
			bool sig1 = false;
			bool sig2 = false;
			bool sig3 = false;


			currentFrame->checkData();
			sig1 = currentFrame->transData();
			sig2 = currentFrame->getData();
			sig3 = currentFrame->dataOpt();

			if (sig1&&sig2&&sig3)
			{

				mBodyChannel[18].mR[0] =(mBodyChannel[18].mR[0] + currentFrame->RRotList[0][0]);
				mBodyChannel[18].mR[1] =(mBodyChannel[18].mR[1] + currentFrame->RRotList[0][1]);
				mBodyChannel[18].mR[2] =(mBodyChannel[18].mR[2] + currentFrame->RRotList[0][2]);
// 				mBodyChannel[18].mR[0] += currentFrame->RRotList[0][0];
// 				mBodyChannel[18].mR[1] += currentFrame->RRotList[0][1];
// 				mBodyChannel[18].mR[2] += currentFrame->RRotList[0][2];

				for (int i=0;i< 16;i++)
				{
					mBodyChannel[i + 22].mR[0] =mBodyChannel[i + 22].mR[0] + currentFrame->LRotList[i][0];
					mBodyChannel[i + 22].mR[1] =mBodyChannel[i + 22].mR[1] + currentFrame->LRotList[i][1];
					mBodyChannel[i + 22].mR[2] =mBodyChannel[i + 22].mR[2] + currentFrame->LRotList[i][2];
// 					mBodyChannel[i + 22].mR[0] += currentFrame->LRotList[i][0];
// 					mBodyChannel[i + 22].mR[1] += currentFrame->LRotList[i][1];
// 					mBodyChannel[i + 22].mR[2] += currentFrame->LRotList[i][2];
				}
				for (int i=0;i<15;i++)
				{
					mBodyChannel[i + 38].mR[0] =mBodyChannel[i + 38].mR[0] + currentFrame->RRotList[i + 1][0];
					mBodyChannel[i + 38].mR[1] =mBodyChannel[i + 38].mR[1] + currentFrame->RRotList[i + 1][1];
					mBodyChannel[i + 38].mR[2] =mBodyChannel[i + 38].mR[2] + currentFrame->RRotList[i + 1][2];
// 					mBodyChannel[i + 38].mR[0] += currentFrame->RRotList[i+1][0];
// 					mBodyChannel[i + 38].mR[1] += currentFrame->RRotList[i+1][1];
// 					mBodyChannel[i + 38].mR[2] += currentFrame->RRotList[i+1][2];
				}
			}
			delete currentFrame;
			currentFrame = NULL;
			if (mDataStreamStop)
			{
				break;
			}
		}
		else
		{
			recvSig = false;
			int err = WSAGetLastError();
			neterror(err);
			break;
		}
	}

	return 0;
}





/************************************************************
*    get QingMu  Charactor  Data
*************************************************************/
// bool ORDeviceMocapGVP::FetchBodyJointData()
// {
// 
// }

