//
//  Filters.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "Filters.h"

@interface Filters ()

@property NSMutableArray * allFilters;

@property (readonly) NSAffineTransform * transform;


@property CIFilter * transformFilter;
@property DeinterlaceFilter * deinterlaceFilter;

@end


@implementation Filters
static void *UpdateFiltersContext = &UpdateFiltersContext;
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
        
        for(CIFilter * filter in self.allFilters){
            [self observeFilter:filter];
        }
    }
    return self;
}

-(void)observeFilter:(CIFilter*)filter{
    for(NSString * key in filter.inputKeys){
        if(![key isEqualToString:@"inputImage"]){
            [filter addObserver:self forKeyPath:key options:0 context:UpdateFiltersContext];
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
    NSLog(@"filters");
    
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

@end
