//
//  LiveMixer.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "LiveMixer.h"
#import "CoreImageViewer.h"

@interface LiveMixer ()
@property float transitionTime;

@property CIFilter * dissolveFilter;
@property CIFilter * constantColorFilter;


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

    }
    return self;
}

-(CIImage*) input:(int)num{

    switch (num) {
        case 1:
            return self.input1;
        case 2:
            return self.input2;
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
            [self performSelector:@selector(updateTransitionTime) withObject:nil afterDelay:0.01];
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
@end
