
#include "ordevicemocap_device.h"

//--- Registration defines
#define ORDEVICEMOCAP_CLASS		    ORDEVICEMOCAP_CLASSNAME
#define ORDEVICEMOCAP_NAME		    ORDEVICEMOCAP_CLASSSTR
#define ORDEVICEMOCAP_LABEL		    "Kinect 1.0"
#define ORDEVICEMOCAP_DESC		    "Kinect 1.0"
#define ORDEVICEMOCAP_PREFIX		"MocapDevice"
#define ORDEVICEMOCAP_ICON		    "devices_body.png"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	ORDEVICEMOCAP_CLASSNAME	);
FBRegisterDevice		(	ORDEVICEMOCAP_NAME,
                         ORDEVICEMOCAP_CLASSNAME,
                         ORDEVICEMOCAP_LABEL,
                         ORDEVICEMOCAP_DESC,
                         ORDEVICEMOCAP_ICON	);

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORDeviceMocap::FBCreate()
{
    // Add model templates
    mRootTemplate =  new FBModelTemplate(ORDEVICEMOCAP_PREFIX, "Reference", kFBModelTemplateRoot);
    ModelTemplate.Children.Add(mRootTemplate);
    mHierarchyIsDefined = false;
    mHasAnimationToTranspose = false;
    mPlotting = false;
    mMocapCharacter = NULL;
    mTargetCharacter = NULL;
	mCharacterized = false;
	mAPReady = false;
	isBodyMocap = false;
	isLiving = false;
	isBind = false;
    // Setup recording
//     mCharacterPath = ""; 
//     mClipDirectory = "";
//     mReadyToSetupRecording = false;
     mMocapCharacterName = "MocapDeviceCharacter";

    // Device sampling information
     SamplingMode = kFBHardwareTimestamp;
     FBPropertyPublish( this, UseReferenceTransformation, "UseReferenceTransformation", NULL, NULL );
     UseReferenceTransformation = true;

    // File IO
//     mFileIO = new FileIO();
//     mDataFromFile = false;

    // Operation status
    mOperationState = eAskForCreateModelBinding;
	
    // Speech
//     mSpeechCommandType = eNoCommand;
//     mCalibrationAudioStartTime = 0;

    // Event
/*    FBSystem().TheOne().OnUIIdle.Add( this,(FBCallback) &ORDeviceMocap::EventUIIdle );*/

    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORDeviceMocap::FBDestroy()
{
//     delete mFileIO;
//     mFileIO = NULL;
    delete mMocapCharacter;
    mMocapCharacter = NULL;
// 
//     if( mCalibrationAudio.Ok() )
//     {
//         mCalibrationAudio->FBDelete();
//     }

    // Propagate to parent
    ParentClass::FBDestroy();
	
/*    FBSystem().TheOne().OnUIIdle.Remove( this,(FBCallback) &ORDeviceMocap::EventUIIdle );*/
}

/************************************************
*	Device operation.
************************************************/
bool ORDeviceMocap::DeviceOperation( kDeviceOperations pOperation )
{
    // Must return the online/offline status of device.
    switch (pOperation)
    {
    case kOpInit:	return Init();
    case kOpStart:	return Start();
    case kOpAutoDetect:	break;
    case kOpStop:	return Stop();
    case kOpReset:	return Reset();
    case kOpDone:	return Done();
    }
    return ParentClass::DeviceOperation(pOperation);
	return true;
}

bool ORDeviceMocap::setAP(const char* addr,int size,int p)
{
	mAPReady = mHardware.setAddrPort(addr,size,p);
	return mAPReady;

}
/************************************************
*	Initialize the device.
************************************************/
bool ORDeviceMocap::Init()
{
	
	if(!mHardware.GetSetupInfo())
	{
		UpdateInfo("Error:", "Cannot Setup Device");
		return false;
	}

	SetOperationState(eAskForCreateModelBinding);
	mHierarchyIsDefined = false;
	Bind();
	isBind = true;
	if (isLiving)
	{
		isLiving = false;
	}
	FBProgress lProgress;

	lProgress.Caption = "Device Init";
	lProgress.Text = "Initializing device...";

	return true;


}


/************************************************
*	Removal of device.
************************************************/
bool ORDeviceMocap::Done()
{
    UnBind();
	isLiving = false;
    /*
    *	Add device removal code here.
    */

    return false;
}

/************************************************
*	Reset of the device.
************************************************/
bool ORDeviceMocap::Reset()
{
    Stop();
    return Start();
}

/************************************************
*	Device is started (online).
************************************************/
bool ORDeviceMocap::Start()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Mocap Device Connect";
	lProgress.Text = "start 11111111111";
	if(mAPReady)
	{
		lProgress.Caption	= "Mocap Device";
		lProgress.Text = "Device Init";
		if( !mHardware.Init())
		{
			UpdateInfo("Error:", "Cannot Initialize Device");
			return false;
		}

		lProgress.Caption	= "Mocap Device";
		lProgress.Text = "Create Mocap Character";
		if(mCharacterized)
		{
			//step1 Open device
			lProgress.Caption = "Mocap Device";
			lProgress.Text = "Open Device";
			if (!mHardware.Open())
			{
				UpdateInfo("Error:","Couldn't Start Device");
				return false;
			}
			//step2 start data stream
			lProgress.Caption = "Mocap Device";
			lProgress.Text = "Start Data Stream";
			if (!mHardware.StartDataStream())
			{
				UpdateInfo("Error:", "Couldn't Start DataStreaming");
				return false;
			}


			FBPlayerControl().TheOne().EvaluationPause();
			UnBindTNode();
			isLiving = true;
			FBPlayerControl().TheOne().EvaluationResume();


			
		}
 		else{
 			lProgress.Caption	= "Mocap Device";
 			lProgress.Text = "Characterize False";
 			return false;
 		}
		return true;
	}
	else{
		return false;
	}
}

/************************************************
*	Device is stopped (offline).
************************************************/
bool ORDeviceMocap::Stop()
{
	//step1 Stop data stream
	if (!mHardware.StopDataStream())
	{
		UpdateInfo("Error:", "Cannot Stop Data");
		return false;
	}

	FBPlayerControl().TheOne().EvaluationPause();
	Bind();
	isLiving = false;
	FBPlayerControl().TheOne().EvaluationResume();

	//step2 Close down device
	if (!mHardware.Close())
	{
		UpdateInfo("Error:", "Cannot Stop Device");
		return false;
	}
//     if( mDataFromFile )
//     {
//         if( mFileIO->GetStatus() == eFileIOImport_Start )
//             mFileIO->SetStatus(eFileIOImport_Pause);
//     }
//     else
//     {
//         // Step 1: Stop data
//         if(! mHardware.StopDataStream() )
//         {
//             UpdateInfo("Error:", "Cannot Stop Data");
//             return false;
//         }
// 
//         // Step 2: Close down device
//         if(! mHardware.Close() )
//         {
//             UpdateInfo("Error:", "Cannot Stop Device");
//             return false;
//         }
//     }
// 

	SetOperationState(eAskForPressOnline);
    return false;
}

/************************************************
*	Define model template hierarchy.
************************************************/
void ORDeviceMocap::DefineHierarchy()
{
    if( !mHierarchyIsDefined && GetChannelCount() > 0 )
    {
        int lParent;
        for(int i=0; i< GetChannelCount(); i++)
        {
            lParent = GetChannelParent(i);
            if(lParent == -1)
            {
                mRootTemplate->Children.Add(mChannels[i].mModelTemplate);
            }
            else
            {
                mChannels[lParent].mModelTemplate->Children.Add(mChannels[i].mModelTemplate);
            }
        }

        mHierarchyIsDefined = true;
    }
}

/************************************************
*	Process global data on template models to local.
************************************************/
// void ORDeviceMocap::ProcessGlobalToLocal()
// {
// //     SetupLocalGlobal(true);
// // 
// //     int i;
// //     FBModelList lModels;
// //     FBModel* lModel;
// // 
// //     FBGetSelectedModels(lModels);
// //     for(i = 0; i < lModels.GetCount(); i++)
// //         lModels[i]->Selected = false;
// // 
// //     lModels.Clear();
// //     for(i = 0; i < GetChannelCount(); i++)
// //     {
// //         if(mChannels[i].mModelTemplate && (lModel = mChannels[i].mModelTemplate->Model) != NULL)
// //         {
// //             lModel->Selected = true;
// //             lModels.Add(lModel);
// //         }
// //     }
// // 
// //     mPlotting = true;
// //     FBSystem().CurrentTake->PlotTakeOnSelected(SamplingPeriod);
// //     mPlotting = false;
// // 
// //     for(i = 0; i < lModels.GetCount(); i++)
// //         lModels[i]->Selected = false;	
// // 
// //     SetupLocalGlobal(false);
// // 
// //     bool ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;
// // 
// //     // Clear Translation and Rotation
// //     if(ApplyReferenceTransformation)
// //     {
// //         FBVector4d x;
// //         mRootTemplate->Model->Translation.SetData(&x);
// //         mRootTemplate->Model->Rotation.SetData(&x);
// //     }
// }

/*  setup   local to global*/

// void ORDeviceMocap::SetupLocalGlobal(bool pGlobal)
// {
// //     for( int i = 0; i < GetChannelCount(); i++ )
// //     {
// //         if( mChannels[i].mTAnimNode )
// //         {
// //             mChannels[i].mTAnimNode->SetBufferType(pGlobal);
// //         }
// // 
// //         if( mChannels[i].mRAnimNode )
// //         {
// //             mChannels[i].mRAnimNode->SetBufferType(pGlobal);
// //         }
// //     }
// }

/************************************************
*	Model Template unbinding notification callback
************************************************/
bool ORDeviceMocap::ModelTemplateUnBindNotify( int pIndex, FBModelTemplate* pModelTemplate)
{

	//UpdateInfo("status:","UnBind Notify");
	
    // Reset skeleton by reevaluating a candidate because of Local/Global animation conversion
   bool ObjectInProcess = GetObjectStatus(kFBStatusCreating) || GetObjectStatus(kFBStatusDestroying) || GetObjectStatus(kFBStatusMerging);
	if(pModelTemplate->Model && pModelTemplate->Model == mRootTemplate->Model && !ObjectInProcess)
    {

		//StatusHandle::One()->PopMessage(eSTATUS_FIVE);

        FBPlayerControl().TheOne().EvaluationPause();
        for( int i = 0; i < GetChannelCount(); i++ )
        {
            // Create new translation and rotation animation nodes if necessary
            if(mChannels[i].mTAnimNode)
            {
                mChannels[i].mModelTemplate->Model->Translation.SetAnimated(true);
                FBAnimationNode* lNode = mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode();
                FBVector3d  lVector(mChannels[i].mModelTemplate->Model->Translation);
                lNode->SetCandidate(lVector.mValue);
            }

            if(mChannels[i].mRAnimNode)
            {
                mChannels[i].mModelTemplate->Model->Rotation.SetAnimated(true);
                FBAnimationNode* lNode = mChannels[i].mModelTemplate->Model->Rotation.GetAnimationNode();
                FBVector3d  lVector(mChannels[i].mModelTemplate->Model->Rotation);
                lNode->SetCandidate(lVector.mValue);
            }
        }
        FBPlayerControl().TheOne().EvaluationResume();
    }

    // Show info after model created.
    SetOperationState(eAskForPressOnline);

    return true;
}

//simple name mapping.
static void SetupMapping(FBCharacter* pCharacter, FBModel* pParent)
{
    FBString lName = (const char*)pParent->Name; lName += "Link";
    FBProperty* lLink = pCharacter->PropertyList.Find(lName,false);
    if(lLink && lLink->IsList())
    {
        ((FBPropertyListComponent*)lLink)->Add(pParent);
    }

    //do the same for children
    for(int i = 0; i < pParent->Children.GetCount(); i++)
    {
        SetupMapping(pCharacter,pParent->Children[i]);
    }
}

/************************************************
*	Characterize
************************************************/
bool ORDeviceMocap::SetupMocapCharacter()
{

    // Character already setup
    if( mMocapCharacter )
        return true;        

    // Skeleton is not ready to setup character
    if( !mHierarchyIsDefined || GetChannelCount() <= 0 || !mRootTemplate->Model )
        return false;

    // Map the skeleton to character and characterize
    mMocapCharacter = new FBCharacter("MocapDeviceCharacter");
    SetupMapping(mMocapCharacter, mRootTemplate->Model);
    mMocapCharacter->SetCharacterizeOn(true);
    mMocapCharacter->SetExternalSolver(2); // Using MB Character Solver currently
    mMocapCharacter->Active = true;

    // Set the device info to character
    FBBodyNodeId* pSkeletonJointsOrder = NULL;
    mHardware.GetBodySkeletonJointsOrder(pSkeletonJointsOrder);
    mMocapCharacter->SetupDeviceMocap((FBDevice*)this,53, pSkeletonJointsOrder);

    delete pSkeletonJointsOrder;

    // No need the data input from channel.  Note: mChannel and mChannels will be removed late.
    //UnBind();
    
    return true;
}


bool ORDeviceMocap::ModelTemplateBindNotify( FBModel* pModel,int pIndex, FBModelTemplate* pModelTemplate)
{
	
    return true;    // Will fail if do characterization here
}

/************************************************
*	End the channel set definition.
************************************************/
void ORDeviceMocap::Bind()
{
	
    int i;
    // Exit edit mode:
    // All used channels: if already defined, don't touch, if new: create animation node and model template
    // All unused channels: delete animation nodes and associated model template
    for( i = 0; i < GetChannelCount(); i++ )
    {
        // Create new translation and rotation animation nodes if necessary
        if( !mChannels[i].mTAnimNode )
        {
            // We must use a unique name for our connector.
            FBString lName( GetChannelName(i), " T" );
            mChannels[i].mTAnimNode = AnimationNodeOutCreate( 10000+i, lName, ANIMATIONNODE_TYPE_LOCAL_TRANSLATION	);
        }

        if( !mChannels[i].mRAnimNode )
        {
            // We must use a unique name for our connector.
            FBString lName( GetChannelName(i), " R" );
            mChannels[i].mRAnimNode = AnimationNodeOutCreate( 10000+i, lName, ANIMATIONNODE_TYPE_LOCAL_ROTATION	);
        }

        // Create new model templates
        if(!mChannels[i].mModelTemplate)
        {
            mChannels[i].mModelTemplate = new FBModelTemplate(ORDEVICEMOCAP_PREFIX, GetChannelName(i), kFBModelTemplateSkeleton);
            // Bind model template to T and R animation nodes
            mChannels[i].mModelTemplate->Bindings.Add(mChannels[i].mTAnimNode);
            mChannels[i].mModelTemplate->Bindings.Add(mChannels[i].mRAnimNode);

            // Setting global values makes weird value when not live
            mChannels[i].mModelTemplate->DefaultTranslation = mHardware.GetDefaultT(i);
            mChannels[i].mModelTemplate->DefaultRotation = mHardware.GetDefaultR(i);
        }
    }

    //Define hierarchy if needed
    DefineHierarchy();
}



void ORDeviceMocap::UnBind()
{
	
	isBind = false;
    int i;
    for( i = 0; i < GetChannelCount(); i++ )
    {
        // Unbind model templates from T and R animation nodes
        if( mChannels[i].mTAnimNode )
        {
            if( mChannels[i].mModelTemplate )
            {
                mChannels[i].mModelTemplate->Bindings.Remove(mChannels[i].mTAnimNode);
            }
        }

        if( mChannels[i].mRAnimNode )
        {
            if( mChannels[i].mModelTemplate )
            {
                mChannels[i].mModelTemplate->Bindings.Remove(mChannels[i].mRAnimNode);
            }
        }

        // Remove as child of root template
        if( mRootTemplate->Children.Find(mChannels[i].mModelTemplate) >= 0 )
        {
            mRootTemplate->Children.Remove(mChannels[i].mModelTemplate);
        }

        // Destroy unused animation nodes
        if( mChannels[i].mTAnimNode )
        {
            AnimationNodeDestroy(mChannels[i].mTAnimNode);
        }

        if( mChannels[i].mRAnimNode )
        {
            AnimationNodeDestroy(mChannels[i].mRAnimNode);
        }

        mChannels[i].mTAnimNode = NULL;
        mChannels[i].mRAnimNode = NULL;

        if( mChannels[i].mModelTemplate )
        {
            mChannels[i].mModelTemplate->Children.RemoveAll();
        }
    }

    for( i = 0; i < GetChannelCount(); i++ )
    {
        // Delete model template
        delete mChannels[i].mModelTemplate;
        mChannels[i].mModelTemplate = NULL;
    }

}

void ORDeviceMocap::UnBindTNode()
{
	isBind = false;
	int i;
	for( i = 0; i < GetChannelCount(); i++ )
	{
		// Unbind model templates from T and R animation nodes
		if( mChannels[i].mTAnimNode )
		{
			if( mChannels[i].mModelTemplate )
			{
				mChannels[i].mModelTemplate->Bindings.Remove(mChannels[i].mTAnimNode);
			}
		}

// 		if( mChannels[i].mRAnimNode )
// 		{
// 			if( mChannels[i].mModelTemplate )
// 			{
// 				mChannels[i].mModelTemplate->Bindings.Remove(mChannels[i].mRAnimNode);
// 			}
// 		}

		// Remove as child of root template
// 		if( mRootTemplate->Children.Find(mChannels[i].mModelTemplate) >= 0 )
// 		{
// 			mRootTemplate->Children.Remove(mChannels[i].mModelTemplate);
// 		}

		// Destroy unused animation nodes
		if( mChannels[i].mTAnimNode )
		{
			AnimationNodeDestroy(mChannels[i].mTAnimNode);
		}

// 		if( mChannels[i].mRAnimNode )
// 		{
// 			AnimationNodeDestroy(mChannels[i].mRAnimNode);
// 		}

		mChannels[i].mTAnimNode = NULL;
//		mChannels[i].mRAnimNode = NULL;

// 		if( mChannels[i].mModelTemplate )
// 		{
// 			mChannels[i].mModelTemplate->Children.RemoveAll();
// 		}
	}

// 	for( i = 0; i < GetChannelCount(); i++ )
// 	{
// 		// Delete model template
// 		delete mChannels[i].mModelTemplate;
// 		mChannels[i].mModelTemplate = NULL;
// 	}
}

/************************************************
*	Real-time Evaluation Engine.
************************************************/
bool ORDeviceMocap::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{

    kReference lReference = pAnimationNode->Reference;

    if (lReference)
    {
		BodyMocap(isLiving, pAnimationNode,pEvaluateInfo);
		HandMocap(isLiving,pAnimationNode,pEvaluateInfo);
	
//         FBTVector	Pos;
//         FBRVector	Rot;
//         FBSVector	Scal;
//         FBMatrix	GlobalNodeTransformation, GlobalReferenceTransformation;
//         bool		ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;
// 
//         if(ApplyReferenceTransformation)
//         {
//             mRootTemplate->Model->GetMatrix(GlobalReferenceTransformation,kModelTransformation,true,pEvaluateInfo);
//         }
// 
//         for(int i=0;i<GetChannelCount();i++)
//         {
//             if(mChannels[i].mTAnimNode && mChannels[i].mRAnimNode)
//             {
// 				int parentC = mHardware.GetChannelParent(i);
//                 bool lDontWrite = false;
//                 if ((!pAnimationNode->Live || mPlotting) && mChannels[i].mModelTemplate->Model)
//                 {
//                     mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);
//                     mChannels[i].mModelTemplate->Model->Rotation.GetAnimationNode()->Evaluate(Rot.mValue,pEvaluateInfo->GetLocalTime(),false);
//                 } else if (pAnimationNode->Live)
//                 {
// 					UpdateInfo("status:","mocap animation rot");
//                     Pos[0] = mHardware.GetDataT(i)[0];
//                     Pos[1] = mHardware.GetDataT(i)[1];
//                     Pos[2] = mHardware.GetDataT(i)[2];
//                     Rot[0] = mHardware.GetDataR(i)[0];
//                     Rot[1] = mHardware.GetDataR(i)[1];
//                     Rot[2] = mHardware.GetDataR(i)[2];
// 
//                 } else
//                 {
//                     lDontWrite = true; // Nothing to do 
//                 }
// 
//                 if(ApplyReferenceTransformation)
//                 {
//                     FBTRSToMatrix(GlobalNodeTransformation,Pos,Rot,Scal);
//                     FBGetGlobalMatrix(GlobalNodeTransformation,GlobalReferenceTransformation,GlobalNodeTransformation);
//                     FBMatrixToTranslation(Pos,GlobalNodeTransformation);
//                     FBMatrixToRotation(Rot,GlobalNodeTransformation);
//                 }
// 
//                 if(!lDontWrite)
//                 {
//                     if (!pAnimationNode->Live || mPlotting)
//                     {
//                         mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo);
//                         mChannels[i].mTAnimNode->WriteData( Pos.mValue, pEvaluateInfo);
//                     }else
//                     {
//                         mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
//                         mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );
//                     }
//                 }
//             }
//         }
    }
    return ParentClass::AnimationNodeNotify( pAnimationNode , pEvaluateInfo);
	return true;
}



/************************************************
*	Transport notication.
*	Useful to compute time offset between Hardware time and System time
*  PreparePlay is called just before play
************************************************/
void ORDeviceMocap::DeviceTransportNotify( kTransportMode pMode, FBTime pTime, FBTime pSystem )
{

}

/************************************************
*	Real-Time Synchronous Device IO.
************************************************/
void ORDeviceMocap::DeviceIONotify( kDeviceIOs pAction,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	//UpdateInfo("status:","DeviceIONotify");
    switch (pAction)
    {
	case kIOPlayModeWrite:
	case kIOStopModeWrite:
        {
            // Output devices
        }
        break;

	case kIOStopModeRead:
	case kIOPlayModeRead:
        {
			//test
        }
        break;
    }
}

// void ORDeviceMocap::RecordingDoneAnimation( FBAnimationNode* pAnimationNode )
// {
//     // Parent call
// //     FBDevice::RecordingDoneAnimation( pAnimationNode );
// //     mHasAnimationToTranspose = true;
// }

/************************************************
*	Record a frame of the device (recording).
************************************************/
// void ORDeviceMocap::DeviceRecordFrame(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo)
// {
// //     if( mPlayerControl.GetTransportMode() == kFBTransportPlay )
// //     {
// //         int i;
// //         FBAnimationNode* Data;
// // 
// //         FBTVector	Pos;
// //         FBRVector	Rot;
// //         bool		ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;
// // 
// //         FBMatrix	GlobalReferenceTransformation;
// //         if(ApplyReferenceTransformation)
// //             mRootTemplate->Model->GetMatrix(GlobalReferenceTransformation,kModelTransformation,true);
// // 
// //         for (i=0; i<GetChannelCount(); i++)
// //         {
// //             // Translation information.
// //             if (mChannels[i].mTAnimNode)
// //             {
// //                 Data = mChannels[i].mTAnimNode->GetAnimationToRecord();
// //                 if (Data)
// //                 {
// //                     Pos[0] = mHardware.GetDataT(i)[0];
// //                     Pos[1] = mHardware.GetDataT(i)[1];
// //                     Pos[2] = mHardware.GetDataT(i)[2];
// // 
// //                     if(ApplyReferenceTransformation)
// //                         FBVectorMatrixMult(Pos,GlobalReferenceTransformation,Pos);
// // 
// //                     Data->KeyAdd(pTime, Pos);
// //                 }
// //             }
// // 
// //             // Rotation information.
// //             if (mChannels[i].mRAnimNode)
// //             {
// //                 Data = mChannels[i].mRAnimNode->GetAnimationToRecord();
// //                 if (Data)
// //                 {
// //                     Rot[0] = mHardware.GetDataR(i)[0];
// //                     Rot[1] = mHardware.GetDataR(i)[1];
// //                     Rot[2] = mHardware.GetDataR(i)[2];
// // 
// //                     if(ApplyReferenceTransformation)
// //                     {
// //                         FBMatrix GRM;
// //                         FBRotationToMatrix(GRM,Rot);
// //                         FBGetGlobalMatrix(GRM,GlobalReferenceTransformation,GRM);
// //                         FBMatrixToRotation(Rot,GRM);
// //                     }
// // 
// //                     Data->KeyAdd(pTime, Rot);
// //                 }
// //             }
// //         }
// //     }
// }

/************************************************
*	Store data in FBX.
************************************************/
bool ORDeviceMocap::FbxStore(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
//     if (pStoreWhat & kAttributes)
//     {
//     }
    return true;
}


/************************************************
*	Retrieve data from FBX.
************************************************/
bool ORDeviceMocap::FbxRetrieve(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
//     if( pStoreWhat & kAttributes )
//     {
//     }
    return true;
}

// void ORDeviceMocap::EventUIIdle( HISender pSender, HKEvent pEvent )
// {
// //     if(mHasAnimationToTranspose)
// //     {
// //         mHasAnimationToTranspose = false;
// // 
// //         // Put the animation back on skeleton
// //         ProcessGlobalToLocal();
// //     }
// // 
// //     if(mReadyToSetupRecording)
// //     {
// //         if( SetupRecording() )
// //         {
// //             SetOperationState(eAskForPressCalibration);
// //         }
// //         else
// //         {
// //             UpdateInfo("Error:", "Fail To Do Calibration");
// //         }
// //         mReadyToSetupRecording = false;
// //     }
// //     
// //     if( mCalibrationAudio.Ok() )
// //     {
// //         FBTime lTimeNow = FBSystem().SystemTime;
// //         if( lTimeNow - mCalibrationAudioStartTime > mCalibrationAudio->SrcDuration )
// //         {
// //             mCalibrationAudio->FBDelete();
// //             mCalibrationAudioStartTime = 0;
// //         }
// //     }
// // 
// //     if(mMocapCharacter &&  mMocapCharacter->GetMocapCalibrationState() == FBCalibrationSuccessfully)
// //     {
// //         PostCalibration();
// //     }
// // 
// //     // Process speech commands
// //     switch (mSpeechCommandType)
// //     {
// //     case eStartRecording:
// //         {
// //             ResetRecording();
// // 
// //             mPlayerControl.Record();
// //             mPlayerControl.Play();
// //             mSpeechCommandType = eNoCommand;
// //         }
// //         break;
// //     case eStopRecording:
// //         {
// //             mPlayerControl.Stop();
// //             mSpeechCommandType = eNoCommand;
// //         }
// //         break;
// //     case ePlay:
// //         {
// //             if( mTargetCharacter && mTargetCharacter->ActiveInput == true )
// //                 mTargetCharacter->ActiveInput = false;
// // 
// //             mPlayerControl.GotoStart();
// //             mPlayerControl.Play();
// //             mSpeechCommandType = eNoCommand;
// //         }
// //         break;
// //     case eReset:
// //         {
// //             mPlayerControl.Stop();
// //             mPlayerControl.GotoStart();    
// // 
// //             ResetRecording();
// //             mSpeechCommandType = eNoCommand;
// //         }
// //         break;
// //     case eNoCommand:
// //         break;
// //     default:
// //         break;
// //     }
// }
// 
void ORDeviceMocap::UpdateInfo(FBString pInfo, FBString pStatus)
{
    HardwareVersionInfo = FBString("Name: ") + mHardware.GetHardWareName();
    Information = pInfo;
    Status = pStatus;
}

// void ORDeviceMocap::PrepareSetupRecording(FBString pCharacterPath, FBString pClipDirectory)
// {
// //     mReadyToSetupRecording = true;
// //     mCharacterPath = pCharacterPath;
// //     mClipDirectory = pClipDirectory;
// }
// 
// bool ORDeviceMocap::SetupRecording()
// {    
// //     FBProgress	lProgress;
// //     lProgress.Caption = "Mocap Device";
// //     lProgress.Text = "Setup Recording";
// // 
// //     // Get Mocap Character     
// //     if( mMocapCharacter == NULL )
// //     {
// //         for(int i=0; i<FBSystem().Scene->Characters.GetCount(); i++)
// //         {
// //             FBString lExpectedName = mMocapCharacterName;
// //             if( lExpectedName == FBSystem().Scene->Characters[i]->Name) 
// //                 mMocapCharacter = FBSystem().Scene->Characters[i];
// //         }
// //         if( mMocapCharacter == NULL )
// //         {
// //             UpdateInfo("Error:","Mocap Character Is Invalid.");
// //             return false;
// //         }
// //     }
// // 
// //     // Get Target Character
// //     if( mTargetCharacter == NULL )
// //     {
// //         // Merge target character
// //         lProgress.Caption = "Mocap Device";
// //         lProgress.Text = "Import Character";
// //         FBFbxOptions* lOption = new FBFbxOptions(true);
// //         lOption->SetAll(kFBElementActionAppend, true);
// //         if( !FileExist( mCharacterPath ) )
// //         {
// //             UpdateInfo("Error:","Target File Not Exist.");
// //             return false;
// //         }
// //         mApplication.FileMerge(mCharacterPath, false, lOption);
// //         delete lOption;
// //         lOption = NULL;
// // 
// //         lProgress.Caption = "Mocap Device";
// //         lProgress.Text = "Retargent Mocap character to Target Character";
// //         int lCharacterCount = FBSystem().Scene->Characters.GetCount();
// //         mTargetCharacter = FBSystem().Scene->Characters[lCharacterCount - 1];
// //         if( mTargetCharacter == NULL || mTargetCharacter == mMocapCharacter )
// //         {
// //             UpdateInfo("Error:","Target Character Not Exist.");
// //             return false;
// //         }
// //     }
// //     if( mTargetCharacter->GetCurrentControlSet(true)==NULL)
// //         mTargetCharacter->CreateControlRig(true);
// //     if( mTargetCharacter->ActiveInput )
// //         mTargetCharacter->ActiveInput = false;
// //     if( mTargetCharacter->InputCharacter != mMocapCharacter)
// //         mTargetCharacter->InputCharacter = mMocapCharacter;
// //     if( mTargetCharacter->InputType != kFBCharacterInputCharacter )
// //         mTargetCharacter->InputType = kFBCharacterInputCharacter;
// // 
// //     // Set story track    
// //     bool lTargetCharacterExistingInStoryRoot = false;
// //     FBStory& lStory = FBStory::TheOne();
// //     FBStoryFolder*	lActionFolder = lStory.RootFolder;
// //     FBPropertyListStoryTrack& lTracks = lActionFolder->Tracks;
// //     for(int i=0; i<lTracks.GetCount(); i++)
// //     {
// //         FBStoryTrack* lTrack = lTracks[i];
// //         if( lTrack->Character == mTargetCharacter && !lTrack->RecordTrack )
// //         {
// //             lTrack->RecordTrack = true;
// //             lTargetCharacterExistingInStoryRoot = true;
// //         }
// //     }
// //     if( !lTargetCharacterExistingInStoryRoot )
// //     {
// //         lProgress.Caption = "Mocap Device";
// //         lProgress.Text = "Creating story track";
// //         FBStoryTrack* lCurrentStoryTrack = new FBStoryTrack(kFBStoryTrackCharacter);
// //         lCurrentStoryTrack->Character = mTargetCharacter;
// //         lCurrentStoryTrack->RecordClipPath = mClipDirectory;
// //         lCurrentStoryTrack->RecordTrack = true;
// //     }
// 
//     return true;
// }
// 
// void ORDeviceMocap::ResetRecording()
// {
//     // Reset the TargetCharacter
// //     if( mTargetCharacter && mMocapCharacter )
// //     {
// //         mTargetCharacter->ActiveInput = true;
// //         mTargetCharacter->InputCharacter = mMocapCharacter;
// //         mTargetCharacter->InputType = kFBCharacterInputCharacter;
// //     }
// // 
// //     // Reset the Story Track
// //     FBStory& lStory = FBStory::TheOne();
// //     FBStoryFolder*	lActionFolder = lStory.RootFolder;
// //     FBPropertyListStoryTrack& lTracks = lActionFolder->Tracks;
// //     for(int i=0; i<lTracks.GetCount(); i++)
// //     {
// //         FBStoryTrack* lTrack = lTracks[i];
// //         if( lTrack->Character == mTargetCharacter && !lTrack->RecordTrack )
// //         {
// //             lTrack->RecordTrack = true;
// //         }
// //     }
// }
// 
// bool ORDeviceMocap::TriggerMocapCalibration()
// {
// //     if( mMocapCharacter )
// //     {
// //         if( mTargetCharacter != NULL )
// //         {
// //             mTargetCharacter->ActiveInput = false;
// //         }
// //         mMocapCharacter->SetMocapCalibrationState( FBCalibrationInProcessing );
// //         SetOperationState(eAskForDoCalibration);
// //         return true;
// //     }
// //     else
// //     {
// //         UpdateInfo("Error:", "Mocap Character is invalid");
// //         return false;
// //     }
// 	return true;
// }
// 
// FileIOStatus ORDeviceMocap::GetFileIOStatus()
// {
//     return mFileIO->GetStatus();
// }
// 
// void ORDeviceMocap::SetFileIOStatus(FileIOStatus pState)
// {
// //     if( pState == eFileIOImport_Loading )
// //     {
// //         mDataFromFile = true;
// //     }
// //     mFileIO->SetStatus(pState);
// }
// 
OperationStatus ORDeviceMocap::GetOperationState()
{
    return mOperationState;
}

void ORDeviceMocap::SetOperationState(OperationStatus pState)
{
    if( mOperationState != pState )
        mOperationState = pState;

    switch (pState)
    {
    case eAskForCreateModelBinding:
        UpdateInfo("Step1:", "Create Model Binding");
        break;
    case eAskForPressOnline:
        UpdateInfo("Step2:", "Press Online");
        break;
    case eAskForPressSetupRecording:
        UpdateInfo("Step3:", "Press Setup Recording");
        break;
    case eAskForPressCalibration:
        UpdateInfo("Step4:", "Press Calibration");
        break;
    case eAskForDoCalibration:
        UpdateInfo("Step5:", "Perform Calibration");
        break;
    case eReadyForRecording:
        {
            UpdateInfo("Step6:", "Ready To Record");
        }
        break;
    default:
        break;
    }
}

void ORDeviceMocap::setCharStatus(bool sig)
{
	mCharacterized = sig;
}

void ORDeviceMocap::setAPSetStatus(bool sig)
{
	mAPReady = sig;
}

void ORDeviceMocap::BodyMocap(bool live,FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo)
{
	if (live)
	{
		UpdateInfo("status:","live BodyMocap");
		FBTVector	Pos;
		FBRVector	Rot;
		FBSVector	Scal;
		FBMatrix	GlobalNodeTransformation, GlobalReferenceTransformation;
		bool		ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;

// 		if(ApplyReferenceTransformation)
// 		{
// 			mRootTemplate->Model->GetMatrix(GlobalReferenceTransformation,kModelTransformation,true,pEvaluateInfo);
// 		}
		FBPlayerControl().TheOne().EvaluationPause();
		for (int i = 0;i<23;i++)  //23   53
		{
			if(mChannels[i].mRAnimNode)
			{
				bool lDontWrite = false;
				if ((!pAnimationNode->Live || mPlotting) && mChannels[i].mModelTemplate->Model)
				{
					//mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);
					mChannels[i].mModelTemplate->Model->Rotation.GetAnimationNode()->Evaluate(Rot.mValue,pEvaluateInfo->GetLocalTime(),false);
				}
				else if (pAnimationNode->Live)
				{
// 					Pos[0] = mHardware.GetDataT(i)[0];
// 					Pos[1] = mHardware.GetDataT(i)[1];
// 					Pos[2] = mHardware.GetDataT(i)[2];		
					Rot[0] = mHardware.GetDataR(i)[0];
					Rot[1] = mHardware.GetDataR(i)[1];
					Rot[2] = mHardware.GetDataR(i)[2];
				}
				else
				{
					lDontWrite = true;
				}

// 				if(ApplyReferenceTransformation)
// 				{
// 					FBTRSToMatrix(GlobalNodeTransformation,Pos,Rot,Scal);
// 					FBGetGlobalMatrix(GlobalNodeTransformation,GlobalReferenceTransformation,GlobalNodeTransformation);
// 					FBMatrixToTranslation(Pos,GlobalNodeTransformation);
// 					FBMatrixToRotation(Rot,GlobalNodeTransformation);
// 				}

				if (!lDontWrite)
				{
					if (!pAnimationNode->Live || mPlotting)
					{
						mChannels[i].mRAnimNode->SetCandidate(Rot);
						//mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo );


					}
					else
					{
						
						mChannels[i].mRAnimNode->SetCandidate(Rot);
						//mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo );

					}
				}

			}

		}
		FBPlayerControl().TheOne().EvaluationResume();
	}
	else
	{
		//UpdateInfo("status:","unlive BodyMocap");
		FBTVector	Pos;
		FBRVector	Rot;
		FBSVector	Scal;
		FBMatrix	GlobalNodeTransformation, GlobalReferenceTransformation;
		bool		ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;

		if(ApplyReferenceTransformation)
		{
			mRootTemplate->Model->GetMatrix(GlobalReferenceTransformation,kModelTransformation,true,pEvaluateInfo);
		}

		for (int i = 0;i<23;i++)  //23   53
		{
			if(mChannels[i].mRAnimNode)
			{
				bool lDontWrite = false;
				if ((!pAnimationNode->Live || mPlotting) && mChannels[i].mModelTemplate->Model)
				{
					mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);
					mChannels[i].mModelTemplate->Model->Rotation.GetAnimationNode()->Evaluate(Rot.mValue,pEvaluateInfo->GetLocalTime(),false);
				}
				else if (pAnimationNode->Live)
				{
					
					Pos[0] = mHardware.GetDataT(i)[0];
					Pos[1] = mHardware.GetDataT(i)[1];
					Pos[2] = mHardware.GetDataT(i)[2];		
					Rot[0] = mHardware.GetDataR(i)[0];
					Rot[1] = mHardware.GetDataR(i)[1];
					Rot[2] = mHardware.GetDataR(i)[2];
				}
				else
				{
					lDontWrite = true;
				}

				if(ApplyReferenceTransformation)
				{
					FBTRSToMatrix(GlobalNodeTransformation,Pos,Rot,Scal);
					FBGetGlobalMatrix(GlobalNodeTransformation,GlobalReferenceTransformation,GlobalNodeTransformation);
					FBMatrixToTranslation(Pos,GlobalNodeTransformation);
					FBMatrixToRotation(Rot,GlobalNodeTransformation);
				}

				if (!lDontWrite)
				{
					if (!pAnimationNode->Live || mPlotting)
					{
						mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo );
						mChannels[i].mTAnimNode->WriteData( Pos.mValue, pEvaluateInfo );

					}
					else
					{
						
						mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
						mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );

					}
				}

			}

		}
		//StatusHandle::One()->PopMessage(eSTATUS_TWO);
	}
}


void ORDeviceMocap::HandMocap(bool live,FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo)
{

	if (live)
	{
// 		if (isBind)
// 		{
// 			UnBindTNode();
// 		}
		
		FBVertex Poi;
		FBTVector Pos,pPos;
		FBRVector Rot,pRot;
		FBSVector Scl,pScl;
		FBMatrix  pMatrix; 
		FBMatrix  rootMatrix;
		UpdateInfo("status:","live HandMocap");
		FBPlayerControl().TheOne().EvaluationPause();
		for (int i=23;i<38;i++)
		{
			
			//mChannels[j].mModelTemplate->Model->GetMatrix(rootMatrix,kModelTransformation,true,pEvaluateInfo);
//			mRootTemplate->Model->GetMatrix(rootMatrix,kModelTransformation,true,pEvaluateInfo);
			// 		FBMatrixToTranslation(pPos,curMatrix);
			// 		FBMatrixToRotation(pRot,curMatrix);
// 			mChannels[j].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(pPos.mValue,pEvaluateInfo->GetLocalTime(),false);
// 			mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);
// 			
// 			pRot[0] = mHardware.GetDataR(j)[0];
// 			pRot[1] = mHardware.GetDataR(j)[1];
// 			pRot[2] = mHardware.GetDataR(j)[2];
			Rot[0] = mHardware.GetDataR(i)[0];
			Rot[1] = mHardware.GetDataR(i)[1];
			Rot[2] = mHardware.GetDataR(i)[2];

// 			FBMatrixToTranslation(pPos,pMatrix);
// 			FBMatrixToRotation(pRot,pMatrix);
// 			Poi[0] = Pos[0];
// 			Poi[1] = Pos[1];
// 			Poi[2] = Pos[2];
// 			string str1 = "thumb first old Position";
// 			vector<double> sV;
// 			if (i == 35)
// 			{
// 				sV.push_back(Poi[0]);
// 				sV.push_back(Poi[1]);
// 				sV.push_back(Poi[2]);
// 			}
// 			saveStr(str1,"G:\\test\\device_Data.txt");
// 			saveVec(sV,"G:\\test\\device_Data.txt");


//  			FBVertexMatrixMult(Poi,pMatrix,Poi);
// 		   
// 			Pos[0] = Poi[0];
// 			Pos[1] = Poi[1];
// 			Pos[2] = Poi[2];
// 			Pos[3] = 1.0;
// 			mHardware.SetDataT(i,Pos[0],Pos[1],Pos[2]);
			mChannels[i].mRAnimNode->SetCandidate(Rot);
			//mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo);


// 			mRootTemplate->Model->GetMatrix(pMatrix,kModelTransformation,true,pEvaluateInfo);
// 			Pos[0] = mHardware.GetDataT(i)[0];
// 			Pos[1] = mHardware.GetDataT(i)[1];
// 			Pos[2] = mHardware.GetDataT(i)[2];
// 			Rot[0] = mHardware.GetDataR(i)[0];
// 			Rot[1] = mHardware.GetDataR(i)[1];
// 			Rot[2] = mHardware.GetDataR(i)[2];
// 
// 			FBTRSToMatrix(curMatrix,Pos,Rot,Scl);
// 			FBGetGlobalMatrix(curMatrix,pMatrix,curMatrix);
// 			FBMatrixToTranslation(Pos,curMatrix);
// 			FBMatrixToRotation(Rot,curMatrix);
// 
// 			mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
// 			mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );

		}
		for (int i=38;i<53;i++)
		{
// 			mRootTemplate->Model->GetMatrix(pMatrix,kModelTransformation,true,pEvaluateInfo);
// 			Pos[0] = mHardware.GetDataT(i)[0];
// 			Pos[1] = mHardware.GetDataT(i)[1];
// 			Pos[2] = mHardware.GetDataT(i)[2];
// 			Rot[0] = mHardware.GetDataR(i)[0];
// 			Rot[1] = mHardware.GetDataR(i)[1];
// 			Rot[2] = mHardware.GetDataR(i)[2];
// 
// 			FBTRSToMatrix(curMatrix,Pos,Rot,Scl);
// 			FBGetGlobalMatrix(curMatrix,pMatrix,curMatrix);
// 			FBMatrixToTranslation(Pos,curMatrix);
// 			FBMatrixToRotation(Rot,curMatrix);
// 
// 			mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
// 			mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );
//			int j = mHardware.GetChannelParent(i);

			//mChannels[j].mModelTemplate->Model->GetMatrix(pMatrix,kModelTransformation,true,pEvaluateInfo);
			//mRootTemplate->Model->GetMatrix(rootMatrix,kModelTransformation,true,pEvaluateInfo);
			// 		FBMatrixToTranslation(pPos,curMatrix);
			// 		FBMatrixToRotation(pRot,curMatrix);
//			mChannels[j].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(pPos.mValue,pEvaluateInfo->GetLocalTime(),false);
//			mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);

// 			pRot[0] = mHardware.GetDataR(j)[0];
// 			pRot[1] = mHardware.GetDataR(j)[1];
// 			pRot[2] = mHardware.GetDataR(j)[2];
			Rot[0] = mHardware.GetDataR(i)[0];
			Rot[1] = mHardware.GetDataR(i)[1];
			Rot[2] = mHardware.GetDataR(i)[2];

//		    FBTRSToMatrix(pMatrix,pPos,pRot,Scl);
// 			FBMatrixToTranslation(pPos,pMatrix);
// 			FBMatrixToRotation(pRot,pMatrix);
// 			Poi[0] = Pos[0];
// 			Poi[1] = Pos[1];
// 			Poi[2] = Pos[2];
// 			FBVertexMatrixMult(Poi,pMatrix,Poi);
// 			Pos[0] = Poi[0];
// 			Pos[1] = Poi[1];
// 			Pos[2] = Poi[2];
// 			Pos[3] = 1.0;
			
			mChannels[i].mRAnimNode->SetCandidate(Rot);
			//mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo);
			
	
		}
		FBPlayerControl().TheOne().EvaluationResume();
		//StatusHandle::One()->PopMessage(eSTATUS_THERE);
	}
	else
	{



// 		if (!isBind)
// 		{
// 			Bind();
// 		}
		//UpdateInfo("status:","unlive HandMocap");
		FBTVector Pos,pPos;
		FBRVector Rot,pRot;
		FBSVector Scl,pScl;
		FBMatrix  pMatrix; 
		FBMatrix  curMatrix;
		for (int i=23;i<53;i++)
		{
			if ( mChannels[i].mModelTemplate->Model)
			{
 				Pos[0] = mHardware.GetDataT(i)[0];
 				Pos[1] = mHardware.GetDataT(i)[1];
 				Pos[2] = mHardware.GetDataT(i)[2];
				Rot[0] = mHardware.GetDataR(i)[0];
				Rot[1] = mHardware.GetDataR(i)[1];
				Rot[2] = mHardware.GetDataR(i)[2];

				mRootTemplate->Model->GetMatrix(pMatrix,kModelTransformation,true,pEvaluateInfo);
 
 				FBTRSToMatrix(curMatrix,Pos,Rot,Scl);
 				FBGetGlobalMatrix(curMatrix,pMatrix,curMatrix);
 				FBMatrixToTranslation(Pos,curMatrix);
 				FBMatrixToRotation(Rot,curMatrix);

				
				mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
				mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );
			}

		}

		//StatusHandle::One()->PopMessage(eSTATUS_FOUR);
	}

	
}



/*bool ORDeviceMocap::DataTrans(FBAnimationNode* pAnimationNode)
{
	kReference lReference = pAnimationNode->Reference;

// 	if( mFileIO->GetStatus() == eFileIOImport_Start && 
// 		mFileIO->FetchMocapData(mHardware.mKinectMocapJointsState) )
// 	{
// 		mMocapCharacter->PassDeviceMocapData(mHardware.mKinectMocapJointsState); 

	if (lReference)
	{
		FBTVector	Pos;
		FBRVector	Rot;
		FBSVector	Scal;
		FBMatrix	GlobalNodeTransformation, GlobalReferenceTransformation;
		bool		ApplyReferenceTransformation = UseReferenceTransformation && mRootTemplate->Model;

		if(ApplyReferenceTransformation)
		{
			mRootTemplate->Model->GetMatrix(GlobalReferenceTransformation,kModelTransformation,true,pEvaluateInfo);
		}

		for(int i=0;i<GetChannelCount();i++)
		{
			if(mChannels[i].mTAnimNode && mChannels[i].mRAnimNode)
			{
				bool lDontWrite = false;
				if ((!pAnimationNode->Live || mPlotting) && mChannels[i].mModelTemplate->Model)
				{
					mChannels[i].mModelTemplate->Model->Translation.GetAnimationNode()->Evaluate(Pos.mValue,pEvaluateInfo->GetLocalTime(),false);
					mChannels[i].mModelTemplate->Model->Rotation.GetAnimationNode()->Evaluate(Rot.mValue,pEvaluateInfo->GetLocalTime(),false);
				} else if (pAnimationNode->Live)
				{
					Pos[0] = mHardware.GetDataT(i)[0];
					Pos[1] = mHardware.GetDataT(i)[1];
					Pos[2] = mHardware.GetDataT(i)[2];
					Rot[0] = mHardware.GetDataR(i)[0];
					Rot[1] = mHardware.GetDataR(i)[1];
					Rot[2] = mHardware.GetDataR(i)[2];
				} else
				{
					lDontWrite = true; // Nothing to do 
				}

				if(ApplyReferenceTransformation)
				{
					FBTRSToMatrix(GlobalNodeTransformation,Pos,Rot,Scal);
					FBGetGlobalMatrix(GlobalNodeTransformation,GlobalReferenceTransformation,GlobalNodeTransformation);
					FBMatrixToTranslation(Pos,GlobalNodeTransformation);
					FBMatrixToRotation(Rot,GlobalNodeTransformation);
				}

				if(!lDontWrite)
				{
					if (!pAnimationNode->Live || mPlotting)
					{
						mChannels[i].mRAnimNode->WriteData( Rot.mValue, pEvaluateInfo);
						mChannels[i].mTAnimNode->WriteData( Pos.mValue, pEvaluateInfo);
					}else
					{
						mChannels[i].mRAnimNode->WriteGlobalData( Rot.mValue, pEvaluateInfo );
						mChannels[i].mTAnimNode->WriteGlobalData( Pos.mValue, pEvaluateInfo );
					}
				}
			}
		}
	}
	return ParentClass::AnimationNodeNotify( pAnimationNode , pEvaluateInfo);
	return true;
}*/



// 
// void ORDeviceMocap::PostCalibration()
// {   
//     // Make a sound
// //     FBSystem lSystem;
// //     FBString lAudioFile( lSystem.ApplicationPath );
// //     lAudioFile += "\\..\\..\\OpenRealitySDK\\Scenes\\calibration.wav";
// //     mCalibrationAudio = new FBAudioClip( (char*)lAudioFile );
// //     if(!mCalibrationAudio.Ok())
// //         FBMessageBox("Mocap Error:", "Cannot open the calibration.wav file!  \nBut you can still use this plugin without it", "OK");
// //     if( mCalibrationAudio.Ok() )
// //     {
// //         mCalibrationAudioStartTime = lSystem.SystemTime;
// //         mCalibrationAudio->Play();
// //     }
// // 
// //     double lAverageSensorFloorOffset = mHardware.GetAverageSensorFloorOffset();
// //     if (lAverageSensorFloorOffset != 0.0)
// //     {
// //         mMocapCharacter->SetSensorFloorOffset(lAverageSensorFloorOffset);
// //         mHardware.SetSensorFloorOffsetSet();
// //     }
// // 
// //     if(mTargetCharacter)
// //     {
// //         mTargetCharacter->ActiveInput = true;
// //     }
// // 
// //     mMocapCharacter->SetMocapCalibrationState(FBCalibrationNoRequest);
// // 
// //     SetOperationState(eReadyForRecording);
// }
// 
// void ORDeviceMocap::ProcessSpeech()
// {
//     // Note: Here we couldn't execute the command to do recording directly, 
//     // else assert pop up say "Plug add in a thread other than the main thread".
// //     SpeechCommands lCmd = mHardware.ProcessSpeech();
// //     switch (lCmd)
// //     {
// //     case eStartRecording:
// //         if( !mPlayerControl.IsRecording && !mPlayerControl.IsPlaying && !mPlayerControl.IsPlotting )
// //         {
// //             mSpeechCommandType = eStartRecording;
// //         }
// //         break;
// //     case eStopRecording:
// //         {
// //             mSpeechCommandType = eStopRecording;
// //         }
// //         break;
// //     case ePlay:
// //         if(!mPlayerControl.IsRecording && !mPlayerControl.IsPlaying && !mPlayerControl.IsPlotting)
// //         {
// //             mSpeechCommandType = ePlay;
// //         }
// //         break;
// //     case eReset:
// //         {
// //             mSpeechCommandType = eReset;
// //         }
// //         break;
// //     case eNoCommand:
// //         break;
// //     default:
// //         break;
// //     }
// }

