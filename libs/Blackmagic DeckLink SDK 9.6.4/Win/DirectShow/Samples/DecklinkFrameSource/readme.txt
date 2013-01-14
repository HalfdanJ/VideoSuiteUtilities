Decklink frame source sample

The purpose of the sample is to demonstrate how to feed video frames from an application, into a DirectShow graph which has the Decklink video render filter.

There are three main components; the DirectShow graph, the application and the interface between the graph and the application.

A source filter, based on the SDK PushSource sample code, is connected to the Decklink video render filter and supplies the render filter with video frames.

This source filter has a custom allocator, a slightly modified version of the SDK CBaseAllocator, to provide a pool of samples to the graph from a delivery queue. The streaming thread of the source filter blocks in the custom allocator if there are no samples in the delivery queue. The delivery queue is fed by the application, via a custom interface on the source filter.

The application requests a frame buffer, or many frame buffers, from the source filter. If none are available this call fails. When ready, the application hands the buffer, or buffers, back to the filter through the custom interface.

The source filter attaches this buffer to a free media sample. This sample is then added to the delivery queue. If no free samples are available, this call will block until downstream filters in the graph have returned a sample to the custom allocator.

Support for the IAMStreamConfig interface has been added so that the application can set the connection format of the source filter's output pin before the output pin has been connected. (Dynamic reconnection is possible by adding DynSource etc. but this is out of the scope of this sample.)

The application can request as many buffers as needed but must deliver them in order. The application can deliver a number of buffers before running the graph, in effect, preroll. The application can deliver at a variable rate. If the application delivers at a rate faster than the average time per frame the source filter will eventually block as there will be no free samples (flood). If the application delivers at a rate slower than the average frame rate the streaming thread will eventually block as there will be no items in the delivery queue (famine).

Once the application has retrieved a buffer then it can render the video frame.  In this example a simple frame counter of frames rendered is converted into timecode.  To demonstrate a basic ticker (scrolling text) the timecode is scrolled across the frame.  This is acheived by first testing to see if the video format is interlaced.  This is important in order to make the scrolling as smooth as possible.  An interlaced frame consists of two images from two different points in time.  Half the lines from each image are combined to produce the completed frame.  The odd lines and taken from one frame and the even lines from the other in order to have the correct spatial separation.  We also have to be careful with the order of the lines since some formats show the even lines first and others show the odd lines first.  If the order is wrong the scrolling will not look smooth.  A progressive frame consists of one image from one point in time and is therefore easy to render when compared to  an interlaced frame.  Once the frame is rendering it is delivered to the DirectShow graph for rendering on the DeckLink hardware.

Blackmagic Design.