//------------------------------------------------------------------------------
// DecklinkComposite.h
//
// Desc: DirectShow sample code - Illustrates a basic frame composition by taking
//								  a non-SDI source frame and compositing onto an
//								  SDI frame size for rendering with the Decklink
//								  video renderer.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CDecklinkComposite class
//
class CDecklinkComposite : public CTransformFilter
						, public ISpecifyPropertyPages
						, public IAMStreamConfig
{
public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    DECLARE_IUNKNOWN;

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // base class overrides, must supply these methods
    HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

    virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType* pmt);

	// ISpecifyPropertyPages interface
	STDMETHODIMP GetPages(CAUUID* pPages);

	// IAMStreamConfig interface
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppamt);
    STDMETHODIMP GetNumberOfCapabilities(int* piCount, int* piSize);
    STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppamt, BYTE* pSCC);
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pamt);

private:
	CMediaType m_mtOutput;

    // Constructor - just calls the base class constructor
    CDecklinkComposite(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr);
    ~CDecklinkComposite();
};
