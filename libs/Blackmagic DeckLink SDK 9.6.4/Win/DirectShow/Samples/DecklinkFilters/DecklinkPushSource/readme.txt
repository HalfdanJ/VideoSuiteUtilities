Decklink push source fitler

This filter, based on the SDK PushSource sample code and is used to push frames from an external application into a DirectShow graph.

This source filter has a custom allocator, a slightly modified version of the SDK CBaseAllocator, to provide a pool of samples to the graph from a delivery queue. The streaming thread of the source filter blocks in the custom allocator if there are no samples in the delivery queue. The delivery queue is fed by the application, via a custom interface on the source filter.

Support for the IAMStreamConfig interface has been added so that an application can set the connection format of the source filter's output pin before the output pin has been connected. (Dynamic reconnection is possible by adding DynSource etc. but this is out of the scope of this sample.)


Blackmagic Design.