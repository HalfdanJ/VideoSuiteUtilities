//
//  BlackMagicItem.h
//  SH
//
//  Created by Flyvende Grise on 1/10/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DeckLinkAPI.h"
#import "DecklinkCallback.h"

@class BlackMagicItem;
@protocol BlackMagicItemDelegate <NSObject>

@optional
-(void)newFrameWithBufer:(CVPixelBufferRef)buffer image:(CIImage*)image item:(BlackMagicItem*)bmItem;


@end


@interface BlackMagicItem : NSObject{
    unsigned char * bytes;
}

@property IDeckLinkInput  *  deckLinkInput;
@property IDeckLinkOutput  *  deckLinkOutput;
@property DecklinkCallback * callback;
@property IDeckLinkGLScreenPreviewHelper* glhelper;

@property CIImage * inputImage;


@property (readonly) NSString * name;
@property NSString * modeDescription;
@property int index;

@property id<BlackMagicItemDelegate> delegate;


-(id) initWithDecklink:(IDeckLink*)deckLink;
-(void) newFrame:(DecklinkCallback*)callback;

-(CVPixelBufferRef) createCVImageBufferFromCallback:(DecklinkCallback*)callback;

@end
