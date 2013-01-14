//------------------------------------------------------------------------------
// DecklinkFieldSwap.h
//
// Desc: DirectShow sample code - Illustrates a very basic field swap filter implementation.
//								  Based entirely upon the NullNull filter sample in the 
//								  DirectShow SDK samples.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CDecklinkFieldSwap class
//
class CDecklinkFieldSwap : public CTransInPlaceFilter
{
public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    DECLARE_IUNKNOWN;

private:
    // Constructor - just calls the base class constructor
    CDecklinkFieldSwap(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr)
        : CTransInPlaceFilter(tszName, punk, CLSID_DecklinkFieldSwap, phr)
    {}

    // Overrides the PURE virtual Transform of CTransInPlaceFilter base class.
    // This is where the "real work" is done by altering *pSample.
    // We do the Null transform by leaving it alone.
    HRESULT Transform(IMediaSample* pSample);

    // We accept any input type.  We'd return S_FALSE for any we didn't like.
    HRESULT CheckInputType(const CMediaType* mtIn);
};
