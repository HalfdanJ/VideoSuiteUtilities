//
//  QlabDragger.h
//  SH
//
//  Created by Flyvende Grise on 1/11/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface QlabDragger : NSView<NSDraggingSource,NSDraggingDestination>
@property NSMutableDictionary * propertyList;

@property NSArray * types;
@property NSMutableDictionary * values;

@end
