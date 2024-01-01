#ifndef __FILEIO_H__
#define __FILEIO_H__

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


#include <fbsdk/fbsdk.h>

enum FileIOStatus
{
    eFileIOExport_Stop,
    eFileIOExport_WaitForCalibration,
    eFileIOExport_Start,
    eFileIOImport_Loading,
    eFileIOImport_WaitForCalibration,
    eFileIOImport_Start,
    eFileIOImport_Pause,
};

bool FileExist( const char* pFileName );

typedef FBArrayTemplate<FBMocapJointsState*> MocapJointsStateArray;

class FileIO
{
public:
    FileIO();
    ~FileIO();

    bool Export(FBMocapJointsState* pKinectMocapJointsState);
    void Close();
    bool FetchMocapData( FBMocapJointsState* pKinectMocapJointsState );
    FileIOStatus GetStatus();
    void SetStatus( FileIOStatus pState );
    bool Load();

private:
    bool Open(bool pReadOnly = false);
    inline void ClearMocapJointsStateArray();

    // Write
    inline void WriteTitle(const char* pS);
    inline void Write(const char* pS);
    inline void Write(const int& pNumber);
    inline void Write(const double& pNumber);
    inline void Write(const FBTVector& pV);
    inline void Write(const FBMatrix& pM);

    // Read
    bool ReadLine(char* pBuffer, double* pData, int pLength);
    inline bool ReadVector(char* pBuffer, FBTVector& pV);
    inline bool ReadInt(char* pBuffer, int& pNumber);
    inline bool ReadDouble(char* pBuffer, double& pNumber);
    inline bool ReadMatrix(char* pBuffer, FBMatrix& pM);
    bool StringToDouble(const char* pString, double& pNumber);
    bool LoadEmptyFrame();
    bool LoadValidFrame();

private:
    FileIOStatus mStatus;
    const char* mFilePath;
    FILE * mFile;
    MocapJointsStateArray mMocapJointsStateArray;
    int mCurrentIndex;
};

#endif /* __FILEIO_H__ */