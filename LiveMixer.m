//
//  LiveMixer.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "LiveMixer.h"
#import "CoreImageViewer.h"
#import "QLabController.h"

@interface LiveMixer ()
@property float transitionTime;

@property CIFilter * dissolveFilter;
@property CIFilter * constantColorFilter;
@property CIFilter * HDtoSDFilter;


@end

@implementation LiveMixer

-(NSString*) name{
    return @"Live Mixer";
}

- (id)init
{
    self = [super init];
    if (self) {
        self.selectedInput = 1;
        self.transitionTime = -1;
        self.opacity = 1;
        
        
        
        self.dissolveFilter = [CIFilter filterWithName:@"CIDissolveTransition"];
        [self.dissolveFilter setDefaults];
        
        self.constantColorFilter = [CIFilter filterWithName:@"CIConstantColorGenerator"];
        [self.constantColorFilter setValue:[CIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:1.0] forKey:@"inputColor"];
        
               
        
        
        int num = 30;
        [globalMidi addBindingTo:self path:@"selectedInput" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"opacity" channel:1 number:num++ rangeMin:0 rangeLength:1];
        [globalMidi addBindingTo:self path:@"crossfade" channel:1 number:num++ rangeMin:0 rangeLength:1];


    }
    return self;
}

-(CIImage*) input:(int)num{
//    NSLog(@"Res %f",self.input2.extent.size.width);
    
    if(!self.HDtoSDFilter && self.input2){
        self.HDtoSDFilter = [CIFilter filterWithName:@"CIAffineTransform"];
        [self.HDtoSDFilter setDefaults];
        
        float w = self.input2.extent.size.width;
        float h = self.input2.extent.size.height;
        
        NSAffineTransform * transform = [NSAffineTransform transform];
        [transform scaleBy:576.0/h];
        [transform translateXBy:-((w-720)*0.5)*576.0/h yBy:0];
        [self.HDtoSDFilter setValue:transform forKey:@"inputTransform"];
    }
    
    
    switch (num) {
        case 1:
            return self.input1;
        case 2:
            [self.HDtoSDFilter setValue:self.input2 forKey:@"inputImage"];
            return [self.HDtoSDFilter valueForKey:@"outputImage"];
        case 3:
            return self.input3;
            
        default:
            [self.constantColorFilter setValue:[CIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:1.0] forKey:@"inputColor"];
            return [self.constantColorFilter valueForKey:@"outputImage"];
        
    }
}

-(CIImage *)output{
   

    CIImage * _outputImage;
    if(self.opacity == 0){
        return [self input:0];
    } 
    
    
    
    //Fading
    if(self.transitionTime >= 1){
        self.transitionTime = -1;
    }
    if(self.crossfade > 0 && self.transitionTime != -1){
        if(self.transitionTime == 0){
            [self updateTransitionTime];
        }
        [self.dissolveFilter setValue:[self input:self.fadeFromInput] forKey:@"inputImage"];
        [self.dissolveFilter setValue:[self input:self.selectedInput] forKey:@"inputTargetImage"];
        
        [self.dissolveFilter setValue:@(self.transitionTime) forKey:@"inputTime"];
        _outputImage = [self.dissolveFilter valueForKey:@"outputImage"];
   
    } else {
        _outputImage = [self input:self.selectedInput];
    }
    
    
    if(self.opacity < 1.0){
        [self.dissolveFilter setValue:[self input:0] forKey:@"inputImage"];
        [self.dissolveFilter setValue:_outputImage forKey:@"inputTargetImage"];
        
        [self.dissolveFilter setValue:@(self.opacity) forKey:@"inputTime"];
        _outputImage = [self.dissolveFilter valueForKey:@"outputImage"];
    }
    
    return _outputImage;
}

+(NSSet *)keyPathsForValuesAffectingOutput{
    return [NSSet setWithObjects:@"input1",@"input2",@"input3", nil];
}



-(void)updateTransitionTime {
    self.transitionTime += 0.51-self.crossfade*0.5;
    if(self.transitionTime < 1){
//        dispatch_async(dispatch_get_main_queue(), ^{
            [self performSelector:@selector(updateTransitionTime) withObject:nil afterDelay:0.005];
      //  });
    }
    
}


-(void)setSelectedInput:(int)selectedInput{
    if(self.crossfade == 0){
        self.transitionTime = -1;
        _selectedInput = selectedInput;
    } else {
        self.transitionTime = 0;
        self.fadeFromInput = _selectedInput;
        _selectedInput = selectedInput;
    }
}

-(int)selectedInput{
    return _selectedInput;
}


-(void) imageViewMouseDown:(CoreImageViewer*)sender{
    self.selectedInput = [sender.customData intValue];
}



-(BOOL)input1Selected{
    return self.selectedInput == 1;
}
-(BOOL)input2Selected{
    return self.selectedInput == 2;
}
-(BOOL)input3Selected{
    return self.selectedInput == 3;
}

+(NSSet *)keyPathsForValuesAffectingInput1Selected{
    return [NSSet setWithObjects:@"selectedInput", nil];
}
+(NSSet *)keyPathsForValuesAffectingInput2Selected{
    return [NSSet setWithObjects:@"selectedInput", nil];
}
+(NSSet *)keyPathsForValuesAffectingInput3Selected{
    return [NSSet setWithObjects:@"selectedInput", nil];
}

-(void) qlab{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Crossfade: %.2f",self.crossfade], QPath: @"crossfade"},
    @{QName : [NSString stringWithFormat:@"Opacity: %.2f",self.opacity], QPath: @"opacity"},
    @{QName : [NSString stringWithFormat:@"Select input: %i",self.selectedInput], QPath: @"selectedInput"}
    ];
    
    NSString * title = [NSString stringWithFormat:@"Select Live Input %i",self.selectedInput];
    if(self.opacity == 0){
        title = [NSString stringWithFormat:@"Hide Live Input"];
    }
    
    [QLabController createCues:cues groupTitle:title sender:self];

}
@end
