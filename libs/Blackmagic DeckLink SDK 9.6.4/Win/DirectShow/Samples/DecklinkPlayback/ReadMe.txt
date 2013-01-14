Decklink Playback Sample

This sample demonstrates building a very simple file playback graph using the Decklink video render filter.  The file media is rendered on Decklink hardware and the video is also previewed within the application.

The graph building used for playback is very crude.  For compressed media such as DV, MPEG and MPEG2 the intelligent connect of the graph builder usually puts the infinite T filter before any decoder filters.  So when rendering to Decklink and to the video preview window, two decoders instances are created which is not a very efficient use of processor cycles.  The graph builder should be optimised to place the infinite T filter after a decoder for compressed media playback.


Blackmagic Design.