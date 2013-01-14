Decklink tone source filter

This filter is based on the PushSource sample from the DirectShow SDK.  It provides a continuous stream of audio samples and uses the concept of virtual files in a similar manner to the Decklink Still Source filter.  Currently only silence is implemented, for the DecklinkMediaPlayer sample, but just about any audio could be generated.  This filter supports the IAMStreamConfig interface to specify the format of the audio and for stream control, supports both the IAMStreamControl and IMediaSeeking interfaces.


Blackmagic Design.