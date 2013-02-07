//
//  Filters.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "Filters.h"
#import "QLabController.h"

@interface Filters ()

@property NSMutableArray * allFilters;

@property (readonly) NSAffineTransform * transform;


@property CIFilter * transformFilter;
@property DeinterlaceFilter * deinterlaceFilter;

@end


@implementation Filters
static void *UpdateFiltersContext = &UpdateFiltersContext;

-(NSString*)name {
    return @"Filters";
}

- (id)init
{
    self = [super init];
    if (self) {
        self.allFilters = [NSMutableArray array];

        
        self.colorControlsFilter = [CIFilter filterWithName:@"CIColorControls"] ;
        [self.allFilters addObject:self.colorControlsFilter];
        
        self.deinterlaceFilter = [[DeinterlaceFilter alloc] init];
        [self.allFilters addObject:self.deinterlaceFilter];
        
        self.transformFilter = [CIFilter filterWithName:@"CIAffineTransform"];
        [self.transformFilter bind:@"inputTransform" toObject:self withKeyPath:@"transform" options:nil];
        [self.allFilters addObject:self.transformFilter];
        
        //  [self.colorControlsFilter setValue:@(0) forKey:@"inputSaturation"];

        
        [self makeDefaults];
        
        
        num = 100;
        
        [globalMidi addBindingTo:self selector:@"makeDefaults" channel:1 number:num++];
        [globalMidi addBindingTo:self path:@"transformX" channel:1 number:num++ rangeMin:-1000/5 rangeLength:2000/5];
        [globalMidi addBindingTo:self path:@"transformY" channel:1 number:num++ rangeMin:-1000/5 rangeLength:2000/5];
        [globalMidi addBindingTo:self path:@"transformScale" channel:1 number:num++ rangeMin:0.5 rangeLength:2];
        [globalMidi addBindingTo:self path:@"deinterlace" channel:1 number:num++ rangeMin:0 rangeLength:1];
        
        for(CIFilter * filter in self.allFilters){
            [self observeFilter:filter];
        }
      

    }
    return self;
}

-(void)observeFilter:(CIFilter*)filter{


    for(NSString * key in filter.inputKeys){
        if(![key isEqualToString:@"inputImage"]){
            NSDictionary * attributes = [filter attributes];

            [filter addObserver:self forKeyPath:key options:0 context:UpdateFiltersContext];
            
            
            NSDictionary * keyAttributes = [attributes valueForKey:key];
            
            NSLog(@"att %@",keyAttributes);
            
            if([[keyAttributes valueForKey:@"CIAttributeClass"] isEqualToString:@"NSNumber"]){
                float min = [[keyAttributes valueForKey:@"CIAttributeSliderMin"] floatValue];
                float max = [[keyAttributes valueForKey:@"CIAttributeSliderMax"] floatValue];
                [globalMidi addBindingTo:filter path:key channel:1 number:num++ rangeMin:min rangeLength:max-min];
                
            }

        }
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context== UpdateFiltersContext){
        [self willChangeValueForKey:@"filters"];
        [self didChangeValueForKey:@"filters"];
    }
}

-(NSArray *)filters{
  //  NSLog(@"filters");
    
    NSMutableArray * arr = [NSMutableArray array];
    
    if(self.deinterlace){
        [arr addObject:self.deinterlaceFilter];
    }
    
    [arr addObjectsFromArray:@[self.colorControlsFilter, self.transformFilter] ];
    
    return arr;
}

+(NSSet *)keyPathsForValuesAffectingFilters{
    return [NSSet setWithObjects:@"deinterlace", nil];
}

-(void)makeDefaults{
    for(CIFilter * filter in self.allFilters){
        [filter setDefaults];
    }

    self.deinterlace = YES;
    
    self.transformX = 0;
    self.transformY = 0;
    self.transformScale = 1.0;
}

-(NSAffineTransform *)transform{
    NSAffineTransform * transform = [NSAffineTransform transform];
    [transform translateXBy:self.transformX yBy:self.transformY];
    [transform scaleXBy:self.transformScale yBy:self.transformScale];
    return transform;
}

+(NSSet *)keyPathsForValuesAffectingTransform{
    return [NSSet setWithObjects:@"transformX",@"transformY",@"transformScale", nil];
}

-(void) qlabDefaults{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Defaults"], QSelector: @"makeDefaults"},
    ];
    
    NSString * title = [NSString stringWithFormat:@"Filter defaults"];
    
    [QLabController createCues:cues groupTitle:title sender:self];

    
    
}

-(void) qlabCurrent{
    NSMutableArray * b = [NSMutableArray array];
    
    for(CIFilter * filter in self.allFilters){
        
        for(NSString * key in filter.inputKeys){
            if(![key isEqualToString:@"inputImage"]){
                NSDictionary * attributes = [filter attributes];
                NSDictionary * keyAttributes = [attributes valueForKey:key];

                if([[keyAttributes valueForKey:@"CIAttributeClass"] isEqualToString:@"NSNumber"]){
                    [b addObject:@{QName : [NSString stringWithFormat:@"%@: %@",key, [filter valueForKey:key]], QPath: key, QObject: filter}];
                }
                
            }
        }
    }
    
    [b addObject:@{QName : [NSString stringWithFormat:@"TransformX: %.2f", self.transformX], QPath: @"transformX"}];
    [b addObject:@{QName : [NSString stringWithFormat:@"TransformY: %.2f", self.transformY], QPath: @"transformY"}];
    [b addObject:@{QName : [NSString stringWithFormat:@"TransformScale: %.2f", self.transformScale], QPath: @"transformScale"}];
    [b addObject:@{QName : [NSString stringWithFormat:@"Deinterlace: %i", self.deinterlace], QPath: @"deinterlace"}];

    
    NSString * title = [NSString stringWithFormat:@"Set filters"];
    
    [QLabController createCues:b groupTitle:title sender:self];
}



@end
