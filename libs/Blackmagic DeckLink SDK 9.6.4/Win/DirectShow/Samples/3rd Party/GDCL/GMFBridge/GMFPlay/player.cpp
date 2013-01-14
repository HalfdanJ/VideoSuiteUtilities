// 
// GDCL Multigraph Framework
//
// Player.cpp: implementation of sample player,
// using GMFBridge to playback multiple clips as a single seekable
// entity.
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk

#include "StdAfx.h"
#include ".\player.h"

ClipPlayer::ClipPlayer(HWND hwnd, UINT msgSegment, UINT msgEvent)
: m_bLoop(false),
  m_tDuration(0),
  m_tStartPosition(0),
  m_bActive(false),
  m_hwndApp(hwnd),
  m_msgEvent(msgEvent),
  m_msgSegment(msgSegment),
  m_pPlayNext(NULL)
{
	HRESULT hr = m_pController.CreateInstance(__uuidof(GMFBridgeController));
    if (hr != S_OK)
    {
        throw hr;
    }

	m_pController->SetNotify(long(hwnd), long(msgSegment));

	// we use a video and an audio stream,
	// options:
	//don't allow compressed in source graphs, 
	//don't discard when not connected
	m_pController->AddStream(true, eUncompressed, false);
	m_pController->AddStream(false, eUncompressed, false);

	// increase buffering at the join, so that audio does not run out
	m_pController->SetBufferMinimum(200);

	m_itCurrent = m_Clips.end();
}

ClipPlayer::~ClipPlayer()
{
	Stop();
	m_pController->BridgeGraphs(NULL, NULL);
	m_Clips.clear();
}

HRESULT 
ClipPlayer::AddClip(const char* path, ClipEntry** ppClip)
{
	list<ClipEntry>::iterator it = m_Clips.insert(m_Clips.end(), ClipEntry());
	ClipEntry* pClip = &(*it);
	*ppClip = pClip;

	HRESULT hr = pClip->Create(m_pController, path);

	// if we expect both audio and video, then all clips
	// must have both audio and video. 
	// If the first clip is video only, then switch
	// to video-only automatically
	if ((hr == VFW_E_UNSUPPORTED_AUDIO) && (m_Clips.size() == 1))
	{
		// new controller, different options (only one video stream)
		m_pController.CreateInstance(__uuidof(GMFBridgeController));
		m_pController->SetNotify(long(m_hwndApp), long(m_msgSegment));
		m_pController->AddStream(true, eUncompressed, false);
		m_pController->SetBufferMinimum(200);

		// try again
		hr = pClip->Create(m_pController, path);
	}

	if (SUCCEEDED(hr))
	{
		pClip->SetStartPosition(m_tDuration);
		m_tDuration += pClip->Duration();

		// if this is the first clip, create the render graph
		if (m_Clips.size() == 1)
		{
			m_pRenderGraph.CreateInstance(CLSID_FilterGraph);
			hr = m_pController->CreateRenderGraph(pClip->SinkFilter(), m_pRenderGraph, &m_pRenderGraphSourceFilter);
			if (SUCCEEDED(hr) && IsWindow(m_hwndApp))
			{
				IMediaEventExPtr pME = m_pRenderGraph;
				if (pME != NULL)
				{
					pME->SetNotifyWindow(OAHWND(m_hwndApp), m_msgEvent, NULL);
				}
			}
		}
	} else {
		m_Clips.erase(--m_Clips.end());
	}

	return hr;
}

HRESULT 
ClipPlayer::Play()
{
	// make all graphs active
	HRESULT hr = S_OK;
	if (!m_bActive)
	{
		UpdateDuration();
		hr = Pause();
	}

	if (m_itCurrent->Graph() == NULL)
	{
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		IMediaControlPtr pMC = m_pRenderGraph;
		hr = pMC->Run();
	}
	return hr;
}

HRESULT 
ClipPlayer::Pause()
{
	HRESULT hr = S_OK;

    // make all graphs active
	if (!m_bActive)
	{
		// render graph first, or data may be discarded
		IMediaControlPtr pMC = m_pRenderGraph;
		pMC->Pause();

        // seek all source graphs to correct start positions
        SeekSourceGraphs();

		// activate each graph
		list<ClipEntry>::iterator it = m_Clips.begin();
		for(; it != m_Clips.end(); it++)
		{
			IMediaControlPtr pMC = it->Graph();
			pMC->Run();
		}
		m_bActive = true;
	}

	IMediaControlPtr pMC = m_pRenderGraph;
	if (pMC != NULL)
	{
		pMC->Pause();
	}
	return S_OK;
}


HRESULT 
ClipPlayer::Stop()
{
	if (m_bActive)
	{
		// deactivate all graphs
		IMediaControlPtr pMC = m_pRenderGraph;
		if (pMC != NULL)
		{
			pMC->Stop();
		}
		list<ClipEntry>::iterator it = m_Clips.begin();
		for(; it != m_Clips.end(); it++)
		{
            it->Stop();
		}
		m_bActive = false;
	}

	// rewind on stop
	m_itCurrent = m_Clips.end();
	m_tStartPosition = 0;
    m_pPlayNext = FALSE;
	return S_OK;
}


void 
ClipPlayer::OnEndOfSegment()
{
	if (m_itCurrent == m_Clips.end())
	{
		// ? already at the end
		return;
	}

    list<ClipEntry>::iterator itOld = m_itCurrent;

	// locate next graph
	m_itCurrent++;
    if (m_pPlayNext)
    {
        // jump to specified clip
		list<ClipEntry>::iterator it = m_Clips.begin();
		for(; it != m_Clips.end(); it++)
		{
            if (&(*it) == m_pPlayNext)
            {
                m_itCurrent = it;
                break;
            }
        }
        m_pPlayNext = NULL;
    }

	if (m_itCurrent == m_Clips.end())
	{
		if (!m_bLoop)
		{
			// no more clips, so allow EOS to propagate through
			// render graph. We will receive EC_COMPLETE eventually.
			m_pController->NoMoreSegments();

            // disconnect graphs -- but we cannot do that until NoMoreSegments has 
            // sent the EndOfStream through the bridge
	        m_pController->BridgeGraphs(NULL, NULL);
			return;
		}

        // disconnect graphs before seeking the source graph
        m_pController->BridgeGraphs(NULL, NULL);
        m_itCurrent = m_Clips.begin();
		m_tStartPosition = 0;

        // looping to same graph? rewind now
        if (m_itCurrent == itOld)
        {
            m_itCurrent->Prime(0);
        }
	}

    // has this graph been rewound?
    m_itCurrent->Prime();

    // now we are about to connect it, so as soon as data is delivered out of the graph it
    // is no longer primed for re-use (it will need a new seek before using again)
    m_itCurrent->InUse();

	// reconnect
	m_pController->BridgeGraphs(m_itCurrent->SinkFilter(), m_pRenderGraphSourceFilter);

    if (m_bLoop && (m_itCurrent != itOld))
    {
        // need to rewind clip for next time round
        // unless we are reusing it immediately (ie looping single clip)
        // it's better to do this when not in use
        itOld->Prime(0);
    }
}

void 
ClipPlayer::OnEvent()
{
	IMediaEventExPtr pME = m_pRenderGraph;
	if (pME != NULL)
	{
		long lEvent, l1, l2;
		while (pME->GetEvent(&lEvent, &l1, &l2, 0) == S_OK)
		{
			switch(lEvent)
			{
			case EC_COMPLETE:
				Stop();
				break;
            case EC_VIDEO_SIZE_CHANGED:
                ResizeWindowToVideo();
                break;
			}
			pME->FreeEventParams(lEvent, l1, l2);
		}
	}
}

void
ClipPlayer::ResizeWindowToVideo()
{
    IVideoWindowPtr pVW = m_pRenderGraph;
    IBasicVideoPtr pBV = m_pRenderGraph;
    if ((pVW != NULL) && (pBV != NULL))
    {
        // size of new video
        long cx, cy;
        pBV->GetVideoSize(&cx, &cy);
        RECT rcVideo = {0, 0, cx, cy};

        // adjust from client size to window size
        long style;
        pVW->get_WindowStyle(&style);
        AdjustWindowRect(&rcVideo, style, false);

        // get current window top/left
        long cxWindow, cyWindow;
        RECT rc;
        pVW->GetWindowPosition(&rc.left, &rc.top, &cxWindow, &cyWindow);
        
        // reposition video window with old top/left position and new size
        pVW->SetWindowPosition(rc.left, rc.top, rcVideo.right - rcVideo.left, rcVideo.bottom - rcVideo.top);
    }
}


REFERENCE_TIME 
ClipPlayer::CurrentPosition()
{
	if ((m_itCurrent == m_Clips.end()) || !m_bActive)
	{
		return m_tStartPosition;
	}
	double dTime = 0;
	m_pController->GetSegmentTime(&dTime);
	REFERENCE_TIME tNow = REFERENCE_TIME(dTime * 10000000);

	// this is relative to the start position within this particular clip.
	// Did we start at the beginning of this clip?
	if (m_tStartPosition > m_itCurrent->GetStartPosition())
	{
		// no, we started some distance into the clip
		tNow += (m_tStartPosition - m_itCurrent->GetStartPosition());
	}
	// offset from start of this clip to start of entire sequence
	tNow += m_itCurrent->GetStartPosition();

	if ((tNow < 0) && m_bLoop)
	{
		// current time is near end of previous loop
		list<ClipEntry>::iterator it = m_Clips.end();
		it--;
		tNow += it->GetStartPosition() + it->Duration();
	}
	return tNow;
}

HRESULT
ClipPlayer::SeekSourceGraphs()
{
    // disconnect first
    m_pController->BridgeGraphs(NULL, NULL);

    // now seek all graphs and activate them

    REFERENCE_TIME tStartThis = 0;
    HRESULT hr = E_INVALIDARG;
	m_itCurrent = m_Clips.begin();
    list<ClipEntry>::iterator it;
    for (it = m_Clips.begin(); it != m_Clips.end(); it++)
    {
        // all clips at or after start: seek to correct position
        if ((tStartThis + it->Duration()) <= m_tStartPosition)
        {
            // whole clip is before beginning point - not required
			// but must rewind if we are in loop mode
			if (m_bLoop)
			{
                it->Prime(0);
			}
        } else 
        {
            // included in clip -- set start position
            REFERENCE_TIME tOffset = 0;
            if (m_tStartPosition >= tStartThis)
            {
                // starts someway in (this is first clip in sequence)
                m_itCurrent = it;
                tOffset = m_tStartPosition - tStartThis;
            }
            it->Prime(tOffset);
        }

        tStartThis += it->Duration();
    }

    // bridge the correct graph
    // note: once connected, this graph is no longer primed (it has output some data
    // and will need rewinding before re-use)
    m_itCurrent->InUse();
    hr = m_pController->BridgeGraphs(m_itCurrent->SinkFilter(), m_pRenderGraphSourceFilter);

    return hr;
}

HRESULT 
ClipPlayer::SetPosition(REFERENCE_TIME tStart)
{
	m_tStartPosition = tStart;
	IMediaControlPtr pMC = m_pRenderGraph;
	if (pMC == NULL)
	{
		return E_FAIL;
	}
	FILTER_STATE fs;
	pMC->GetState(0, (OAFilterState*)&fs);
	bool bRunning = false;
	if (fs == State_Running)
	{
		pMC->Pause();
		bRunning = true;
	} else if (fs == State_Stopped)
	{
        // on going active, we'll seek all graphs
        return S_OK;
    }

    HRESULT hr = SeekSourceGraphs();

	if (bRunning)
	{
		pMC->Run();
	}
	return hr;
}

ClipEntry* 
ClipPlayer::CurrentClip()
{
	if (m_itCurrent == m_Clips.end())
	{
		return NULL;
	}
	return &(*m_itCurrent);
}

void 
ClipPlayer::UpdateDuration()
{
	// loop through all clips setting position and calculating duration
	m_tDuration = 0;
	list<ClipEntry>::iterator it;
	for (it = m_Clips.begin(); it != m_Clips.end(); it++)
	{
		ClipEntry* pClip = &(*it);
		pClip->SetStartPosition(m_tDuration);
		m_tDuration += pClip->Duration();
	}
}


HRESULT 
ClipPlayer::SetClipLimits(ClipEntry* pClip, REFERENCE_TIME tStart, REFERENCE_TIME tEnd)
{
    REFERENCE_TIME tDur = pClip->Duration();
    HRESULT hr = pClip->SetLimits(tStart, tEnd);
    if (FAILED(hr) || (tDur == pClip->Duration()))
    {
        return hr;
    }

    // this is called from the same message loop as the end-of-segment processing, so it's safe to 
    // access the current segment
    if (pClip == CurrentClip())
    {
        // send just the stop time to the graph
        // in the hope that it is not too late
        hr = pClip->SetStopTime();
    }

    // clip duration has changed: update start position of
    // all subsequent clips (for current position slider UI)

	m_tDuration = 0;
    bool bFound = false;
	list<ClipEntry>::iterator it;
	for (it = m_Clips.begin(); it != m_Clips.end(); it++)
	{
		ClipEntry* pThis = &(*it);
        if (pThis == pClip)
        {
            bFound = true;
            m_tDuration = pClip->GetStartPosition() + pClip->Duration();
        } else if (bFound)
        {
            // following clip: adjust
            pClip->SetStartPosition(m_tDuration);
            m_tDuration += pClip->Duration();
        }
    }
    return S_OK;
}

void
ClipPlayer::SetLoop(bool bLoop)
{
	m_bLoop = bLoop;
}

FILTER_STATE 
ClipPlayer::GetState()
{
	FILTER_STATE state = State_Stopped;
	IMediaControlPtr pMC = m_pRenderGraph;
	if (pMC != NULL)
	{
		pMC->GetState(0, (OAFilterState*)&state);
	}
	return state;
}

    
ClipEntry* 
ClipPlayer::GetClipByIndex(long idx)
{
	// loop through all clips setting position and calculating duration
	list<ClipEntry>::iterator it;
	for (it = m_Clips.begin(); it != m_Clips.end(); it++)
	{
		ClipEntry* pClip = &(*it);
        if (idx-- == 0)
        {
            return pClip;
        }
	}
    return NULL;
}


void 
ClipPlayer::PlayNext(ClipEntry* pClip)
{
    // synchronised by windows message queue
    m_pPlayNext = pClip;
}


// ---------- ClipEntry implementation -----------------

ClipEntry::ClipEntry()
: m_tOffset(0),
  m_tStart(0),
  m_tStop(0),
  m_bPrimed(false)
{
}

ClipEntry::ClipEntry(const ClipEntry& r)
{
	m_tOffset = r.m_tOffset;
	m_pGraph = r.m_pGraph;
	m_pSinkFilter = r.m_pSinkFilter;
	m_strName = r.m_strName;
	m_tStart = r.m_tStart;
	m_tStop = r.m_tStop;
    m_bPrimed = r.m_bPrimed;
}

ClipEntry::~ClipEntry()
{
	IMediaControlPtr pMC = m_pGraph;
	if (pMC != NULL)
	{
		pMC->Stop();
	}
}

const ClipEntry& 
ClipEntry::operator=(const ClipEntry& r)
{
	m_tOffset = r.m_tOffset;
	m_pGraph = r.m_pGraph;
	m_pSinkFilter = r.m_pSinkFilter;
    m_strName = r.m_strName;
	m_tStart = r.m_tStart;
	m_tStop = r.m_tStop;
    m_bPrimed = r.m_bPrimed;
	return *this;
}

HRESULT 
ClipEntry::Create(IGMFBridgeController* pController, const char* path)
{
    m_bPrimed = false;

	m_pGraph.CreateInstance(CLSID_FilterGraph);
	_bstr_t bstr = path;
	HRESULT hr = pController->CreateSourceGraph(bstr, m_pGraph, &m_pSinkFilter);

	const char* p = strrchr(path, '\\');
	p++;
	m_strName = p;

	if (FAILED(hr))
	{
		m_pGraph.Release();
	}

	return hr;
}

HRESULT
ClipEntry::SetLimits(REFERENCE_TIME tStart, REFERENCE_TIME tStop)
{
	m_tStart = tStart;
	m_tStop = tStop;
    m_bPrimed = false;
	return S_OK;
}

REFERENCE_TIME 
ClipEntry::Duration()
{
	if (m_tStop == 0)
	{
		REFERENCE_TIME tDur = 0;
		IMediaSeekingPtr pMS = m_pGraph;
        HRESULT hr = pMS->GetDuration(&tDur);
        if (FAILED(hr))
        {
            return 0;
        }
        
		return tDur - m_tStart;
	}
	return m_tStop - m_tStart;
}

    
REFERENCE_TIME 
ClipEntry::NativeDuration()
{
    // native duration with no limits applied
	REFERENCE_TIME tDur = 0;
	IMediaSeekingPtr pMS = m_pGraph;
    HRESULT hr = pMS->GetDuration(&tDur);
	return tDur;
}

REFERENCE_TIME 
ClipEntry::CurrentPosition()
{
	// stream time in graph
	IMediaSeekingPtr pMS = m_pGraph;
	REFERENCE_TIME tNow = 0;
	pMS->GetCurrentPosition(&tNow);
	return tNow + m_tStart;
}


HRESULT 
ClipEntry::SetPosition(REFERENCE_TIME tStart)
{
	IMediaSeekingPtr pSeek = m_pGraph;
	HRESULT hr = E_NOINTERFACE;
	if (pSeek != NULL)
	{
		// input time is relative to clip start -- add on offset
		// from start of media
		tStart += m_tStart;
		if (m_tStop == 0)
		{
			hr = pSeek->SetPositions(
							&tStart,
							AM_SEEKING_AbsolutePositioning, 
							NULL,
							AM_SEEKING_NoPositioning);
		} else {
			hr = pSeek->SetPositions(
							&tStart,
							AM_SEEKING_AbsolutePositioning, 
							&m_tStop,
							AM_SEEKING_AbsolutePositioning);
		}
	}
	return hr;
}

HRESULT
ClipEntry::SetStopTime()
{
    // for limits change of active clip: pass the stop time to graph

	IMediaSeekingPtr pSeek = m_pGraph;
	HRESULT hr = E_NOINTERFACE;
	if (pSeek != NULL)
	{
		hr = pSeek->SetPositions(
						NULL,
						AM_SEEKING_NoPositioning,
						&m_tStop,
						AM_SEEKING_AbsolutePositioning);
    }
    return hr;
}


void 
ClipEntry::Prime(REFERENCE_TIME tStart)
{
    if (!m_bPrimed)
    {
        SetPosition(tStart);

        IMediaControlPtr pMC = Graph();
        OAFilterState state;
        pMC->GetState(0, &state);
        if (state != State_Running)
        {
            pMC->Run();
        }
        m_bPrimed = true;
    }
}

void 
ClipEntry::Stop()
{
    IMediaControlPtr pMC = Graph();
    pMC->Stop();
    m_bPrimed = false;
}


