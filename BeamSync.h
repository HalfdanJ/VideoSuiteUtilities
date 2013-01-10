// This class can enable and disable beamsync. 
// Disabling beamsync improves the opengl performance alot often

#import <Foundation/Foundation.h>

@interface BeamSync : NSObject 
+(void) enable;
+(void) disable;
@end
