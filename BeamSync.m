#import "BeamSync.h"


extern void CGSSetDebugOptions(int);
extern void CGSDeferredUpdates(int);

typedef enum {
    disableBeamSync = 0,
    automaticBeamSync = 1,
    forcedBeamSyncMode = 2
} beamSyncMode;



@implementation BeamSync

+(void) disable{
    //Disable beam sync
    beamSyncMode mode = disableBeamSync;
    
    CGSSetDebugOptions(mode ? 0 : 0x08000000);
    CGSDeferredUpdates(mode);
}

+(void) enable{
    //Disable beam sync
    beamSyncMode mode = automaticBeamSync;
    
    CGSSetDebugOptions(mode ? 0 : 0x08000000);
    CGSDeferredUpdates(mode);
}


@end
