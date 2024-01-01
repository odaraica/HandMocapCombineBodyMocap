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

#include "speech.h"
#include <fbsdk/fbsdk.h>

Speech::Speech():
    mNuiSensor(NULL),
    mKinectAudioStream(NULL),
    mSpeechStream(NULL),
    mSpeechRecognizer(NULL),
    mSpeechContext(NULL),
    mSpeechGrammar(NULL),
    mValid(false)
{
    // Get the path for grammar file
    FBString lBinPath( FBSystem().ApplicationPath );
    FBString lFilePath = lBinPath + "//..//..//OpenRealitySDK//Scenes//speechwords.grxml";
    size_t lNewSize = strlen(lFilePath) + 1;
    wchar_t* lWFilePath = new wchar_t[lNewSize];
    size_t lConvertedChars = 0;
    mbstowcs_s(&lConvertedChars, lWFilePath, lNewSize, lFilePath, _TRUNCATE);
    mGrammarFileName = lWFilePath;    
}

Speech::~Speech()
{
    delete[] mGrammarFileName;
    SafeRelease(mKinectAudioStream);
    SafeRelease(mSpeechStream);
    SafeRelease(mSpeechRecognizer);
    SafeRelease(mSpeechContext);
    SafeRelease(mSpeechGrammar);
}


/// <summary>
/// Create the first connected Kinect found.
/// </summary>
/// <returns>S_OK on success, otherwise failure code.</returns>
HRESULT Speech::CreateFirstConnected(INuiSensor* pSensor)
{
    mNuiSensor = pSensor;

    HRESULT hr;
    if (NULL == mNuiSensor)
    {
        SetStatusMessage("No ready Kinect found! \n\nBut you can still use this plugin without voice control feature.");
        return E_FAIL;
    }

    hr = InitializeAudioStream();
    if (FAILED(hr))
    {
        SetStatusMessage("Could not initialize audio stream. \n\nBut you can still use this plugin without voice control feature.");
        return hr;
    }

    hr = CreateSpeechRecognizer();
    if (FAILED(hr))
    {
        SetStatusMessage("Could not create speech recognizer. Please ensure that Microsoft Speech SDK and other sample requirements are installed.  \n\nBut you can still use this plugin without voice control feature.");
        return hr;
    }

    hr = LoadSpeechGrammar();
    if (FAILED(hr))
    {
        SetStatusMessage("Could not load speech grammar. Please ensure that grammar configuration file was properly deployed.  \n\nBut you can still use this plugin without voice control feature.");
        return hr;
    }

    hr = StartSpeechRecognition();
    if (FAILED(hr))
    {
        SetStatusMessage("Could not start recognizing speech. \n\nBut you can still use this plugin without voice control feature.");
        return hr;
    }

    mValid = true;

    return hr;
}

HRESULT Speech::InitializeAudioStream()
{
    // Get the audio source
    if (NULL == mNuiSensor)
    {
        SetStatusMessage("No ready Kinect found!");
        return E_FAIL;
    }

    INuiAudioBeam*      pNuiAudioSource = NULL;
    IMediaObject*       pDMO = NULL;
    IPropertyStore*     pPropertyStore = NULL;
    IStream*            pStream = NULL;

    HRESULT hr = mNuiSensor->NuiGetAudioSource(&pNuiAudioSource);
    if (SUCCEEDED(hr))
    {
        hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&pDMO);

        if (SUCCEEDED(hr))
        {
            hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore);

            // Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
            // Possible values are:
            //   SINGLE_CHANNEL_AEC = 0
            //   OPTIBEAM_ARRAY_ONLY = 2
            //   OPTIBEAM_ARRAY_AND_AEC = 4
            //   SINGLE_CHANNEL_NSAGC = 5
            PROPVARIANT pvSysMode;
            PropVariantInit(&pvSysMode);
            pvSysMode.vt = VT_I4;
            pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
            pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
            PropVariantClear(&pvSysMode);

            // Set DMO output format
            WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
            DMO_MEDIA_TYPE mt = {0};
            MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

            mt.majortype = MEDIATYPE_Audio;
            mt.subtype = MEDIASUBTYPE_PCM;
            mt.lSampleSize = 0;
            mt.bFixedSizeSamples = TRUE;
            mt.bTemporalCompression = FALSE;
            mt.formattype = FORMAT_WaveFormatEx;	
            memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

            hr = pDMO->SetOutputType(0, &mt, 0);

            if (SUCCEEDED(hr))
            {
                mKinectAudioStream = new KinectAudioStream(pDMO);

                hr = mKinectAudioStream->QueryInterface(IID_IStream, (void**)&pStream);

                if (SUCCEEDED(hr))
                {
                    hr = CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&mSpeechStream);

                    if (SUCCEEDED(hr))
                    {
                        hr = mSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &wfxOut);
                    }
                }
            }

            MoFreeMediaType(&mt);
        }
    }

    SafeRelease(pStream);
    SafeRelease(pPropertyStore);
    SafeRelease(pDMO);
    SafeRelease(pNuiAudioSource);

    return hr;
}

HRESULT Speech::StartSpeechRecognition()
{
    HRESULT hr = mKinectAudioStream->StartCapture();

    if (SUCCEEDED(hr))
    {
        // Specify that all top level rules in grammar are now active
        mSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);

        // Specify that engine should always be reading audio
        mSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

        // Specify that we're only interested in receiving recognition events
        mSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));

        // Ensure that engine is recognizing speech and not in paused state
        hr = mSpeechContext->Resume(0);
    }

    return hr;
}

HRESULT Speech::LoadSpeechGrammar()
{
    HRESULT hr = mSpeechContext->CreateGrammar(1, &mSpeechGrammar);

    if (SUCCEEDED(hr))
    {
        // Populate recognition grammar from file
        TCHAR lBuffer[1000];
        GetFullPathName(mGrammarFileName, 1000, lBuffer, NULL);
        hr = mSpeechGrammar->LoadCmdFromFile(lBuffer, SPLO_STATIC);
    }

    return hr;
}

SpeechCommands Speech::Process()
{
    if( !mValid )
        return eNoCommand;

    const float ConfidenceThreshold = 0.3f;

    SPEVENT curEvent;
    ULONG fetched = 0;
    HRESULT hr = S_OK;

    mSpeechContext->GetEvents(1, &curEvent, &fetched);

    while (fetched > 0)
    {
        switch (curEvent.eEventId)
        {
        case SPEI_RECOGNITION:
            if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
            {
                // this is an ISpRecoResult
                ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                SPPHRASE* pPhrase = NULL;

                hr = result->GetPhrase(&pPhrase);
                if (SUCCEEDED(hr))
                {
                    if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
                    {
                        const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
                        if (pSemanticTag->SREngineConfidence > ConfidenceThreshold)
                        {
                            SpeechCommands action = MapSpeechTagToAction(pSemanticTag->pszValue);
                            return action;
                        }
                    }
                    ::CoTaskMemFree(pPhrase);
                }
            }
            break;
        }

        mSpeechContext->GetEvents(1, &curEvent, &fetched);
    }

    return eNoCommand;
}

bool Speech::IsValid()
{
    return mValid;
}

SpeechCommands Speech::MapSpeechTagToAction( LPCWSTR pszSpeechTag )
{
    struct SpeechTagToAction
    {
        LPCWSTR pszSpeechTag;
        SpeechCommands action;
    };
    const SpeechTagToAction Map[] =
    {
        {L"RECORD_START", eStartRecording},
        {L"RECORD_STOP", eStopRecording},
        {L"PLAY", ePlay},
        {L"RESET", eReset}        
    };

    SpeechCommands action = eNoCommand;

    for (int i = 0; i < _countof(Map); ++i)
    {
        if (0 == wcscmp(Map[i].pszSpeechTag, pszSpeechTag))
        {
            action = Map[i].action;
            break;
        }
    }

    return action;
}

HRESULT Speech::CreateSpeechRecognizer()
{
    ISpObjectToken *pEngineToken = NULL;

    HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&mSpeechRecognizer);

    if (SUCCEEDED(hr))
    {
        mSpeechRecognizer->SetInput(mSpeechStream, FALSE);
        hr = SpFindBestToken(SPCAT_RECOGNIZERS,L"Language=409;Kinect=True",NULL,&pEngineToken);

        if (SUCCEEDED(hr))
        {
            mSpeechRecognizer->SetRecognizer(pEngineToken);
            hr = mSpeechRecognizer->CreateRecoContext(&mSpeechContext);

            // For long recognition sessions (a few hours or more), it may be beneficial to turn off adaptation of the acoustic model. 
            // This will prevent recognition accuracy from degrading over time.
            //if (SUCCEEDED(hr))
            //{
            //    hr = m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);                
            //}
        }
    }

    SafeRelease(pEngineToken);

    return hr;
}

void Speech::SetStatusMessage(const char* szMessage)
{
    FBMessageBox( "Mocap Device Error:", 
        szMessage,
        "OK");
}

void Speech::Stop()
{
    if( mKinectAudioStream )
        mKinectAudioStream->StopCapture();
    if( mSpeechStream )
        mSpeechStream->Close();
    mValid = false;
}


