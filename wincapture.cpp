#include "wincapture.h"
#include "gdiplus.h"

#include <dshow.h>
#include <cstdio>
#include <strsafe.h>
#include <iostream>

namespace mswin32 {

namespace {
HWND ghApp=0;
IVideoWindow  * g_pVW = NULL;
IMediaControl * g_pMC = NULL;
IMediaEventEx * g_pME = NULL;
IGraphBuilder * g_pGraph = NULL;
ICaptureGraphBuilder2 * g_pCapture = NULL;
PLAYSTATE g_psCurrent = Stopped;

IBaseFilter *pGrabberFilter = NULL;
ISampleGrabber *pGrabber = NULL;

long pBufferSize = 0;
unsigned char* pBuffer = 0;

unsigned int gWidth = 0;
unsigned int gHeight = 0;
unsigned int gChannels = 0;

ULONG_PTR gdiToken = 0;
CLSID pBmpEncoder = GUID_NULL;
}

void freeMediaType(AM_MEDIA_TYPE& mt);
HRESULT addSampleGrabber(IGraphBuilder *pGraph);
HRESULT setSampleGrabberMediaType();
HRESULT getSampleGrabberMediaType();

void closeSampleGrabber()
{
    if (pBuffer != 0)
    {
        delete[] pBuffer;
        pBuffer = 0;
        pBufferSize = 0;
    }

    SAFE_RELEASE(pGrabberFilter);
    SAFE_RELEASE(pGrabber);

    gWidth = 0;
    gHeight = 0;
    gChannels = 0;
}

HRESULT addSampleGrabber(IGraphBuilder *pGraph)
{
    // Create the Sample Grabber.
    HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IBaseFilter, (void**) & pGrabberFilter);
    if (FAILED(hr))
    {
        return hr;
    }
    hr = pGraph->AddFilter(pGrabberFilter, L"SGrabber");
    if (FAILED(hr))
    {
        return hr;
    }

    pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);

    return hr;
}

HRESULT setSampleGrabberMediaType()
{
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    HRESULT hr = pGrabber->SetMediaType(&mt);
    if (FAILED(hr))
    {
        return hr;
    }
    hr = pGrabber->SetOneShot(FALSE);
    hr = pGrabber->SetBufferSamples(TRUE);
    return hr;
}

HRESULT getSampleGrabberMediaType()
{
    AM_MEDIA_TYPE mt;
    HRESULT hr = pGrabber->GetConnectedMediaType(&mt);
    if (FAILED(hr))
    {
        return hr;
    }

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER *)mt.pbFormat;
    gChannels = pVih->bmiHeader.biBitCount / 8;
    gWidth = pVih->bmiHeader.biWidth;
    gHeight = pVih->bmiHeader.biHeight;

    freeMediaType(mt);
    return hr;
}

unsigned char* GetFrame()
{
    HRESULT hr;

    if (pGrabber == 0)  return 0;

    long Size = 0;
    hr = pGrabber->GetCurrentBuffer(&Size, NULL);
    if (FAILED(hr))
    {
        return 0;
    }
    else if (Size != pBufferSize)
    {
        pBufferSize = Size;
        if (pBuffer != 0)
        {
            delete[] pBuffer;
        }
        pBuffer = new unsigned char[pBufferSize];
    }

    hr = pGrabber->GetCurrentBuffer(&pBufferSize, (long*)pBuffer);
    if (FAILED(hr))
    {
        return 0;
    }

    // the buffer here is upside down
    return pBuffer;
}

void freeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

HRESULT GetInterfaces(void)
{
    HRESULT hr;

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (void **) &g_pGraph);
    if (FAILED(hr))  return hr;

    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void **) &g_pCapture);
    if (FAILED(hr))  return hr;

    // Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &g_pMC);
    if (FAILED(hr))  return hr;

    hr = g_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *) &g_pVW);
    if (FAILED(hr))  return hr;

    hr = g_pGraph->QueryInterface(IID_IMediaEventEx, (LPVOID *) &g_pME);
    if (FAILED(hr))  return hr;

    // Set the window handle used to process graph events
    hr = g_pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0);

    return hr;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);

    if (size == 0)
    {
        return -1;    // Failure
    }

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));

    if (pImageCodecInfo == NULL)
    {
        return -1;    // Failure
    }

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    (void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(NULL, szBuffer, TEXT("QuickScan 1.0"), MB_OK | MB_ICONERROR);
}

HRESULT CaptureVideo()
{
    HRESULT hr;
    IBaseFilter *pSrcFilter=NULL;

    // Get DirectShow interfaces
    hr = GetInterfaces();
    if (FAILED(hr))
    {
        //Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Failed to get video interfaces!"));
        return hr;
    }

    // Attach the filter graph to the capture graph
    hr = g_pCapture->SetFiltergraph(g_pGraph);
    if (FAILED(hr))
    {
        //Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Failed to set capture filter graph!"));
        return hr;
    }

    // Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.
    hr = FindCaptureDevice(&pSrcFilter);
    if (FAILED(hr))
    {
        // Don't display a message because FindCaptureDevice will handle it
        return hr;
    }

    // Add Capture filter to our graph.
    hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
        //Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
        Msg(TEXT("Couldn't add the capture filter to the graph!  \r\n\r\n")
            TEXT("If you have a working video capture device, please make sure\r\n")
            TEXT("that it is connected and is not being used by another application.\r\n\r\n")
            //TEXT("The sample will now close."), hr);
            TEXT("The sample will now close."));
        pSrcFilter->Release();
        return hr;
    }

    hr = addSampleGrabber(g_pGraph);

    if (FAILED(hr))
    {
        //Msg(TEXT("Couldn't add the SampleGrabber filter to the graph!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Couldn't add the filter to the graph!"));
        return hr;
    }

    hr = setSampleGrabberMediaType();

    if (FAILED(hr))
    {
        //Msg(TEXT("Couldn't set the SampleGrabber media type!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Couldn't set the media type!"));
        return hr;
    }

    // Render the preview pin on the video capture filter
    // Use this instead of g_pGraph->RenderFile
    hr = g_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                  pSrcFilter, pGrabberFilter/*NULL*/, NULL);

    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
            TEXT("The capture device may already be in use by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

    hr = getSampleGrabberMediaType();

    // Now that the filter has been added to the graph and we have
    // rendered its stream, we can release this reference to the filter.
    pSrcFilter->Release();

    /////////////////////////////////////////////////////////////////////////

    // Set the video window to be a child of the main window
    hr = g_pVW->put_Owner((OAHWND)ghApp);
    if (FAILED(hr))
    {
        //Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Couldn't initialize video window!"));
        return hr;
    }
    // Set video window style
    hr = g_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
    if (FAILED(hr))
        return hr;

    // Use helper function to position video window in client rect
    // of main application window
    //ResizeVideoWindow();

    // Make the video window visible, now that it is properly positioned
    hr = g_pVW->put_Visible(OAFALSE);
    if (FAILED(hr))  return hr;

    /////////////////////////////////////////////////////////////////////////

    // Start previewing video data
    hr = g_pMC->Run();
    if (FAILED(hr))
    {
        //Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
        // friendly message
        Msg(TEXT("Couldn't run the graph!"));
        return hr;
    }

    // Remember current state
    g_psCurrent = Running;

    //std::cout << "live capture..\n";

    return S_OK;
}

HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr = S_OK;
    IBaseFilter * pSrc = NULL;
    IMoniker* pMoniker =NULL;
    ICreateDevEnum *pDevEnum =NULL;
    IEnumMoniker *pClassEnum = NULL;

    if (!ppSrcFilter)
    {
        return E_POINTER;
    }

    // Create the system device enumerator
    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
    }

    // Create an enumerator for the video capture devices

    if (SUCCEEDED(hr))
    {
        hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // If there are no enumerators for the requested type, then
        // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
        if (pClassEnum == NULL)
        {
            MessageBox(ghApp,TEXT("No video capture device was detected.\r\n\r\n")
                TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                TEXT("to be installed and working properly.  The sample will now close."),
                TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
            hr = E_FAIL;
        }
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.

    if (SUCCEEDED(hr))
    {
        hr = pClassEnum->Next (1, &pMoniker, NULL);
        if (hr == S_FALSE)
        {
            Msg(TEXT("Unable to access video capture device!"));
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
        }
    }

    // Copy the found filter pointer to the output parameter.
    if (SUCCEEDED(hr))
    {
        *ppSrcFilter = pSrc;
        (*ppSrcFilter)->AddRef();
    }

    SAFE_RELEASE(pSrc);
    SAFE_RELEASE(pMoniker);
    SAFE_RELEASE(pDevEnum);
    SAFE_RELEASE(pClassEnum);

    return hr;
}

void CloseInterfaces(void)
{
    // Stop previewing data
    if (g_pMC)
        g_pMC->StopWhenReady();

    g_psCurrent = Stopped;

    // Stop receiving events
    if (g_pME)
        g_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

    // Relinquish ownership (IMPORTANT!) of the video window.
    // Failing to call put_Owner can lead to assert failures within
    // the video renderer, as it still assumes that it has a valid
    // parent window.
    if(g_pVW)
    {
        g_pVW->put_Visible(OAFALSE);
        g_pVW->put_Owner(NULL);
    }

#ifdef REGISTER_FILTERGRAPH
    // Remove filter graph from the running object table
    if (g_dwGraphRegister)
        RemoveGraphFromRot(g_dwGraphRegister);
#endif

    // Release DirectShow interfaces
    SAFE_RELEASE(g_pMC);
    SAFE_RELEASE(g_pME);
    SAFE_RELEASE(g_pVW);
    SAFE_RELEASE(g_pGraph);
    SAFE_RELEASE(g_pCapture);
}

HRESULT ChangePreviewState(int nShow)
{
    HRESULT hr=S_OK;

    // If the media control interface isn't ready, don't call it
    if (!g_pMC)  return S_OK;

    if (nShow)
    {
        if (g_psCurrent != Running)
        {
            // Start previewing video data
            hr = g_pMC->Run();
            g_psCurrent = Running;
        }
    }
    else
    {
        // Stop previewing video data
        hr = g_pMC->StopWhenReady();
        g_psCurrent = Stopped;
    }

    return hr;
}

void ResizeVideoWindow(void)
{
    // Resize the video preview window to match owner window size
    if (g_pVW)
    {
        RECT rc;

        // Make the preview video fill our window
        GetClientRect(ghApp, &rc);
        g_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
    }
}

void CleanUp()
{
    CloseInterfaces();  // Stop capturing and release interfaces
    CoUninitialize();
    Gdiplus::GdiplusShutdown(gdiToken);
}

void Initialize(HWND hwnd)
{
    ghApp = hwnd;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));
        exit(1);
    }

    // call 3 lines below before obtaining the encoder command
    Gdiplus::GdiplusStartupInput input;
    GdiplusStartup(&gdiToken, &input, NULL);

    if (GetEncoderClsid(L"image/jpeg", &pBmpEncoder) < 0)
    {
        Msg(TEXT("Failed to get image/jpeg encoder"));
    }

    if(ghApp)
    {
        HRESULT hr;

        // Create DirectShow graph and start capturing video
        hr = CaptureVideo();
        if (FAILED (hr))
        {
            CloseInterfaces();
            DestroyWindow(ghApp);
        }
    }
}

int GetFrameWidth()
{
    return gWidth;
}

int GetFrameHeight()
{
    return gHeight;
}

}
