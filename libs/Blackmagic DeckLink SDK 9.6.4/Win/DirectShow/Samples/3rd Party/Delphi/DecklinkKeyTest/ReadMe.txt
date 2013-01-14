Blackmagic Design Delphi Code Examples
======================================

These Delphi programs are very simple examples of interfacing with a Decklink Card.
Both build a basic graph as follows:
  Decklink Still Source Filter -> Decklink Video Renderer

IDecklinkKeyer is queried to access the alpha keying function.

1. DecklinkKeyTest - Uses CoCreateInstance() to build the graph.
2. DecklinkKeyDSPackTest - Uses DSPack to build the graph.

The BMPs included in Samples\DecklinkKey\res are required.
Copy these to the same location as the EXE.

Note that #2 utilizes the (free) Graphic32 package available from
http://sourceforge.net/projects/graphics32.
This is only used for the custom bitmap created.  This package
makes it easier to build an alpha bitmap.  Alternative methods
could of course be used.

More information is included in the respective main form pas files.

Enhancements to these programs and additional examples written
would be most welcomed by other Delphi programers.

Enjoy

Ian Krigsman
www.discoverysystems.com.au