//
// Sample DirectShow player that uses
// GDCL Multigraph Framework to play a sequence of
// multiple clips
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk


#pragma once

#include "..\GMFBridge_h.h"
_COM_SMARTPTR_TYPEDEF(IGMFBridgeController, __uuidof(IGMFBridgeController));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, IID_IGraphBuilder);
_COM_SMARTPTR_TYPEDEF(IMediaControl, IID_IMediaControl);
_COM_SMARTPTR_TYPEDEF(IMediaSeeking, IID_IMediaSeeking);
_COM_SMARTPTR_TYPEDEF(IMediaEventEx, IID_IMediaEventEx);
_COM_SMARTPTR_TYPEDEF(IVideoWindow, IID_IVideoWindow);
_COM_SMARTPTR_TYPEDEF(IBasicVideo, IID_IBasicVideo);

#include <string>
#include <list>
using namespace std;

class ClipEntry
{
public:
	ClipEntry();
	ClipEntry(const ClipEntry& r);
	~ClipEntry();
	const ClipEntry& operator=(const ClipEntry& r);

	HRESULT Create(IGMFBridgeController* pController, const char* path);

	HRESULT SetLimits(REFERENCE_TIME tStart, REFERENCE_TIME tStop);
    void GetLimits(REFERENCE_TIME* ptStart, REFERENCE_TIME* ptStop)
    {
        *ptStart = m_tStart;
        *ptStop = m_tStop;
    }

	REFERENCE_TIME Duration();
	REFERENCE_TIME CurrentPosition();
	
	IUnknown* Graph()
	{
		return m_pGraph;
	}
	IUnknown* SinkFilter()
	{
		return m_pSinkFilter;
	}
	string Name()
	{
		return m_strName;
	}
	void SetStartPosition(REFERENCE_TIME tOffset)
	{
		m_tOffset = tOffset;
	}
	REFERENCE_TIME GetStartPosition()
	{
		return m_tOffset;
	}
	HRESULT SetPosition(REFERENCE_TIME tStart);

    //  has this graph been started/rewound ready to go?
    bool IsPrimed()
    {
        return m_bPrimed;
    }
    
    // in use: so no longer primed
    void InUse()
    {
        m_bPrimed = false;
    }

    // set position and run
    void Prime(REFERENCE_TIME tStart = 0);
    void Stop();

    // returns whole file duration even when limits applied
    REFERENCE_TIME NativeDuration();

    // updates the graph stop time to reflect the limits (for active clip)
    HRESULT SetStopTime();

private:
	IGraphBuilderPtr m_pGraph;
	IUnknownPtr m_pSinkFilter;
	string m_strName;
	REFERENCE_TIME m_tOffset;
	REFERENCE_TIME m_tStart;
	REFERENCE_TIME m_tStop;

    // rewound ready to re-use
    bool m_bPrimed;
};

class ClipPlayer
{
public:
	ClipPlayer(HWND hwnd, UINT msgSegment, UINT msgEvent);
	~ClipPlayer();

	HRESULT AddClip(const char* path, ClipEntry** ppClip);
	HRESULT Play();
	HRESULT Pause();
	HRESULT Stop();

	void OnEndOfSegment();
	void OnEvent();

	REFERENCE_TIME TotalDuration()
	{
		return m_tDuration;
	}
	REFERENCE_TIME CurrentPosition();
	HRESULT SetPosition(REFERENCE_TIME tStart);
	ClipEntry* CurrentClip();
	void UpdateDuration();
	void SetLoop(bool bLoop);
	bool IsLooping()
	{
		return m_bLoop;
	}
	FILTER_STATE GetState();
	bool IsCued()
	{
		IMediaControlPtr pMC = m_pRenderGraph;
		if (pMC != NULL)
		{
			OAFilterState fs;
            HRESULT hr = pMC->GetState(0, &fs);
			if (hr == VFW_S_STATE_INTERMEDIATE)
			{
				return false;
			}
		}
		return true;
	}

    HRESULT SetClipLimits(ClipEntry* pClip, REFERENCE_TIME tStart, REFERENCE_TIME tEnd);
    ClipEntry* GetClipByIndex(long idx);
    void PlayNext(ClipEntry* pClip);

private:
	HRESULT SeekSourceGraphs();
    void ResizeWindowToVideo();


private:
	IGMFBridgeControllerPtr m_pController;
	list<ClipEntry> m_Clips;
	list<ClipEntry>::iterator m_itCurrent;
	IGraphBuilderPtr m_pRenderGraph;
	IUnknownPtr m_pRenderGraphSourceFilter;
	REFERENCE_TIME m_tDuration;
	REFERENCE_TIME m_tStartPosition;
	bool m_bLoop;
	bool m_bActive;

	HWND m_hwndApp;
	UINT m_msgSegment;
	UINT m_msgEvent;

    // jump to this clip at end of clip
    ClipEntry* m_pPlayNext;
};
