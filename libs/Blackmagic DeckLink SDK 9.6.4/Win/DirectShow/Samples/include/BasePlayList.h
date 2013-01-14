#pragma once

#include "BaseClip.h"
#include <map>
using namespace std;

// CBasePlayList

class CBasePlayList : public CListCtrl
{
	DECLARE_DYNAMIC(CBasePlayList)

public:
	CBasePlayList();
	virtual ~CBasePlayList() {}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);

	int AddItem(CBaseClip* pClip);	// add a clip to the playlist
	int AddItem(int item, CBaseClip* pClip);	// add a clip to the playlist
	int RemoveItem(void);	// remove a clip from the playlist
	void MoveItemUp(void);	// move an item up in the playlist
	void MoveItemDown(void);	// move an item down in the playlist

	BOOL SetSelectedItem(int item);	// highlight the specified item
	int GetSelectedItem(void);	// return the index of the selected item

	void SetInpoint(int Item, CTimecode& Timecode);	// set the inpoint of a clip and update the UI
	CTimecode Inpoint(int Item);	// get the inpoint of a clip and update the UI
	void SetOutpoint(int Item, CTimecode& Timecode);	// set the outpoint of a clip and update the UI
	CTimecode Outpoint(int Item);	// get the outpoint of a clip and update the UI

protected:
	CImageList m_ImageList;
	bool m_bInit;

	map<basic_string<TCHAR>, int> m_IconExtensions;

	DECLARE_MESSAGE_MAP()
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);	// overridden to highlight the entire row of a selected item
	int GetIconIndex(CBaseClip* pClip);
};


