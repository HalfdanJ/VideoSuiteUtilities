//------------------------------------------------------------------------------
// CustomAllocator.cpp
//
// Desc: DirectShow sample code - Custom Allocator
//		 Based largely upon the DirectShow SDK memory allocator base class
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "CustomAllocator.h"

//------------------------------------------------------------------------------
static const long SAMPLE_LIST_SIZE = 32;

static const int DBG_MEM = 3;
static const int DBG_ERR = 1;
static const int DBG_WRN = 2;
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown * WINAPI CCustomAllocator::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown *pUnkRet = new CCustomAllocator(NAME("CCustomAllocator"), pUnk, phr);
    return pUnkRet;
}

//------------------------------------------------------------------------------
// Constructor
//   Constructor overrides the default settings for the free list to request
//   that it be alertable (ie the list can be cast to a handle which can be
//   passed to WaitForSingleObject). Both of the allocator lists also ask for
//   object locking, the all list matches the object default settings but I
//   have included them here just so it is obvious what kind of list it is
CCustomAllocator::CCustomAllocator(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, BOOL bEvent, BOOL fEnableReleaseCallback)
	: CBaseAllocator(pName, pUnk, phr)
    , m_pBuffer(NULL)
	, m_lFree("Custom allocator free list", SAMPLE_LIST_SIZE)
	, m_lDeliver("Decklink push source custom allocator deliver list", SAMPLE_LIST_SIZE)
	, m_lBuffers("Decklink push source custom allocator buffer list", SAMPLE_LIST_SIZE)
	, m_hSemFree(NULL)
	, m_lWaitingFree(0)
{
	m_hSemFree = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	if (NULL == m_hSemFree)
	{
		*phr = E_OUTOFMEMORY;
		return;
	}
}

//------------------------------------------------------------------------------
// Destructor
//
CCustomAllocator::~CCustomAllocator(void)
{
	Decommit();
	ReallyFree();

	if (m_hSemFree)
	{
		EXECUTE_ASSERT(CloseHandle(m_hSemFree));
	}
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CCustomAllocator::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IDecklinkPushSource)
	{
		return GetInterface((IDecklinkPushSource*)this, ppv);
	}

	if (riid == IID_IDecklinkPushSource2)
	{
		return GetInterface((IDecklinkPushSource2*)this, ppv);
	}

	if (riid == IID_IDecklinkPushSource3)
	{
		return GetInterface((IDecklinkPushSource3*)this, ppv);
	}

	return CBaseAllocator::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------------
// SetProperties
//
STDMETHODIMP CCustomAllocator::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pActual, E_POINTER);
    ValidateReadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

    ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

    ASSERT(pRequest->cbBuffer > 0);

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

    /*  Check the alignment request is a power of 2 */
    if ((-pRequest->cbAlign & pRequest->cbAlign) != pRequest->cbAlign)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment requested 0x%x not a power of 2!"), pRequest->cbAlign));
    }
    /*  Check the alignment requested */
    if (pRequest->cbAlign == 0 || (SysInfo.dwAllocationGranularity & (pRequest->cbAlign - 1)) != 0)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid alignment 0x%x requested - granularity = 0x%x"), pRequest->cbAlign, SysInfo.dwAllocationGranularity));
        return VFW_E_BADALIGN;
    }

    /* Can't do this if already committed, there is an argument that says we
       should not reject the SetProperties call if there are buffers still
       active. However this is called by the source filter, which is the same
       person who is holding the samples. Therefore it is not unreasonable
       for them to free all their samples before changing the requirements */

    if (m_bCommitted == TRUE)
    {
        return VFW_E_ALREADY_COMMITTED;
    }

    /* Must be no outstanding buffers */

    if (m_lFree.GetCount() < m_lAllocated)
    {
        return VFW_E_BUFFERS_OUTSTANDING;
    }

    /* There isn't any real need to check the parameters as they
       will just be rejected when the user finally calls Commit */

    // round length up to alignment - remember that prefix is included in
    // the alignment
    LONG lSize = pRequest->cbBuffer + pRequest->cbPrefix;
    LONG lRemainder = lSize % pRequest->cbAlign;
    if (lRemainder != 0)
    {
        lSize = lSize - lRemainder + pRequest->cbAlign;
    }

    pActual->cbBuffer = m_lSize = (lSize - pRequest->cbPrefix);
    pActual->cBuffers = m_lCount = pRequest->cBuffers;
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    m_bChanged = TRUE;
    return NOERROR;
}

//------------------------------------------------------------------------------
// GetBuffer
// get container for a sample. Blocking, synchronous call to get the
// next free buffer (as represented by an IMediaSample interface).
// on return, the time etc properties will be invalid and in this case
// the buffer pointer and size will be set when the app returns a buffer.
HRESULT CCustomAllocator::GetBuffer(IMediaSample** ppBuffer, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(pStartTime);
    UNREFERENCED_PARAMETER(pEndTime);
    UNREFERENCED_PARAMETER(dwFlags);
    CMediaSample* pSample;

    *ppBuffer = NULL;
    for (;;)
    {
        {  // scope for lock
            CAutoLock cObjectLock(this);

            /* Check we are committed */
            if (!m_bCommitted)
            {
                return VFW_E_NOT_COMMITTED;
            }
            pSample = (CMediaSample*)m_lDeliver.RemoveHead();
            if (pSample == NULL)
            {
                SetWaiting();
            }
        }

        /* If we didn't get a sample then wait for the list to signal */

        if (pSample)
        {
            break;
        }
        if (dwFlags & AM_GBF_NOWAIT)
        {
            return VFW_E_TIMEOUT;
        }
        ASSERT(m_hSem != NULL);
        WaitForSingleObject(m_hSem, INFINITE);
    }

    /* Addref the buffer up to one. On release
       back to zero instead of being deleted, it will requeue itself by
       calling the ReleaseBuffer member function. NOTE the owner of a
       media sample must always be derived from CBaseAllocator */

    ASSERT(pSample->m_cRef == 0);
    pSample->m_cRef = 1;
    *ppBuffer = pSample;

	DbgLog((LOG_TRACE, DBG_MEM, TEXT("CCustomAllocator::GetBuffer(): m_lFree: %d  m_lDeliver: %d"), m_lFree.GetCount(), m_lDeliver.GetCount()));

    return NOERROR;
}

//------------------------------------------------------------------------------
// ReleaseBuffer
// Final release of a CMediaSample will call this
STDMETHODIMP CCustomAllocator::ReleaseBuffer(IMediaSample* pSample)
{
	CheckPointer(pSample, E_POINTER);
	ValidateReadPtr(pSample, sizeof(IMediaSample));

	BOOL bRelease = FALSE;
	{
		CAutoLock cal(this);

		BYTE* pBuffer = NULL;
		pSample->GetPointer(&pBuffer);
		ASSERT(pBuffer);
		m_lBuffers.AddTail(pBuffer);

		/* Put back on the free list */

		m_lFree.AddTail((CMediaSample*)pSample);
		if (m_lWaitingFree != 0)
		{
			NotifySampleFree();
		}

		DbgLog((LOG_TRACE, DBG_MEM, TEXT("CCustomAllocator::ReleaseBuffer(): m_lFree: %d  m_lDeliver: %d"), m_lFree.GetCount(), m_lDeliver.GetCount()));

		// if there is a pending Decommit, then we need to complete it by
		// calling Free() when the last buffer is placed on the free list

		LONG l1 = m_lFree.GetCount();
		if (m_bDecommitInProgress && (l1 == m_lAllocated))
		{
			Free();
			m_bDecommitInProgress = FALSE;
			bRelease = TRUE;
		}
	}

	if (m_pNotify)
	{
		ASSERT(m_fEnableReleaseCallback);

		//
		// Note that this is not synchronized with setting up a notification
		// method.
		//
		m_pNotify->NotifyRelease();
	}

	/* For each buffer there is one AddRef, made in GetBuffer and released
	here. This may cause the allocator and all samples to be deleted */

	if (bRelease)
	{
		Release();
	}
	return NOERROR;
}

//------------------------------------------------------------------------------
// NotifySampleFree
//
void CCustomAllocator::NotifySampleFree()
{
	if (m_lWaitingFree != 0)
	{
		ASSERT(m_hSemFree);
		ReleaseSemaphore(m_hSemFree, m_lWaitingFree, 0);
		m_lWaitingFree = 0;
	}
}

//------------------------------------------------------------------------------
// GetFreeCount
//
STDMETHODIMP CCustomAllocator::GetFreeCount(LONG* plBuffersFree)
{
    ASSERT(m_fEnableReleaseCallback);
    CAutoLock cObjectLock(this);
    *plBuffersFree = m_lCount - m_lAllocated + m_lFree.GetCount();
    return NOERROR;
}

//------------------------------------------------------------------------------
// Decommit
//
STDMETHODIMP CCustomAllocator::Decommit(void)
{
    BOOL bRelease = FALSE;
    {
        /* Check we are not already decommitted */
        CAutoLock cObjectLock(this);
        if (m_bCommitted == FALSE)
        {
            if (m_bDecommitInProgress == FALSE)
            {
                return NOERROR;
            }
        }

        /* No more GetBuffer calls will succeed */
        m_bCommitted = FALSE;

        // are any buffers outstanding?
        if (m_lFree.GetCount() < m_lAllocated)
        {
            // please complete the decommit when last buffer is freed
            m_bDecommitInProgress = TRUE;
        }
        else
        {
            m_bDecommitInProgress = FALSE;

            // need to complete the decommit here as there are no
            // outstanding buffers

            Free();
            bRelease = TRUE;
        }

        // Tell anyone waiting that they can go now so we can
        // reject their call
        NotifySample();
        NotifySampleFree();
    }

    if (bRelease)
    {
        Release();
    }
    return NOERROR;
}

//------------------------------------------------------------------------------
// Alloc
// Base definition of allocation which checks we are ok to go ahead and do
// the full allocation. We return S_FALSE if the requirements are the same
HRESULT CCustomAllocator::Alloc(void)
{
	CAutoLock lck(this);

	/* Check he has called SetProperties */
	/**************************************************/
	// override the call to the base class to check our
	// internal free list
    HRESULT hr = NOERROR;
    /* Error if he hasn't set the size yet */
    if (m_lCount <= 0 || m_lSize <= 0 || m_lAlignment <= 0)
    {
        hr = VFW_E_SIZENOTSET;
    }

    /* should never get here while buffers outstanding */
    ASSERT(m_lFree.GetCount() == m_lAllocated);

    /* If the requirements haven't changed then don't reallocate */
    if (m_bChanged == FALSE)
    {
        hr = S_FALSE;
    }
	/**************************************************/

	if (FAILED(hr))
	{
		return hr;
	}

    // If the requirements haven't changed then don't reallocate
    if (hr == S_FALSE)
    {
		ASSERT(m_pBuffer);
		return NOERROR;
    }
    ASSERT(hr == S_OK);

    // Free the old resources
    if (m_pBuffer)
        ReallyFree();

    // Compute the aligned size
    LONG lAlignedSize = m_lSize + m_lPrefix;
    if (m_lAlignment > 1)
    {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0)
            lAlignedSize += (m_lAlignment - lRemainder);
    }

    // Create the contiguous memory block for the samples
	// making sure it's properly aligned

    ASSERT(lAlignedSize % m_lAlignment == 0);

	m_pBuffer = (PBYTE)VirtualAlloc(NULL, m_lCount * lAlignedSize, MEM_COMMIT, PAGE_READWRITE);

	if (m_pBuffer == NULL)
	{
		return E_OUTOFMEMORY;
	}

	LPBYTE pNext = m_pBuffer;
	CMediaSample* pSample;

	ASSERT(m_lAllocated == 0);

	// Create the new samples - we have allocated m_lSize bytes for each sample
	// plus m_lPrefix bytes per sample as a prefix. We set the pointer to
	// the memory after the prefix - so that GetPointer() will return a pointer
	// to m_lSize bytes.
	for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize)
	{
		pSample = new CMediaSample(NAME("Default memory media sample"), this, &hr, NULL, 0);
		ASSERT(SUCCEEDED(hr));
		if (pSample == NULL)
		{
			return E_OUTOFMEMORY;
		}

		// This CANNOT fail
		m_lFree.AddTail(pSample);

		m_lBuffers.AddTail(pNext + m_lPrefix);
	}

    m_bChanged = FALSE;
    return hr;
}

//------------------------------------------------------------------------------
// Free
// in our case, we keep the memory until we are deleted, so
// we do nothing here. The memory is deleted in the destructor by
// calling ReallyFree()
void CCustomAllocator::Free(void)
{
	return;
}

//------------------------------------------------------------------------------
// ReallyFree
// called from the destructor (and from Alloc if changing size/count) to
// actually free up the memory
void CCustomAllocator::ReallyFree(void)
{
    /* Should never be deleting this unless all buffers are freed */
    ASSERT(m_lAllocated == m_lFree.GetCount());
    ASSERT(0 == m_lDeliver.GetCount());

    /* Free up all the CMediaSamples */
    CMediaSample* pSample = m_lFree.RemoveHead();
    while (pSample)
    {
		delete pSample;
		pSample = m_lFree.RemoveHead();
    }

	m_lBuffers.RemoveAll();

    m_lAllocated = 0;

	// free the block of buffer memory
	if (m_pBuffer)
	{
		EXECUTE_ASSERT(VirtualFree(m_pBuffer, 0, MEM_RELEASE));
		m_pBuffer = NULL;
	}

	return;
}

//------------------------------------------------------------------------------
// IDecklinkPushSource interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GetFrameBuffer
// Provide the caller with a free frame buffer
STDMETHODIMP CCustomAllocator::GetFrameBuffer(unsigned char** ppBuffer, unsigned long* pSize)
{
	HRESULT hr = S_OK;

	if (ppBuffer && pSize)
	{
		CAutoLock lck(this);
		unsigned char* pBuffer = m_lBuffers.RemoveHead();
		if (pBuffer)
		{
			*ppBuffer = pBuffer;
			*pSize = m_lSize;
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//------------------------------------------------------------------------------
// Deliver
// Attach the supplied buffer to a free media sample.  Add media sample to delivery
// queue so that the streaming thread can deliver the sample downstream.  If there
// are no free samples, all have been delivered downstream, block until one is returned
// to the free list.  If the streaming thread is blocked, waiting for a sample to
// become available on the delivery queue, signal the thread when a new sample is added.
STDMETHODIMP CCustomAllocator::Deliver(unsigned char* pBuffer)
{
	HRESULT hr = S_OK;

	if (pBuffer)
	{
		CMediaSample* pSample = NULL;

	    for (;;)
		{
			{  // scope for lock
				CAutoLock cObjectLock(this);

	            /* Check we are committed */
		        if (!m_bCommitted)
			    {
				    return VFW_E_NOT_COMMITTED;
				}
				pSample = (CMediaSample*)m_lFree.RemoveHead();
				if (pSample == NULL)
				{
					SetWaitingFree();
	            }
		    }

		    if (pSample)
			{
				hr = pSample->SetPointer(pBuffer, m_lSize);
				if (SUCCEEDED(hr))
				{
					CAutoLock cObjectLock(this);
					m_lDeliver.AddTail(pSample);
					NotifySample();
					DbgLog((LOG_TRACE, DBG_MEM, TEXT("CCustomAllocator::Deliver(): m_lFree: %d  m_lDeliver: %d"), m_lFree.GetCount(), m_lDeliver.GetCount()));
				}
				break;
	        }

	        /* If we didn't get a sample then wait for the list to signal */
	        ASSERT(m_hSemFree);
		    WaitForSingleObject(m_hSemFree, INFINITE);
		}
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}

//------------------------------------------------------------------------------
// IDecklinkPushSource2 interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GetFrameBuffer
// Provide the caller with a free media sample.  If there are no free samples,
// all have been delivered downstream, block until one is returned to the free list.  
STDMETHODIMP CCustomAllocator::GetFrameBuffer(IMediaSample** ppSample)
{
	HRESULT hr = S_OK;

	if (ppSample)
	{
		CAutoLock lck(this);
		unsigned char* pBuffer = m_lBuffers.RemoveHead();	// retrieve a frame buffer
		if (pBuffer)
		{
			// attempt to retrieve a free media sample
			CMediaSample* pSample = NULL;
			for (;;)
			{
				{  // scope for lock
					CAutoLock cObjectLock(this);

					/* Check we are committed */
					if (!m_bCommitted)
					{
						return VFW_E_NOT_COMMITTED;
					}
					pSample = (CMediaSample*)m_lFree.RemoveHead();
					if (pSample == NULL)
					{
						SetWaitingFree();
					}
				}

				if (pSample)
				{
					// return the sample to the caller
					hr = pSample->SetPointer(pBuffer, m_lSize);
					*ppSample = pSample;
					break;
				}

				/* If we didn't get a sample then wait for the list to signal */
				ASSERT(m_hSemFree);
				WaitForSingleObject(m_hSemFree, INFINITE);
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//------------------------------------------------------------------------------
// Deliver
// Add the supplied media sample to delivery queue so that the streaming thread can
// deliver the sample downstream.If the streaming thread is blocked, waiting for a sample to
// become available on the delivery queue, signal the thread when a new sample is added.
STDMETHODIMP CCustomAllocator::Deliver(IMediaSample* pSample)
{
	HRESULT hr = S_OK;

	if (pSample)
	{
		CAutoLock cObjectLock(this);
		m_lDeliver.AddTail((CMediaSample*)pSample);
		NotifySample();
		DbgLog((LOG_TRACE, DBG_MEM, TEXT("CCustomAllocator::Deliver(): m_lFree: %d  m_lDeliver: %d"), m_lFree.GetCount(), m_lDeliver.GetCount()));
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}

//------------------------------------------------------------------------------
// IDecklinkPushSource3 interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GetFrameBuffer
// Provide the caller with a free media sample.  If there are no free samples,
// all have been delivered downstream, block until one is returned to the free list.  
STDMETHODIMP CCustomAllocator::GetFrameBuffer(IUnknown** ppSample)
{
	return GetFrameBuffer(reinterpret_cast<IMediaSample**>(ppSample));
}

//------------------------------------------------------------------------------
// Deliver
// Add the supplied media sample to delivery queue so that the streaming thread can
// deliver the sample downstream.If the streaming thread is blocked, waiting for a sample to
// become available on the delivery queue, signal the thread when a new sample is added.
STDMETHODIMP CCustomAllocator::Deliver(IUnknown* pSample)
{
	return Deliver(reinterpret_cast<IMediaSample*>(pSample));
}
