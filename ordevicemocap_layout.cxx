

//--- Class declarations
#include <direct.h>
#include <fstream>
#include "ordevicemocap_layout.h"

#define ORDEVICEMOCAP_LAYOUT    ORDeviceMocapLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(    ORDEVICEMOCAP_LAYOUT        );
FBRegisterDeviceLayout        (    ORDEVICEMOCAP_LAYOUT,
							   ORDEVICEMOCAP_CLASSSTR,
							   FB_DEFAULT_SDK_ICON            );    // Icon filename (default=Open Reality icon)

/************************************************
*    FiLMBOX constructor.
************************************************/
bool ORDeviceMocapLayout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((ORDeviceMocap *)(FBDevice *)Device);

	// Create/configure UI
	UICreate       ();
	UIConfigure    ();
	UIReset        ();
	// Add device & system callbacks
	mDevice->OnStatusChange.Add    ( this,(FBCallback)&ORDeviceMocapLayout::EventDeviceStatusChange  );
	FBSystem().OnUIIdle.Add        ( this,(FBCallback)&ORDeviceMocapLayout::EventUIIdle              );

	return true;
}


/************************************************
*    FiLMBOX destructor.
************************************************/
void ORDeviceMocapLayout::FBDestroy()
{
	// Remove device & system callbacks
	FBSystem().OnUIIdle.Remove          ( this,(FBCallback)&ORDeviceMocapLayout::EventUIIdle              );
	mDevice->OnStatusChange.Remove      ( this,(FBCallback)&ORDeviceMocapLayout::EventDeviceStatusChange  );

	// Remove the buttons callbacks
	mTabPanel.OnChange.Remove           ( this, (FBCallback)&ORDeviceMocapLayout::EventTabPanelChange );
// 	mButtonTargetCharacter.OnClick.Remove( this, (FBCallback)&ORDeviceMocapLayout::EventTargetCharacterPath );
// 	mButtonClip.OnClick.Remove          ( this, (FBCallback)&ORDeviceMocapLayout::EventClipPath );
// 	mButtonRecording.OnClick.Remove     ( this, (FBCallback)&ORDeviceMocapLayout::EventSetupRecording );
// 	mButtonCalibration.OnClick.Remove   ( this, (FBCallback)&ORDeviceMocapLayout::EventCalibration ); 
// 	mButtonExport.OnClick.Remove        ( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOExport ); 
// 	mButtonImport.OnClick.Remove        ( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOImport ); 
	//mButtonStop.OnClick.Remove        ( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOStop );     
}


/************************************************
*    Create the UI.
************************************************/
void ORDeviceMocapLayout::UICreate()
{
	int lS, lH;        // space, height
	lS = 4;
	lH = 25;

	// Create regions
	AddRegion    ( "TabPanel",    "TabPanel",        0,        kFBAttachLeft,        "",            1.00,
		0,        kFBAttachTop,        "",            1.00,
		0,        kFBAttachRight,        "",            1.00,
		lH,        kFBAttachNone,        NULL,        1.00 );
	AddRegion    ( "MainLayout",    "MainLayout",    lS,        kFBAttachLeft,        "TabPanel",    1.00,
		lS,        kFBAttachBottom,    "TabPanel",    1.00,
		-lS,    kFBAttachRight,        "TabPanel",    1.00,
		-lS,    kFBAttachBottom,    "",            1.00 );

	// Assign regions
	SetControl    ( "TabPanel",    mTabPanel        );
	SetControl    ( "MainLayout",    mLayoutSetup    );

	// Create sub layouts
	UICreateLayoutJoints();   
	UICreateLayoutSetup();
}


/************************************************
*    Create the joints layout.
************************************************/
void ORDeviceMocapLayout::UICreateLayoutJoints()
{
	// Add regions
	mLayoutJoints.AddRegion( "SpreadJoints", "SpreadJoints",
		0,        kFBAttachLeft,        "",        1.00,
		0,        kFBAttachTop,        "",        1.00,
		0,        kFBAttachRight,        "",        1.00,
		0,        kFBAttachBottom,    "",        1.00 );

	// Assign regions
	mLayoutJoints.SetControl( "SpreadJoints", mSpreadJoints );
}


/************************************************
*    Create the setup layout.
************************************************/
void ORDeviceMocapLayout::UICreateLayoutSetup()
{
	int lS = 4;
	int lW = 200;
	int lH = 18;
	//mLabelAP***************************************************
	mLayoutSetup.AddRegion(  "LableAP", "LableAP",
		lS,         kFBAttachLeft,          "",             1.00,
		lS,         kFBAttachTop,           "",             1.00,
		lW,         kFBAttachNone,          NULL,           1.00,
		lH,         kFBAttachNone,          NULL,           1.00 );
	mLayoutSetup.SetControl("LabelAP",mLabelAP);
	//mEditAP
	mLayoutSetup.AddRegion(     "EditAP", "EditAP",
		lS,         kFBAttachLeft,          "",             1.00,
		lS,         kFBAttachBottom,        "LableAP",      1.00,
		lW * 2,     kFBAttachNone,          NULL,           1.00,
		lH,         kFBAttachNone,          NULL,           1.00 );
	mLayoutSetup.SetControl(    "EditAP", mEditAP );
	//mButtonAP
	mLayoutSetup.AddRegion(     "ButtonAP", "ButtonAP",
		lS,   kFBAttachRight,     "EditAP",        1.00,
		0,     kFBAttachTop,      "EditAP",        1.00,
		50,    kFBAttachNone,        NULL,      1.00,
		lH,    kFBAttachNone,        NULL,      1.00 );
	mLayoutSetup.SetControl( "ButtonAP", mButtonAP );

	// mLableTargetCharacter******************************************************
	mLayoutSetup.AddRegion(     "LableTargetCharacter", "LableTargetCharacter",
		lS,         kFBAttachLeft,          "",             1.00,
		lS,         kFBAttachBottom,        "EditAP",             1.00,
		lW,         kFBAttachNone,          NULL,           1.00,
		lH,         kFBAttachNone,          NULL,           1.00 );
	mLayoutSetup.SetControl(    "LableTargetCharacter", mLableTargetCharacter );

	// mEditTargetCharacter
	mLayoutSetup.AddRegion(     "EditTargetCharacter", "EditTargetCharacter",
		lS,         kFBAttachLeft,          "",             1.00,
		lS,         kFBAttachBottom,        "LableTargetCharacter",   1.00,
		lW * 2,     kFBAttachNone,          NULL,           1.00,
		lH,         kFBAttachNone,          NULL,           1.00 );
	mLayoutSetup.SetControl(    "EditTargetCharacter", mEditTargetCharacter );

	// mButtonTargetCharacter
	mLayoutSetup.AddRegion(     "ButtonTargetCharacter", "ButtonTargetCharacter",
		lS,   kFBAttachRight,     "EditTargetCharacter",        1.00,
		0,     kFBAttachTop,      "EditTargetCharacter",        1.00,
		20,    kFBAttachNone,        NULL,      1.00,
		lH,    kFBAttachNone,        NULL,      1.00 );
	mLayoutSetup.SetControl( "ButtonTargetCharacter", mButtonTargetCharacter );

	// mLableClip****************************************************************
	mLayoutSetup.AddRegion(     "LableClip", "LableClip",
		lS,     kFBAttachLeft,        "",        1.00,
		lS,     kFBAttachBottom,      "EditTargetCharacter",        1.00,
		lW,     kFBAttachNone,        NULL,      1.00,
		lH,     kFBAttachNone,        NULL,      1.00 );
	mLayoutSetup.SetControl( "LableClip", mLableClip );

	// mEditClip
	mLayoutSetup.AddRegion(     "EditClip", "EditClip",
		lS,     kFBAttachLeft,     "",        1.00,
		lS,     kFBAttachBottom,      "LableClip",        1.00,
		lW * 2,    kFBAttachNone,        NULL,      1.00,
		lH,    kFBAttachNone,        NULL,      1.00 );
	mLayoutSetup.SetControl( "EditClip", mEditClip );

	// mButtonClip
	mLayoutSetup.AddRegion(     "ButtonClip", "ButtonClip",
		lS,     kFBAttachRight,     "EditClip",        1.00,
		0,      kFBAttachTop,        "EditClip",        1.00,
		20,     kFBAttachNone,        NULL,      1.00,
		lH,     kFBAttachNone,        NULL,      1.00 );
	mLayoutSetup.SetControl( "ButtonClip", mButtonClip );

	// mLabelRecording*************************************************
// 	mLayoutSetup.AddRegion (    "LabelRecording",  "LabelRecording",
// 		lS,         kFBAttachLeft,          "",             1.00,
// 		lS,         kFBAttachBottom,        "EditTargetCharacter",             1.00,
// 		lW*2,       kFBAttachNone,          NULL,           1.00,
// 		lH,         kFBAttachNone,          NULL,           1.00 );
// 	mLayoutSetup.SetControl(    "LabelRecording", mLabelRecording );
// 
// 
// 	// mButtonRecording
// 	mLayoutSetup.AddRegion ( "ButtonRecording",    "ButtonRecording",
// 		lS,        kFBAttachLeft,       "",         1.00,
// 		lS,         kFBAttachBottom,          "LabelRecording",    1.00,
// 		lW,        kFBAttachNone,       NULL,       1.00,
// 		lH,        kFBAttachNone,       NULL,       1.00 );
// 	mLayoutSetup.SetControl( "ButtonRecording",    mButtonRecording );

	// mLabelCharactorize************************************************
	mLayoutSetup.AddRegion (    "LabelCharactorize",  "LabelCharactorize",
		lS,         kFBAttachLeft,          "",             1.00,
		lS,         kFBAttachBottom,        "EditClip",             1.00,
		lW,         kFBAttachNone,          NULL,           1.00,
		lH,         kFBAttachNone,          NULL,           1.00 );
	mLayoutSetup.SetControl(    "LabelCharactorize", mLabelCharactorize );

	// mButtonCharactorize
	mLayoutSetup.AddRegion ( "ButtonCharactorize",    "ButtonCharactorize",
		lS,        kFBAttachLeft,       "",         1.00,
		lS,         kFBAttachBottom,          "LabelCharactorize",    1.00,
		lW,        kFBAttachNone,       NULL,       1.00,
		lH,        kFBAttachNone,       NULL,       1.00 );
	mLayoutSetup.SetControl( "ButtonCharactorize",    mButtonCharactorize );

/*	if( IsMocapDataFileExisting() )*/
			{
				// mButtonExport
// 				mLayoutSetup.AddRegion ( "ButtonExport",    "ButtonExport",
// 					300,        kFBAttachRight,      "LableTargetCharacter",         1.00,
// 					0,         kFBAttachTop,        "LableTargetCharacter",    1.00,
// 					lW,        kFBAttachNone,       NULL,       1.00,
// 					lH,        kFBAttachNone,       NULL,       1.00 );
// 				mLayoutSetup.SetControl( "ButtonExport",    mButtonExport );
		
				// mButtonImport
// 				mLayoutSetup.AddRegion ( "ButtonImport",    "ButtonImport",
// 					0,         kFBAttachLeft,      "ButtonExport",         1.00,
// 					lS,        kFBAttachBottom,        "ButtonExport",    1.00,
// 					lW,        kFBAttachNone,       NULL,       1.00,
// 					lH,        kFBAttachNone,       NULL,       1.00 );
// 				mLayoutSetup.SetControl( "ButtonImport",    mButtonImport );
		
				//// mButtonStop
				//mLayoutSetup.AddRegion ( "ButtonStop",    "ButtonStop",
				//    0,         kFBAttachLeft,      "ButtonImport",         1.00,
				//    lS,        kFBAttachBottom,    "ButtonImport",    1.00,
				//    lW,        kFBAttachNone,       NULL,       1.00,
				//    lH,        kFBAttachNone,       NULL,       1.00 );
				//mLayoutSetup.SetControl( "ButtonStop",    mButtonStop );
	}
}


/************************************************
*    Configure the UI elements (main layout).
************************************************/
void ORDeviceMocapLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Setup~Joints");
	mTabPanel.OnChange.Add( this, (FBCallback)&ORDeviceMocapLayout::EventTabPanelChange );

	UIConfigureLayoutJoints();
	UIConfigureLayoutSetup();
}


/************************************************
*    Configure the UI elements (joint layout).
************************************************/
void ORDeviceMocapLayout::UIConfigureLayoutJoints()
{

}

/************************************************
*    Configure the UI elements (setup layout).
************************************************/
void ORDeviceMocapLayout::UIConfigureLayoutSetup()
{
	mLabelAP.Caption = "Input addr and port";
	mEditAP.Text ="192.168.1.239:7000";
	mButtonAP.Caption = "Yes";
	mButtonAP.OnClick.Add(this,(FBCallback)&ORDeviceMocapLayout::EventSetAP);


	mLableTargetCharacter.Caption = "Choose Body Mocap Character";
	mButtonTargetCharacter.Caption = "...";
	mButtonTargetCharacter.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventTargetCharactorChoose );

	
	mEditTargetCharacter.Text ="dick";
	mEditClip.Text = FBString(FBSystem().UserConfigPath) + "//..//recording";



// 	if( !FileExist( mEditClip.Text ) )
// 	{
// 		_mkdir(mEditClip.Text.AsString());
// 	}
	mLableClip.Caption = "Recording Path";
	mButtonClip.Caption = "...";
	mButtonClip.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventClipPath );


 	mLabelCharactorize.Caption = "";
 	mButtonCharactorize.Caption = "Charactorize";
 	mButtonCharactorize.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventCharactorSetup ); 

// 	mLabelRecording.Caption = "";
// 	mButtonRecording.Caption = "Setup Recording";
// 	mButtonRecording.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventSetupRecording );



// 	if( mDevice->GetFileIOStatus() == eFileIOExport_WaitForCalibration )
// 		mButtonExport.Caption = "Stop Exporting";
// 	else
// 		mButtonExport.Caption = "Start Exporting";    
// 	mButtonExport.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOExport ); 
// 	mButtonImport.Caption = "Import";
// 	mButtonImport.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOImport ); 
	//mButtonStop.Caption = "Stop";
	//mButtonStop.OnClick.Add( this, (FBCallback)&ORDeviceMocapLayout::EventFileIOStop );

//	EnableDisableButtons();
}

/************************************************
*    Refresh the UI.
************************************************/
void ORDeviceMocapLayout::UIRefresh()
{
	UIRefreshSpreadSheet();
}


/************************************************
*    Refresh the spreadsheet content.
************************************************/
void ORDeviceMocapLayout::UIRefreshSpreadSheet()
{

// 	string tempS = "dick";
// 	string jointNam = "LeftHand";
// 	saveStr(tempS,"G:\\test\\device_Data.txt");
	for(int i=0; i<53;i++)
	{
		mSpreadJoints.SetCell( i, 0, mDevice->GetDataTX(i) );
		mSpreadJoints.SetCell( i, 1, mDevice->GetDataTY(i) );
		mSpreadJoints.SetCell( i, 2, mDevice->GetDataTZ(i) );
		mSpreadJoints.SetCell( i, 3, mDevice->GetDataRX(i) );
		mSpreadJoints.SetCell( i, 4, mDevice->GetDataRY(i) );
		mSpreadJoints.SetCell( i, 5, mDevice->GetDataRZ(i) );


// 		if (i==22)
// 		{
// 			saveStr(jointNam,"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataTX(i)),"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataTY(i)),"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataTZ(i)),"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataRX(i)),"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataRY(i)),"G:\\test\\device_Data.txt");
// 			saveStr(to_string(mDevice->GetDataRZ(i)),"G:\\test\\device_Data.txt");
// 		}

	}
}


/************************************************
*    Reset the UI values from device.
************************************************/
void ORDeviceMocapLayout::UIReset()
{
	UIResetSpreadSheet();
}


/************************************************
*    Re-build the spreadsheet.
************************************************/
void ORDeviceMocapLayout::UIResetSpreadSheet()
{
	int i;

	mSpreadJoints.Clear();

	// Spreadsheet
	int lColumnIndex = -1;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 200;

	// column 0: Translation X
	mSpreadJoints.ColumnAdd ("PosX");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 1: Translation Y
	mSpreadJoints.ColumnAdd ("PosY");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 2: Translation Z
	mSpreadJoints.ColumnAdd ("PosZ");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 3: Rotation X
	mSpreadJoints.ColumnAdd ("RotX");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 4: Rotation Y
	mSpreadJoints.ColumnAdd ("RotY");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 5: Rotation Z
	mSpreadJoints.ColumnAdd ("RotZ");
	lColumnIndex++;
	mSpreadJoints.GetColumn(lColumnIndex).Width = 60;
	mSpreadJoints.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	for (i=0;i<53;i++)
	{
		mSpreadJoints.RowAdd( mDevice->GetChannelName(i), i );
		mSpreadJoints.GetCell(i,lColumnIndex).ReadOnly = true;
	}
}



/************************************************
*    Events
************************************************/

void ORDeviceMocapLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
	case 0:    SetControl("MainLayout", mLayoutSetup );    break;
	case 1:    SetControl("MainLayout", mLayoutJoints );    break;
	}
}

void ORDeviceMocapLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


void ORDeviceMocapLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		UIRefresh();
		//EnableDisableButtons();
	}
}

void ORDeviceMocapLayout::EventTargetCharactorChoose( HISender pSender, HKEvent pEvent )
{
	//FBSystem().Scene->Characters()
// 	FBFilePopup        lPopup;
// 	lPopup.Caption = "FBX file to load";
// 	lPopup.Style = kFBFilePopupOpen;
// 	lPopup.Filter = "*.fbx";
// 	if( lPopup.Execute() )
// 	{
// 		mEditTargetCharacter.Text = lPopup.FullFilename;
// 	}
	return;
}

void ORDeviceMocapLayout::EventCharactorSetup(HISender pSender,HKEvent pEvent)
{
	if (mDevice->SetupMocapCharacter())
	{
		mDevice->setCharStatus(true);
	}
	else
	{
		mDevice->setCharStatus(false);
	}
}

void ORDeviceMocapLayout::EventClipPath( HISender pSender, HKEvent pEvent )
{
	FBFolderPopup        lPopup;
	lPopup.Caption    = "Clip directory to save";
	if( lPopup.Execute() )
	{
		mEditClip.Text = lPopup.Path;
	}
}

// void ORDeviceMocapLayout::EventSetupRecording( HISender pSender, HKEvent pEvent )
// {
// 	if( mDevice->GetOperationState() < eAskForPressSetupRecording )
// 	{
// 		EnableDisableButtons();
// 		return;
// 	}
// 
// 	// Note: Must move the file merging step into UI Idle time as file merging causes layout destroy,
// 	// which causes the UI assert when it still invoking this destroyed button event callback.
// 	FBString lCharacterPath = mEditTargetCharacter.Text;
// 	FBString lClipDirectory = mEditClip.Text;
// 	mDevice->PrepareSetupRecording(lCharacterPath, lClipDirectory);
// }


// void ORDeviceMocapLayout::EventCalibration( HISender pSender, HKEvent pEvent )
// {
// 	if( mDevice->GetOperationState() < eAskForPressCalibration )
// 	{
// 		EnableDisableButtons();
// 		return;
// 	}
// 
// 	if( mDevice->TriggerMocapCalibration() )
// 	{
// 		// Do calibration 
// 		static bool gMuteCalibrationPopDialog = false;
// 		if( !gMuteCalibrationPopDialog )
// 		{
// 			FBMessageBoxWithCheck("Mocap Device Info:",
// 				"Stand in front of the camera for calibration",
// 				"OK", NULL, NULL,
// 				"Don't show this dialog", gMuteCalibrationPopDialog
// 				);
// 		}
// 	}
// 	return;
// }

// void ORDeviceMocapLayout::EventFileIOExport( HISender pSender, HKEvent pEvent )
// {
// 	if( mDevice->GetFileIOStatus() == eFileIOImport_Loading ||
// 		mDevice->GetFileIOStatus() == eFileIOImport_WaitForCalibration ||
// 		mDevice->GetFileIOStatus() == eFileIOImport_Pause ||
// 		mDevice->GetFileIOStatus() == eFileIOImport_Start )
// 	{
// 		static bool gCannotExportNowDialog = false;
// 		if( !gCannotExportNowDialog )
// 			FBMessageBoxWithCheck("Mocap Device Info:",
// 			"Cannot export now as importing is in progress",
// 			"OK", NULL, NULL,
// 			"Don't show this dialog", gCannotExportNowDialog
// 			);
// 		return;
// 	}
// 
// 	if( mDevice->GetFileIOStatus() == eFileIOExport_WaitForCalibration ||
// 		mDevice->GetFileIOStatus() == eFileIOExport_Start )
// 	{
// 		mDevice->SetFileIOStatus(eFileIOExport_Stop);
// 		mButtonExport.Caption = "Start Exporting";
// 	}
// 	else 
// 	{
// 		mDevice->SetFileIOStatus(eFileIOExport_WaitForCalibration);
// 		mButtonExport.Caption = "Stop Exporting"; 
// 	}    
// }

// void ORDeviceMocapLayout::EventFileIOImport( HISender pSender, HKEvent pEvent )
// {
// 	if( mDevice->GetFileIOStatus() == eFileIOExport_WaitForCalibration ||
// 		mDevice->GetFileIOStatus() == eFileIOExport_Start )
// 	{
// 		static bool gCannotImportNowDialog = false;
// 		if( !gCannotImportNowDialog )
// 			FBMessageBoxWithCheck("Mocap Device Info:",
// 			"Cannot import now as exporting is in progress",
// 			"OK", NULL, NULL,
// 			"Don't show this dialog", gCannotImportNowDialog
// 			);
// 		return;
// 	}
// 
// 	mDevice->SetFileIOStatus(eFileIOImport_Loading);
// }

//void ORDeviceMocapLayout::EventFileIOStop( HISender pSender, HKEvent pEvent )
//{
//    mDevice->SetFileIOStatus(eStop);
//}

bool ORDeviceMocapLayout::IsMocapDataFileExisting()
{
// 	FBString lFile = FBString( FBSystem().TheOne().UserConfigPath ) + "//..//recording//mocap.txt";
// 	if( FileExist( lFile ) )
// 		return true;
	return false;
}   

void ORDeviceMocapLayout::EventSetAP(HISender pSender, HKEvent pEvent  )
{
	if (mEditAP.Text)
	{
		string tempstr;
		string str = ":";
		tempstr = mEditAP.Text;
		for (int i=0;i<tempstr.length();i++)
		{
			if ((tempstr.at(i)) == str[0])
			{
				int size = sizeof(tempstr.substr(0,i).c_str());
				bool sig = mDevice->setAP(tempstr.substr(0,i).c_str(),size,(int)(tempstr.substr(i+1).c_str()));

				if (sig)
				{
					mDevice->setAPSetStatus(true);
					return;
				}
				else
				{
					mDevice->setAPSetStatus(false);
				}
			}


		}
	}
}


/*void ORDeviceMocapLayout::EnableDisableButtons()
{
	if( mDevice->GetOperationState() >= eAskForPressSetupRecording )
	{
		if( !mButtonRecording.Enabled )
			mButtonRecording.Enabled = true;
	}
	else
	{
		if( mButtonRecording.Enabled )
			mButtonRecording.Enabled = false;
	}

	if( mDevice->GetOperationState() >= eAskForPressCalibration )
	{
		if( !mButtonCalibration.Enabled )
			mButtonCalibration.Enabled = true;
	}
	else
	{
		if( mButtonCalibration.Enabled )
			mButtonCalibration.Enabled = false;
	}
}*/

