Decklink Capture Sample

The purpose of this sample is to demonstrate a very basic video capture application, programmatic setting and restoring the video and audio formats through the IAMStreamConfig interface.  Capturing to a compressed format is also possible provided the encoder is available on the host.

When the application starts up the system registry is queried for any previously stored state information, the last video and audio formats.  A preview graph is then created, programmatically setting the video and audio formats of the Decklink capture filters.

In the application a picture control is used to preview the incoming video and a few hooks are in place to process any windows messages received by the application dialog in order to update this control.

Any changes to the video format, audio format or compression format results in the preview graph being rebuilt with the new user settings.

When capturing to a file, the preview graph is completed by adding the extra components to write the uncompressed, or compressed steam to disk.  In the case of DV capture this would involve adding the DV encoder, AVI mux and file writer filters to the preview graph.

In order to build the most efficient capture graph the ICaptureGraphBuilder2 interface is NOT used.  Instead a number of supplemental functions have been written that allow graph building 'by hand'.  It is possible to use graphedit to attach to the capture sample application in order to view the filter graph that has been created.


Blackmagic Design.