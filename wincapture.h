#ifndef WINCAPTURE_H
#define WINCAPTURE_H

#include <windows.h>
#include "strmif.h"


/* This class based on combination of codes from directshow example (playcap), and some codeproject tutorial
 * codeproject for adding filter to grab the frame from the capture source
 * credit to the codeproject author.
 */
namespace mswin32 {
    // Application-defined message to notify app of filtergraph events
    #define WM_GRAPHNOTIFY  WM_APP+1

    // Macros
    #define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

    #define JIF(x) if (FAILED(hr=(x))) \
        {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

    enum PLAYSTATE {Stopped, Paused, Running, Init};
    EXTERN_C const CLSID CLSID_NullRenderer;

    interface ISampleGrabberCB : public IUnknown
    {
        virtual STDMETHODIMP SampleCB( double SampleTime, IMediaSample *pSample ) = 0;
        virtual STDMETHODIMP BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) = 0;
    };

    interface ISampleGrabber : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot ) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType ) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType ) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem ) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( long *pBufferSize, long *pBuffer ) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( IMediaSample **ppSample ) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback, long WhichMethodToCallback ) = 0;
    };

    static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
    static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
    static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

    HRESULT GetInterfaces(void);
    HRESULT CaptureVideo();
    HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);
    HRESULT ChangePreviewState(int nShow);

    void ResizeVideoWindow(void);
    void Msg(TCHAR *szFormat, ...);
    void CloseInterfaces(void);
    void Initialize(HWND hwnd);
    void CleanUp();

    unsigned char* GetFrame();
    int GetFrameWidth();
    int GetFrameHeight();
}

#endif // WINCAPTURE_H
