// BasePlayList.cpp : implementation file
//

#include "stdafx.h"
#include "BasePlayList.h"

#include <qedit.h>

//-----------------------------------------------------------------------------
// CBasePlayList
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Construction
//
IMPLEMENT_DYNAMIC(CBasePlayList, CListCtrl)
CBasePlayList::CBasePlayList()
	: m_bInit(true)
{
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CBasePlayList, CListCtrl)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnCreate
//
int CBasePlayList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = CListCtrl::OnCreate(lpCreateStruct);
	DragAcceptFiles(TRUE);
	return ret;
}

//-----------------------------------------------------------------------------
// OnDropFiles
// With this control enabled to be a drop target, extract the file(s) and insert
// them into the file list.
void CBasePlayList::OnDropFiles(HDROP hDropInfo)
{
	// determine the number of files being dropped
	UINT cFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (UINT File=0; File<cFiles; ++File)
	{
		UINT cchFile = DragQueryFile(hDropInfo, File, NULL, 0) + 1;	// determine the size of the buffer incl. null terminator
		if (1 < cchFile)
		{
			try
			{
				TCHAR* pFilename = new TCHAR [cchFile];
				ZeroMemory(pFilename, cchFile);
				if (DragQueryFile(hDropInfo, File, pFilename, cchFile))
				{
					AddItem(new CBaseClip(basic_string<TCHAR>(pFilename)));
				}
				delete [] pFilename;
			}
			catch (std::bad_alloc)
			{
			}
		}
	}

	return CListCtrl::OnDropFiles(hDropInfo);
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// AddItem
// Insert the clip into the playlist.
int CBasePlayList::AddItem(CBaseClip* pClip)
{
	int item = AddItem(GetItemCount(), pClip);
	SetSelectedItem(item);
	return item;
}

//-----------------------------------------------------------------------------
// AddItem
// Insert the clip into the playlist.
int CBasePlayList::AddItem(int Item, CBaseClip* pClip)
{
	CComPtr<IMediaDet> pIMediaDet;	// for extracting parameters from media files
	// create a media detector object to determine the media types of the playback files
	HRESULT hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, reinterpret_cast<void**>(&pIMediaDet));
	if (pIMediaDet)
	{
		// attempt to extract clip frame rate and duration
		if (SUCCEEDED(pIMediaDet->put_Filename(CT2W(pClip->FilePath().c_str()))))
		{
			double Duration;
			if (SUCCEEDED(pIMediaDet->get_StreamLength(&Duration)))
			{
				double FrameRate;
				if (SUCCEEDED(pIMediaDet->get_FrameRate(&FrameRate)))
				{
					pClip->SetFrameRate(FrameRate);

					CTimecode tcStart((WORD)FrameRate, true, (DWORD)0);
					CTimecode tcDuration((WORD)FrameRate, true, (DWORD)(Duration * FrameRate));
					pClip->SetStart(tcStart);	// set the starting timecode of the clip
					pClip->SetEnd(tcDuration - 1);	// set the end timecode of the clip
					pClip->SetDuration(tcDuration);	// set the clip duration
					pClip->SetInpoint(tcStart);	// for now set the inpoint to be the same as the clip start
					pClip->SetOutpoint(tcDuration - 1);	// for now set the ouput to be the same as the clip end
				}
			}
		}
	}

	if (m_bInit)
	{
		// initialise the image list
		if (m_ImageList.Create(16, 16, ILC_COLOR32, 0, 1))
		{
			SetImageList(&m_ImageList, LVSIL_SMALL);
			m_bInit = false;
		}
	}

	// insert the clip name and icon for the item
	LVITEM lvi = {0};
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = Item;
	lvi.iSubItem = 0;
	basic_string<TCHAR> test = pClip->Filename().c_str();
	lvi.pszText = (LPTSTR)test.c_str();
	lvi.iImage = GetIconIndex(pClip);
	int item = InsertItem(&lvi);
	SetItemData(item, reinterpret_cast<DWORD_PTR>(pClip));
	EnsureVisible(item, FALSE);

	CString buf;
	buf.Format(TEXT("%.2lf fps"),  pClip->FrameRate());
	SetItemText(item, 1, buf);

	buf = pClip->Start().TimecodeToString();
	SetItemText(item, 2, buf);
	SetItemText(item, 5, buf);

	buf = pClip->End().TimecodeToString();
	SetItemText(item, 3, buf);
	SetItemText(item, 6, buf);

	buf = pClip->Duration().TimecodeToString();
	SetItemText(item, 4, buf);

	buf = pClip->Path().c_str();
	SetItemText(item, 7, buf);

	return item;
}

//-----------------------------------------------------------------------------
// RemoveItem
// Remove the selected item from the playlist.
int CBasePlayList::RemoveItem(void)
{
	int sel = GetSelectedItem();
	if (-1 < sel)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(sel));
		if (pClip)
		{
			delete pClip;
		}

		DeleteItem(sel);
		
		int max = GetItemCount() - 1;
		if (sel < max)
		{
			SetSelectedItem(sel);
		}
		else
		{
			SetSelectedItem(max);
		}
	}
	return sel;
}

//-----------------------------------------------------------------------------
// MoveItemUp
// Move the selected item up in the playlist.
void CBasePlayList::MoveItemUp(void)
{
	int sel = GetSelectedItem();
	if (0 < sel)
	{
		// remove the preceding item and reinsert it after the selected item
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(sel - 1));
		if (pClip)
		{
			SetItemData(sel - 1, NULL);
			DeleteItem(sel - 1);
			AddItem(sel, pClip);
			SetItemState(sel - 1, LVIS_SELECTED | LVIS_DROPHILITED, LVIS_SELECTED | LVIS_DROPHILITED);
			SetItemState(sel, 0, LVIS_SELECTED | LVIS_DROPHILITED);
			EnsureVisible(sel - 1, FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
// MoveItemDown
// Move the selected item down in the playlist.
void CBasePlayList::MoveItemDown(void)
{
	int max = GetItemCount() - 1;
	int sel = GetSelectedItem();
	if ((max > sel) && (-1 != sel))
	{
		// remove this item and reinsert it after the following item
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(sel));
		if (pClip)
		{
			SetItemData(sel, NULL);
			DeleteItem(sel);
			AddItem(sel + 1, pClip);
			SetItemState(sel, 0, LVIS_SELECTED | LVIS_DROPHILITED);
			SetItemState(sel + 1, LVIS_SELECTED | LVIS_DROPHILITED, LVIS_SELECTED | LVIS_DROPHILITED);
			EnsureVisible(sel + 1, FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
// SetSelectedItem
//
BOOL CBasePlayList::SetSelectedItem(int item)
{
	EnsureVisible(item, FALSE);
	return SetItemState(item, LVIS_SELECTED | LVIS_DROPHILITED, LVIS_SELECTED | LVIS_DROPHILITED);
}

//-----------------------------------------------------------------------------
// GetSelectedItem
//
int CBasePlayList::GetSelectedItem(void)
{
	int sel = -1, cItems = GetItemCount();
	for (int item=0; item<cItems; ++item)
	{
		if (LVIS_SELECTED & GetItemState(item, LVIS_SELECTED))
		{
			sel = item;
			break;
		}
	}
	return sel;
}

//-----------------------------------------------------------------------------
// SetInpoint
// Set the inpoint of the specified clip and update the outpoint and UI.
void CBasePlayList::SetInpoint(int Item, CTimecode& Timecode)
{
	if (-1 < Item)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(Item));
		if (pClip)
		{
			pClip->SetInpoint(Timecode);
			CString buf = Timecode.TimecodeToString();
			if (SetItemText(Item, 5, buf))
			{
				CTimecode Outpoint = Timecode + pClip->Duration() - 1;
				pClip->SetOutpoint(Outpoint);
				buf = Outpoint.TimecodeToString();
				SetItemText(Item, 6, buf);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Inpoint
// Get the inpoint of the specified clip.
CTimecode CBasePlayList::Inpoint(int Item)
{
	CTimecode Timecode;
	if (-1 < Item)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(Item));
		if (pClip)
		{
			Timecode = pClip->Inpoint();
		}
	}
	return Timecode;
}

//-----------------------------------------------------------------------------
// SetOutpoint
// Set the outpoint of the specified clip and update the inpoint and UI.
void CBasePlayList::SetOutpoint(int Item, CTimecode& Timecode)
{
	if (-1 < Item)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(Item));
		if (pClip)
		{
			pClip->SetOutpoint(Timecode);
			CString buf = Timecode.TimecodeToString();
			if (SetItemText(Item, 6, buf))
			{
				CTimecode Inpoint = Timecode - pClip->Duration() + 1;
				pClip->SetInpoint(Inpoint);
				buf = Inpoint.TimecodeToString();
				SetItemText(Item, 5, buf);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Outpoint
// Get the outpoint of the specified clip.
CTimecode CBasePlayList::Outpoint(int Item)
{
	CTimecode Timecode;
	if (-1 < Item)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(GetItemData(Item));
		if (pClip)
		{
			Timecode = pClip->Outpoint();
		}
	}
	return Timecode;
}

//-----------------------------------------------------------------------------
// CBasePlayList message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// DrawItem
// Overridden to highlight the whole row when selected.
void CBasePlayList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect Rect(lpDrawItemStruct->rcItem);
	int Row = lpDrawItemStruct->itemID;
	CImageList* pImageList = GetImageList(LVSIL_SMALL);
	bool HasFocus = (GetFocus() == this);
	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = Row;
	lvi.iSubItem = 0;
	lvi.stateMask = -1;
	GetItem(&lvi);
	CRect VisibleRect;
	GetClientRect(&VisibleRect);
	VisibleRect.top = Rect.top;
	VisibleRect.bottom = Rect.bottom;

	if (lvi.state & LVIS_SELECTED)
	{
		if((lvi.state & LVIS_DROPHILITED) || HasFocus)
		{
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
			pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		}
	}
	else
	{
		pDC->SetBkColor((Row % 2) ? GetSysColor(COLOR_WINDOW) : RGB(255, 248, 240));
		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	}

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	CRect CellRect(Rect);
	CellRect.right = CellRect.left; //We'll be adding the column width to it in the Column loop:
	for (int Col=0; GetColumn(Col, &lvc); ++Col)
	{
		CellRect.left = CellRect.right; //Next cell
		CellRect.right += lvc.cx; //Keep track of the right of the cell
		//Erase the background (the last column may have overdrawn an icon to this column). This is probably faster and definitely easier than clipping an icon draw.
		pDC->FillSolidRect(CellRect, pDC->GetBkColor());

		if ((CellRect.right<VisibleRect.left)||(CellRect.left>VisibleRect.right))
		{
			continue; // Clipping: loop until we get to a visible cell.
		}

		lvi.iSubItem = Col;
		GetItem(&lvi);

		//Draw the Row's State Icon:
		if ((Col==0) && (CellRect.right>VisibleRect.left) && (lvi.state & LVIS_STATEIMAGEMASK) && GetImageList(LVSIL_STATE))
		{
			GetImageList(LVSIL_STATE)->Draw(pDC, ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1, CellRect.TopLeft(), ILD_TRANSPARENT);
		}

		if ((Col==0) && pImageList && (lvi.iImage != -1))
		{
			CellRect.left += pDC->GetTextExtent(" ", 1).cx << 1; // Text and Icons are spaced by an amount related to the width of a Space character
			pImageList->Draw(pDC, lvi.iImage, CellRect.TopLeft(), ILD_TRANSPARENT);
			CellRect.left += 16;
		}

		CString sText = GetItemText(Row, Col);
		if (sText.GetLength() == 0)
		{
			continue;
		}

		UINT nJustify = DT_LEFT; // Get the text justification
		switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;

			case LVCFMT_RIGHT:
				nJustify=DT_RIGHT;
				break;
		}
		
		pDC->DrawText(' ' + sText + ' ', -1, CellRect, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS);
	}

	if ((lvi.state & LVIS_FOCUSED) && HasFocus)
	{
		pDC->DrawFocusRect(Rect); // Draw focus rectangle if item has focus
	}
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// GetIconIndex
// Return the imagelist index of the icon for this clip adding a new icon if necessary.
int CBasePlayList::GetIconIndex(CBaseClip* pClip)
{
	int Index = -1;
	if (pClip)
	{
		map<basic_string<TCHAR>, int>::iterator it = m_IconExtensions.find(pClip->Extension());
		if (it != m_IconExtensions.end())
		{
			// the clip extension already exists in the imagelist so provide the index
			Index = it->second;
		}
		else
		{
			// the clip extension does not exist in the imagelist so add its icon to the imagelist
			LPMALLOC pIMalloc = NULL;
			if (SUCCEEDED(SHGetMalloc(&pIMalloc)))
			{
				IShellFolder* pIShellFolderDesktop = NULL;
				if (SUCCEEDED(SHGetDesktopFolder(&pIShellFolderDesktop)))
				{
					USES_CONVERSION;
					LPITEMIDLIST pidlFolder = NULL;

					if (SUCCEEDED(pIShellFolderDesktop->ParseDisplayName(NULL, NULL, T2OLE(pClip->Path().c_str()), NULL, &pidlFolder, NULL)) && pidlFolder)
					{
						// TODO: Use the last parameter of ParseDisplayName to check for folders?
						IShellFolder* pIShellFolderClip = NULL;
						HRESULT hr = pIShellFolderDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pIShellFolderClip));
						if (SUCCEEDED(hr) && pIShellFolderClip)
						{
							LPITEMIDLIST pidlFile = NULL;

							if (SUCCEEDED(pIShellFolderClip->ParseDisplayName(NULL, NULL, T2OLE(pClip->Filename().c_str()), NULL, &pidlFile, NULL)) && pidlFile)
							{
								IExtractIcon* pIExtractIcon = NULL;
								if (SUCCEEDED(pIShellFolderClip->GetUIObjectOf(NULL, 1, const_cast<LPCITEMIDLIST*>(&pidlFile), IID_IExtractIcon, NULL, reinterpret_cast<void**>(&pIExtractIcon))) && pIExtractIcon)
								{
									TCHAR IconFile[MAX_PATH] = {0};
									UINT Flags;
									if (SUCCEEDED(pIExtractIcon->GetIconLocation(0, IconFile, MAX_PATH, &Index, &Flags)))
									{
										HICON hIcon;
										if (SUCCEEDED(pIExtractIcon->Extract(IconFile, Index, NULL, &hIcon, (UINT)16 << 16)))
										{
											Index = m_ImageList.Add(hIcon);
											m_IconExtensions.insert(pair<basic_string<TCHAR>, int>(pClip->Extension(), Index));
										}
									}
									pIExtractIcon->Release();
								}
								pIMalloc->Free(pidlFile);
							}
							pIShellFolderClip->Release();
						}

						pIMalloc->Free(pidlFolder);
					}
					pIShellFolderDesktop->Release();
				}
				pIMalloc->Release();
			}
		}
	}
	return Index;
}
