Decklink Histogram Sample

The purpose of this sample is to demonstrate the retrieval of video samples into an application from a very basic video capture graph.  The application then performs, by way of example, a histogram analysis of the video which is then plotted in the application window.

When the application starts up a capture graph is constructed which incorporates the sample grabber filter and the default video renderer is used, but is not required, for a video preview.

In the application a picture control is used to preview the incoming video and a few hooks are in place to process any windows messages received by the application dialog in order to update this control.

The sample grabber filter can be configured in a number of ways, the method chosen for this sample was to use the buffering mode of the filter.  A separate thread is used to poll the sample grabber filter for media samples, generate the histogram and update the custom histogram control, a modified version of a static control.

The connection media type for the sample grabber must be specified before the filter is connected.  This sample has two choices, YUV or RGB.  As the DeckLink capture filter only provided YUV frames and conversion is required if the samples received in the sample grabber callback are to be RGB.  The histogram control will plot either individual R, G and B histograms or a luminosity histogram depending upon the connection media type.


Blackmagic Design.