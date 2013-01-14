Decklink Export to Tape Sample

The purpose of this sample is to demonstrate frame accurate print-to-tape using the IAMExtTransport interface.  If no deck is present then the sample becomes a very simple play list application.

When the application starts a project video format is specified so that clips added to the play list can be verified.  A very basic playback graph constructed using the selected external renderer device so that timecode can be interpreted correctly while using the deck controls.

A customised report style CListCtrl object simplifies the insertion, positioning and removal of clips from this object,  while a generic CClip class is used to store details of every clip in the play list.

The IMediaDet interface is used prior to inserting a clip into the play list to extract clip frame rate and clip duration parameters which are used in conjunction with the in and out points of a clip when exporting to tape.

When exporting to tape, the deck is set to insert edit mode.  This means the deck controls the points at which the video and audio tracks will be recorded whilst preserving the control track.  All the application has to do is output the video at the correct time.

For each clip in the play list, a simple file playback graph is constructed.  If there is a deck present then an insert edit mode is setup and the application waits for the inpoint before starting the graph.  Otherwise if no deck is present the graph is started immediately.  A few optimisations have been made in order to start the graph as quickly as possible: pausing the graph to get the streams to allocate their resources in advance of streaming and using the IMediaSeeking interface to force the file source filter to cache some frames off disk.

While exporting to tape the application waits for the outpoint, or during playback it waits for the EC_COMPLETE notification from the graph, before proceeding with the next clip.

Blackmagic Design.