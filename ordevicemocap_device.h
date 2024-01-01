#ifndef __ORDEVICE_MOCAP_DEVICE_H__
#define __ORDEVICE_MOCAP_DEVICE_H__
/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2013 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

/**	\file	ordevicemocap_device.h
*	Declaration of a simple motion capture device class.
*	Class declaration of the ORDeviceMocap class.
*/

//--- SDK include
//--- Class declaration
//#include "fileio.h"

#include <fbsdk/fbsdk.h>
#include "ordevicemocap_hardware.h"
//--- Registration defines
#define ORDEVICEMOCAP_CLASSNAME		ORDeviceMocap
#define ORDEVICEMOCAP_CLASSSTR		"ORDeviceMocap"



//! Data channel class.
class DataChannel
{
public:
	//! Constructor.
	DataChannel()
	{
		mTAnimNode		= NULL;
		mRAnimNode		= NULL;
		mModelTemplate	= NULL;
	}

	//! Destructor.
	~DataChannel()
	{
	}

public:
	FBAnimationNode*	mTAnimNode;			//!< Position animation node.
	FBAnimationNode*	mRAnimNode;			//!< Rotation animation node.
	FBModelTemplate*	mModelTemplate;		//!< Joint model template driven by the data channel.
};

enum OperationStatus
{
    eAskForCreateModelBinding,
    eAskForPressOnline,
    eAskForPressSetupRecording,
    eAskForPressCalibration,
    eAskForDoCalibration,
    eReadyForRecording,
};

//! Simple motion capture device.
class ORDeviceMocap : public FBDevice
{
	//--- Declaration
	FBDeviceDeclare( ORDeviceMocap, FBDevice );

public:
	virtual bool FBCreate() override;
	virtual void FBDestroy() override;

	bool setAP(const char* addr,int size,int p);
    //--- Standard device virtual functions evaluation
	virtual bool DeviceOperation	( kDeviceOperations pOperation									) override;		//!< Operate device.
	virtual bool AnimationNodeNotify( FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo	) override;		//!< Real-time evaluation function.
	virtual void DeviceIONotify		( kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo	) override;		//!< Hardware I/O notification.
	virtual void DeviceTransportNotify( kTransportMode pMode, FBTime pTime, FBTime pSystem ) override;				//!< Transport notification.
	

	void  BodyMocap(bool live,FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);
	void  HandMocap(bool live,FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);
	//--- Recording of frame information
/*	virtual void	RecordingDoneAnimation( FBAnimationNode* pAnimationNode) override;*/
/*	void	DeviceRecordFrame			(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo);*/

	//--- Load/Save.
	virtual bool FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	//--- Initialisation/Shutdown
    bool	Init	();		//!< Initialize device.
    bool	Start	();		//!< Start device.
    bool	Stop	();		//!< Stop device.
    bool	Reset	();		//!< Reset device.
    bool	Done	();		//!< Device removal.

	//--- Get data from hardware.
	double GetDataTX(int pChannelIndex)			{	return mHardware.GetDataT(pChannelIndex)[0];		}
	double GetDataTY(int pChannelIndex)			{	return mHardware.GetDataT(pChannelIndex)[1];		}
	double GetDataTZ(int pChannelIndex)			{	return mHardware.GetDataT(pChannelIndex)[2];		}
	double GetDataRX(int pChannelIndex)			{	return mHardware.GetDataR(pChannelIndex)[0];		}
	double GetDataRY(int pChannelIndex)			{	return mHardware.GetDataR(pChannelIndex)[1];		}
	double GetDataRZ(int pChannelIndex)			{	return mHardware.GetDataR(pChannelIndex)[2];		}

	//--- Joint set management.
 	void	DefineHierarchy();				            //!< Define model template hierarchy.
 	void	Bind();										//!< Bind models
 	void	UnBind();									//!< Unbind models
	void	UnBindTNode();
	bool    DataTrans(FBAnimationNode* pAnimationNode);
	//--- Channel list manipulation.
 	int		GetChannelCount()					{	return mHardware.GetChannelCount();				}
 	int		GetChannelParent(int pMarkerIndex)	{	return mHardware.GetChannelParent(pMarkerIndex);}
 	const char*	GetChannelName(int pMarkerIndex){	return mHardware.GetChannelName(pMarkerIndex);	}

    //--- Model Template binding and unbinding notification callback.
    virtual bool ModelTemplateBindNotify( FBModel* pModel,int pIndex, FBModelTemplate* pModelTemplate );
    virtual bool ModelTemplateUnBindNotify( int pIndex, FBModelTemplate* pModelTemplate );

    //--- Setup character and story track
//     bool    SetupRecording();
//     void    PrepareSetupRecording(FBString pCharacterPath, FBString pClipDirectory);
 	bool	SetupMocapCharacter();				//!< Test if characterization process can be start.
//     bool    TriggerMocapCalibration();
	void    setCharStatus(bool sig);
	void    setAPSetStatus(bool sig);
    //--- Operation Status
     OperationStatus    GetOperationState();
     void    SetOperationState(OperationStatus pState);

	//--- Stop displaying process to local message on model unbind
/*	void	EventUIIdle( HISender pSender, HKEvent pEvent );*/

    //--- File IO
//     FileIOStatus GetFileIOStatus();
//     void SetFileIOStatus(FileIOStatus pState);

private:
    //--- Info update
	void    UpdateInfo(FBString pInfo, FBString pStatus);

    // Process global data on template models to local
//     void    ProcessGlobalToLocal();
//     void    SetupLocalGlobal(bool pGlobal);
//     void    PostCalibration();

    // Speech
//     void    ProcessSpeech();
//     void    ResetRecording();

public:
    ORDeviceMocapGVP            mHardware;					//!< Hardware abstraction object.
    FBModelTemplate*			mRootTemplate;				//!< Root model binding.

private:
    FBCharacter*                mMocapCharacter;
    FBCharacter*                mTargetCharacter;

	DataChannel					mChannels[53];	//!< Data channels.
	FBPropertyBool				UseReferenceTransformation;             // !< Apply reference transformation on incoming global data.
	bool						mHierarchyIsDefined;                    //!< True if the hierarchy is already defined
//  addr set status
	bool                        mAPReady;
//  charactor setup status
	bool                        mCharacterized;
//  is or not Body mocap
	bool                        isBodyMocap;
	bool                        isLiving;
	bool                        isBind;
/*	FBPlayerControl				mPlayerControl;   */                      //!< To get play mode for recording.
	FBApplication				mApplication;
	bool						mHasAnimationToTranspose;
	bool						mPlotting;

    // Setup recording
    FBString                    mCharacterPath; 
    FBString                    mClipDirectory;
    bool                        mReadyToSetupRecording;
    const char*                 mMocapCharacterName;
//     FileIO*                     mFileIO;
//     bool                        mDataFromFile;            // Data from File IO or from Kinect Device

    // Operation status
    OperationStatus             mOperationState;

    // Speech
/*    SpeechCommands              mSpeechCommandType;*/

    // Sound
//     HdlFBPlugTemplate<FBAudioClip>	mCalibrationAudio;
//     FBTime                      mCalibrationAudioStartTime;
};

#endif /* __ORDEVICE_MOCAP_DEVICE_H__ */
