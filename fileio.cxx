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

#include "fileio.h"
#include <string>
#include <fbsdk/fbsdk.h>
#include <direct.h>
#include <cstdlib>
#include <sys/timeb.h>

using namespace std;

bool FileExist( const char* pFileName )
{
    bool lReturn = false;

    FILE* lFile = NULL;
    fopen_s( &lFile, pFileName, "r" );

    if( lFile != NULL )
    {
        lReturn = true;
        fclose(lFile);
    }

    return lReturn;
}

FileIO::FileIO():
    mFilePath(""),
    mFile(NULL),
    mStatus(eFileIOExport_Stop),
    mCurrentIndex(0)
{}

FileIO::~FileIO()
{
    delete mFile;
    mFile = NULL;
    ClearMocapJointsStateArray();
}

const char* gRecordingDir = "//..//recording";
const char* gMocapFile = "//mocap.txt";
bool FileIO::Open(bool pReadOnly)
{
    FBString lDir = FBString( FBSystem().TheOne().UserConfigPath ) + gRecordingDir;
    if( !FileExist( lDir ) )
    {
        _mkdir(lDir);
    }
    lDir = FBString(lDir) + gMocapFile;
    mFilePath = lDir;

    if( pReadOnly )
        fopen_s(&mFile, mFilePath,"r");
    else
        fopen_s(&mFile, mFilePath,"a+");
    if(mFile != NULL)
        return true;

    return false;
}

const int gMaxBufferSize = 200;
const int gFixedJointCount = 20;
const char* gEmptyFrameStr = "EMPTY_FRAME\n";
const char* gValidFrameStr = "VALID_FRAME\n";
const char* gValidFrameEndStr = "VALID_FRAME_END\n";
const char* gFloorClipPlaneWStr = "FloorClipPlane\n";
const char* gSkeletonJointsCountStr = "SkeletonJointsCount\n";
const char* gSkeletonPositionStr = "SkeletonPosition\n";
const char* gSkeletonTrackingStateStr = "SkeletonTrackingState\n";
const char* gSkeletonJointsPositionsStr = "SkeletonJointsPositions\n";
const char* gSkeletonJointsGlobalOrientationsStr = "SkeletonJointsGlobalOrientations\n";
const char* gSkeletonJointTrackingStateStr = "SkeletonJointTrackingState\n";

bool FileIO::Export(FBMocapJointsState* pState)
{
    if( mStatus != eFileIOExport_Start )
        return false; 

    if( !Open() )
        return false;

    if( pState->mSkeletonTrackingState != kFBSkeletonTracked || 
        pState->mSkeletonJointTrackingState[0] != kFBSkeletonTracked )
    {
        Write("\n");
        Write(gEmptyFrameStr);
        Close();
        return true;
    }

    Write("\n");
    WriteTitle(gValidFrameStr);

    WriteTitle(gFloorClipPlaneWStr);
    Write(pState->mFloorClipPlaneW);
    Write("\n");

    WriteTitle(gSkeletonJointsCountStr);
    Write(pState->mSkeletonJointsCount);
    Write("\n");

    WriteTitle(gSkeletonPositionStr);
    Write(pState->mSkeletonPosition);
    Write("\n");
    
    WriteTitle(gSkeletonTrackingStateStr);
    Write((int)(pState->mSkeletonTrackingState));
    Write("\n");

    WriteTitle(gSkeletonJointsPositionsStr);
    for(int i=0; i<pState->mSkeletonJointsCount; i++)
    {
        Write(pState->mSkeletonJointsPositions[i]);
        Write("\n");
    }

    WriteTitle(gSkeletonJointsGlobalOrientationsStr);
    for(int i=0; i<pState->mSkeletonJointsCount; i++)
    {
        Write(pState->mSkeletonJointsGlobalOrientations[i]);
        Write("\n");
    }

    WriteTitle(gSkeletonJointTrackingStateStr);
    for(int i=0; i<pState->mSkeletonJointsCount; i++)
    {
        Write(pState->mSkeletonJointTrackingState[i]);
        Write("\n");
    }

    Write("\n");
    WriteTitle(gValidFrameEndStr);
    Write("\n");

    Close();
    
    return true;
}

void FileIO::Close()
{
    if(mFile)
        fclose(mFile);
    mFile = NULL;
}

bool FileIO::FetchMocapData(FBMocapJointsState* pKinectMocapJointsState)
{
    if( mStatus == eFileIOImport_Start && 
        mCurrentIndex < mMocapJointsStateArray.GetCount() )
    {
        // Do data copy
        FBMocapJointsState* lTemp = mMocapJointsStateArray.GetAt(mCurrentIndex);
        pKinectMocapJointsState->mSkeletonJointsCount = lTemp->mSkeletonJointsCount;
        pKinectMocapJointsState->mSkeletonTrackingState = lTemp->mSkeletonTrackingState;
        pKinectMocapJointsState->mSkeletonPosition = lTemp->mSkeletonPosition;
        pKinectMocapJointsState->mFloorClipPlaneW = lTemp->mFloorClipPlaneW;
        for(int i=0; i<pKinectMocapJointsState->mSkeletonJointsCount; i++)
        {
            pKinectMocapJointsState->mSkeletonJointTrackingState[i] = lTemp->mSkeletonJointTrackingState[i];
            pKinectMocapJointsState->mSkeletonJointsPositions[i] = lTemp->mSkeletonJointsPositions[i];
            pKinectMocapJointsState->mSkeletonJointsGlobalOrientations[i] = lTemp->mSkeletonJointsGlobalOrientations[i];
        }
        mCurrentIndex++;
        return true;
    }

    return false;
}

void FileIO::WriteTitle(const char* pS)
{
    fputs("\n", mFile);
    fputs(pS, mFile);
    fputs("\n", mFile);
}

void FileIO::Write(const char* pS)
{
    fputs(pS, mFile);
}

void FileIO::Write(const int& pNumber)
{
    string s;
    s = std::to_string( pNumber );
    fputs(s.c_str(), mFile);
}

void FileIO::Write(const double& pNumber)
{
    string s;
    s = std::to_string( pNumber );
    fputs(s.c_str(), mFile);
}

void FileIO::Write(const FBTVector& pV)
{
    string s;
    for(int i=0; i<3; i++)
    {
        double d = pV[i];
        s = std::to_string( d );
        fputs(s.c_str(), mFile);
        fputs("\t", mFile);
    }
}

void FileIO::Write(const FBMatrix& pM)
{
    string s;
    for(int i=0; i<16; i++)
    {
        double d = pM[i];
        s = std::to_string( d );
        fputs(s.c_str(), mFile);
        fputs("\t", mFile);
    }
}

bool FileIO::StringToDouble(const char* pString, double& pNumber)
{
    try
    {
        pNumber = atof(pString);
    }
    catch(const std::invalid_argument&)
    {
        return false;
    }
    catch(const std::out_of_range&) 
    {
        return false;
    }
    return true;
}

bool FileIO::Load()
{
    if(Open(true))
    {
        char lBuffer[gMaxBufferSize];
        while( fgets(lBuffer, gMaxBufferSize, mFile) )
        {
            if(strcmp(lBuffer, "\n") == 0)
            {
                continue;
            }
            else if(strcmp(lBuffer, gEmptyFrameStr) == 0)
            {
                LoadEmptyFrame();            
            }
            else if(strcmp(lBuffer, gValidFrameStr) == 0)
            {
                LoadValidFrame();            
            }
        }
        Close();
    }
    return false;
}

bool FileIO::LoadEmptyFrame()
{
    return true;
}

bool FileIO::LoadValidFrame()
{
    FBMocapJointsState* lState = new FBMocapJointsState(gFixedJointCount);
    char lBuffer[gMaxBufferSize];

    while( fgets(lBuffer, gMaxBufferSize, mFile) )
    {
        if(strcmp(lBuffer, "\n") == 0 )
        {
            continue;
        }
        else if(strcmp(lBuffer, gSkeletonJointsCountStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            fgets(lBuffer, gMaxBufferSize, mFile);
            ReadInt(lBuffer, lState->mSkeletonJointsCount);
        }
        else if(strcmp(lBuffer, gFloorClipPlaneWStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            fgets(lBuffer, gMaxBufferSize, mFile);
            ReadDouble(lBuffer, lState->mFloorClipPlaneW);
        }
        else if(strcmp(lBuffer, gSkeletonPositionStr) == 0)
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            fgets(lBuffer, gMaxBufferSize, mFile);
            ReadVector(lBuffer, lState->mSkeletonPosition);
        }
        else if(strcmp(lBuffer, gSkeletonTrackingStateStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            int lTemp;
            fgets(lBuffer, gMaxBufferSize, mFile);
            ReadInt(lBuffer, lTemp);
            lState->mSkeletonTrackingState = FBSkeletonTrackingState(lTemp);
        }
        else if(strcmp(lBuffer, gSkeletonJointsPositionsStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            for(int i=0; i<gFixedJointCount; i++)
            {
                fgets(lBuffer, gMaxBufferSize, mFile);
                ReadVector(lBuffer, lState->mSkeletonJointsPositions[i]);
            }
        }
        else if(strcmp(lBuffer, gSkeletonJointsGlobalOrientationsStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            for(int i=0; i<gFixedJointCount; i++)
            {
                fgets(lBuffer, gMaxBufferSize, mFile);
                ReadMatrix(lBuffer, lState->mSkeletonJointsGlobalOrientations[i]);
            }
        }
        else if(strcmp(lBuffer, gSkeletonJointTrackingStateStr) == 0 )
        {
            fgets(lBuffer, gMaxBufferSize, mFile); // for /n
            for(int i=0; i<gFixedJointCount; i++)
            {
                int lTemp;
                fgets(lBuffer, gMaxBufferSize, mFile);
                ReadInt(lBuffer, lTemp);
                lState->mSkeletonJointTrackingState[i] = FBSkeletonJointTrackingState(lTemp);
            }
        }
        else if(strcmp(lBuffer, gValidFrameEndStr) == 0 )
        {
            break;
        }
    }

    mMocapJointsStateArray.Add(lState);
    return true;
}

bool FileIO::ReadLine(char* pBuffer, double* pData, int pLength)
{
    string str(pBuffer);
    string str2;
    int i = 0;
    size_t lPos = 0;

    lPos = str.find("\t");
    if(lPos == string::npos)
    {
        StringToDouble(str.c_str(), pData[0]);
        return true;
    }

    do 
    {
        str2 = str.substr(0, lPos);
        double lTemp = 0.0;
        StringToDouble(str2.c_str(), lTemp);
        assert(i<pLength);
        pData[i] = lTemp;
        i++;
        str.erase(0, lPos + 1);
        lPos = str.find("\t");
    } while (lPos != string::npos);

    return true;
}

bool FileIO::ReadInt(char* pBuffer, int& pNumber)
{
    double lNumber;
    ReadLine(pBuffer, &lNumber, 1);
    pNumber = (int) lNumber;
    return true;
}

bool FileIO::ReadDouble(char* pBuffer, double& pNumber)
{
    return ReadLine(pBuffer, &pNumber, 1);
}

bool FileIO::ReadVector(char* pBuffer, FBTVector& pV)
{
    return ReadLine(pBuffer, &(pV.mValue[0]), 3);
}

bool FileIO::ReadMatrix(char* pBuffer, FBMatrix& pM)
{
    double lTemp[16];
    memset(lTemp, 0.0, sizeof(double) * 16);
    ReadLine(pBuffer, lTemp, 16);
    pM = FBMatrix(lTemp);
    return true;
}

FileIOStatus FileIO::GetStatus()
{
    return mStatus;
}

void FileIO::SetStatus( FileIOStatus pState )
{
    if( pState == eFileIOImport_Start && mCurrentIndex == mMocapJointsStateArray.GetCount() )
        mCurrentIndex = 0;     // reset index to start

    if( pState == eFileIOImport_Loading )
    {
        mCurrentIndex = 0;
        ClearMocapJointsStateArray();
        Close();
        Load();
        pState = eFileIOImport_WaitForCalibration;
    }

    mStatus = pState;
}

void FileIO::ClearMocapJointsStateArray()
{
    for(int i=0; i<mMocapJointsStateArray.GetCount(); i++)
    {
        FBMocapJointsState* lState = mMocapJointsStateArray[i];
        delete lState;
    }
    mMocapJointsStateArray.Clear();
}
