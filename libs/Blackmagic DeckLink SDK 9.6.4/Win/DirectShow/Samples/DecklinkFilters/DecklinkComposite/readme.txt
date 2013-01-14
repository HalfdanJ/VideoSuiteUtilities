Decklink composite sample

This filter is a very simple transform filter which takes non-standard frame sizes and composites them onto frames with dimensions acceptable to Decklink hardware.

It was developed to address the problems of rendering popular video formats whose frame dimensions do not conform to SDI or HD-SDI video.

This filter accepts any size RGB or YUV media on its input pin.  The default mode of the filter is to pass the video through unmodified.  If different dimensions are specified through the property page of the filter then the source video is composited onto the destination frame using the following rules.  If the source is smaller than the destination the source is centered in the destination frame.  Conversely if the source is larger than the destination frame the central portion of the source is copied to the destination.  Care is taken to ensure that a field swap will not happen on interlaced media.

Blackmagic Design.