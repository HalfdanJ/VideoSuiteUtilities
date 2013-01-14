//-----------------------------------------------------------------------------
// BaseEDLReader.cpp
//
// Desc: Abstract base class for reading EDLs.
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "BaseEDLReader.h"

//-----------------------------------------------------------------------------
// Construction
//
CBaseEDLReader::CBaseEDLReader(const CString Filename, HRESULT* phr)
	: m_hFile(NULL)
	, m_pFile(NULL)
	, m_cbFile(0)
{
	HRESULT hr = S_OK;
	m_hFile = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != m_hFile)
	{
		m_cbFile = SetFilePointer(m_hFile, 0, NULL, FILE_END);
		if (INVALID_SET_FILE_POINTER != m_cbFile)
		{
			if (INVALID_SET_FILE_POINTER != SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN))
			{
				try
				{
					m_pFile = new char [m_cbFile];
					if (m_pFile)
					{
						DWORD dwNumberOfBytesRead;
						if (0 == ReadFile(m_hFile, m_pFile, m_cbFile, &dwNumberOfBytesRead, NULL))
						{
							hr = AmHresultFromWin32(GetLastError());
						}
					}
				}
				catch (std::bad_alloc)
				{
					hr = E_OUTOFMEMORY;
				}
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = AmHresultFromWin32(GetLastError());
	}
	if (FAILED(hr) && phr)
	{
		*phr = hr;
	}
}

//-----------------------------------------------------------------------------
// Destruction
//
CBaseEDLReader::~CBaseEDLReader(void)
{
	SAFE_DELETE(m_pFile);
	CloseHandle(m_hFile);
}
