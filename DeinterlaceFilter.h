//
//  DeinterlaceFilter.h
//  ViljensTriumf
//
//  Created by Jonas Jongejan on 05/10/12.
//
//

#import <QuartzCore/QuartzCore.h>

@interface DeinterlaceFilter : CIFilter{
    CIImage   * _inputImage;
}

@property (strong) CIImage   *inputImage;
@end
