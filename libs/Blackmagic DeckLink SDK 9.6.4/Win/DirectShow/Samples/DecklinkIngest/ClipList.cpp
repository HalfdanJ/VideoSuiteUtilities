// ClipList.cpp : implementation file
//

#include "stdafx.h"
#include "DecklinkIngest.h"
#include "ClipList.h"


//-----------------------------------------------------------------------------
// CClipList
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//
IMPLEMENT_DYNAMIC(CClipList, CListCtrl)
CClipList::CClipList()
{
}

//-----------------------------------------------------------------------------
//
//
CClipList::~CClipList()
{
}

//-----------------------------------------------------------------------------
//
//
BEGIN_MESSAGE_MAP(CClipList, CListCtrl)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// AddItem
// Insert the clip into the playlist.
int CClipList::AddItem(CClip* pClip)
{
	int item = AddItem(GetItemCount(), pClip);
	SetSelectedItem(item);
	return item;
}

//-----------------------------------------------------------------------------
// AddItem
// Insert the clip into the playlist.
int CClipList::AddItem(int Item, CClip* pClip)
{
	// insert the clip name and icon for the item
	LVITEM lvi = {0};
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = Item;
	lvi.iSubItem = 0;
	basic_string<TCHAR> test = pClip->Clipname().c_str();
	lvi.pszText = (LPTSTR)test.c_str();
	lvi.iImage = -1;
	int item = InsertItem(&lvi);
	SetItemData(item, reinterpret_cast<DWORD_PTR>(pClip));
	EnsureVisible(item, FALSE);

	CString buf;
	buf = pClip->Inpoint().TimecodeToString();
	SetItemText(item, 1, buf);

	buf = pClip->Outpoint().TimecodeToString();
	SetItemText(item, 2, buf);

	buf = pClip->Duration().TimecodeToString();
	SetItemText(item, 3, buf);

	buf = pClip->Tapename().c_str();
	SetItemText(item, 4, buf);

	return item;
}

//-----------------------------------------------------------------------------
// RemoveItem
// Remove the selected item from the playlist.
void CClipList::RemoveItem(void)
{
	int sel = GetSelectedItem();
	if (-1 < sel)
	{
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(sel));
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
}

//-----------------------------------------------------------------------------
// MoveItemUp
// Move the selected item up in the playlist.
void CClipList::MoveItemUp(void)
{
	int sel = GetSelectedItem();
	if (0 < sel)
	{
		// remove the preceding item and reinsert it after the selected item
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(sel - 1));
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
void CClipList::MoveItemDown(void)
{
	int max = GetItemCount() - 1;
	int sel = GetSelectedItem();
	if ((max > sel) && (-1 != sel))
	{
		// remove this item and reinsert it after the following item
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(sel));
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
BOOL CClipList::SetSelectedItem(int item)
{
	EnsureVisible(item, FALSE);
	return SetItemState(item, LVIS_SELECTED | LVIS_DROPHILITED, LVIS_SELECTED | LVIS_DROPHILITED);
}

//-----------------------------------------------------------------------------
// GetSelectedItem
//
int CClipList::GetSelectedItem(void)
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
void CClipList::SetInpoint(int Item, CTimecode& Timecode)
{
	if (-1 < Item)
	{
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(Item));
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
CTimecode CClipList::Inpoint(int Item)
{
	CTimecode Timecode;
	if (-1 < Item)
	{
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(Item));
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
void CClipList::SetOutpoint(int Item, CTimecode& Timecode)
{
	if (-1 < Item)
	{
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(Item));
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
CTimecode CClipList::Outpoint(int Item)
{
	CTimecode Timecode;
	if (-1 < Item)
	{
		CClip* pClip = reinterpret_cast<CClip*>(GetItemData(Item));
		if (pClip)
		{
			Timecode = pClip->Outpoint();
		}
	}
	return Timecode;
}

//-----------------------------------------------------------------------------
// CClipList message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// DrawItem
// Overridden to highlight the whole row when selected.
void CClipList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
