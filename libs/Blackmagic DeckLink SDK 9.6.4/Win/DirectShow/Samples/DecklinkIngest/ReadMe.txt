Decklink Ingest Sample

[KEYWORDS: Capture, frame accurate, GMFBridge, continuous preview, batch capture, bitmap/stills capture, EDL, timecode, VITC, SMPTE 12M, SMPTE RP188]

The purpose of this sample is to demonstrate how to frame accurately capture video clips from tape using the IAMExtTransport and IAMStreamControl interfaces and a set of custom filters collectively known as the GMFBridge†.

One of the probems with capturing with DirectShow is that the graph must be stopped in order to finalise the capture file.  When capturing clips, either manually or through a batch capture process, this will cause the preview to stop and restart each time the graph is stopped, the file closed and capture restarted into a new file.  The GMFBridge from GDCL Ltd. overcomes this problem by making it possible to capture using two graphs.  The first graph is used for previewing the capture stream, the second graph is used for writing the capture stream to disk and the GMFBridge is used as the link between them.

When the sample application starts, the source side graph for capture preview is built.  This incorporates a bridge sink filter which is used by the GMFBridge to connect to sink side graphs for file writing.  The source side graph also includes two sample grabbers.  One, in the video preview, is used to implement the stills capture feature of the sample application.  When activated this simply writes the current sample to disk as a bitmap image file.

The second sample grabber is required in order to implement frame accurate capture.  This reads timecode from HANC (RP188), VITC (SMPTE 12M) or LTC (via the RS422 serial port).  As the timecode from HANC or VITC is derived from a video frame, then the sample timestamps of these samples will match the corresponding video frame.  It is vital to have the timecode samples synchronised to the video samples since the IAMStreamControl interface can then be used to implement frame accurate control of the video stream.

The source side graph is always running and is only stopped and rebuilt if the capture device or the capture format is changed.  The sink side graph is used for writing the video capture stream to disk.  This graph also contains the video compressor if compression has been selected.  This graph is rebuilt and run for every new file.

The operation for capturing video to a file involves bridging a source side graph to a sink side graph.  In this sample the term to describe this is closing the bridge.  When the bridge is closed samples can cross from the source side to the sink side.  If a compressor is present the video samples are compressed before being written to disk.  To stop the sink side graph in order to finalise the file, the bridge is opened.  As there is no connection between the two graphs when the bridge is open, the sink side graph can be stopped and rebuilt without affecting the source side graph and the bridge simply discards source side samples.

For the capture now feature of this sample, the bridge is closed when this feature is activated.  The preferences are used to specify the file name template and capture location for files captured in this way.  Audio and video samples cross the bridge to the sink side graph until the operation is stopped.  The bridge is then opened to complete the capture.

Batch capture is performed in the same manner with the addition of monitoring timecode to determine when to start and when to stop the capture.

Support for the IAMStreamControl interface was added to the GMFBridge sink filter to precisely control which frames cross the bridge.  Simply closing the bridge is not enough to ensure only the desired frames cross the bridge so IAMStreamControl is the 'boom gate' which provides per frame control.  The timecode sample grabber monitors the timecode and the sample timestamps which are then used to determine the streams times of the capture start.  Once capture has started then the source side graph is monitored for events using the IMediaEvent interface.  The source side, bridge sink filter issues EC_STREAM_CONTROL_STOPPED when the specified number of frames have been captured.  This is the signal that capture is complete for the current clip so the bridge is opened, the file finalised and the next clip queued for capture.

A basic interface is available to enter the data required for batch capturing clips from a tape deck.  Each clip has a name, inpoint, outpoint and tape name property.  The name is used as the capture file name, the inpoint and outpoint used for frame accurate control of the streams and the tape name monitored to prompt the user to switch tapes in the tape deck for differently named tapes.


†Please read the licensing information in the GMFBridge project or visit GDCL at the following URL: www.gdcl.co.uk.

Blackmagic Design.