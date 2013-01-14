Decklink field swap sample

This filter is a very simple trans-in-place filter which achieves a field swap for PAL 8 bit YUV.

It was developed to address the field order difference between PAL SD and PAL DV so that PAL DV can be played back with the correct field order on Decklink equipment.

The field swap is achieved by moving the video data in the buffer down by one line.  The side effect of the field swap is a loss of one active line which is replaced by black video.

We cannot do a literal field swap where we would copy field 1 to field 2 and vice versa as this would change the relationship between the video in the two fields.  Imagine a diagonal line in the video before and after this kind of operation.

The sample was developed from the DirectX SDK October 2004 release NullNull sample and built with Dev Studio 7.0.  You will probably have to modify the project/dev studio settings to point to your include and library paths.

Use regsvr32.exe to register BMDFieldSwap1.ax.  The filter appears under the DirectShow filters category as 'BMD PAL Field Swap'.  Insert this filter between the DV decoder and the Decklink video render filter, re: fieldswapgrf.jpg.

One final point, I thought that this method would be the best path to take in addressing the PAL DV field order issue.  Once support for PAL DV is available in the Decklink video render filter any existing 3rd party code using a field swap filter will still work.  Any new 3rd party code developed should be able to take advantage of the new support in the Decklink video render filter resulting in minimum impact for 3rd party developers.


Blackmagic Design.