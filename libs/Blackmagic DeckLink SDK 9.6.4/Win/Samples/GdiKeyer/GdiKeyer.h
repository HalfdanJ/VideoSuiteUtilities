
#include "DeckLinkAPI_h.h"

// The callback class is used for video input format detection in this example.
class DeckLinkKeyerDelegate : public IDeckLinkInputCallback
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
	virtual ULONG STDMETHODCALLTYPE  Release(void) { return 1; }
	virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);
};
