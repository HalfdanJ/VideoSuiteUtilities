//
//  VideoBankPlayer.h
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VideoBank.h"

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface VideoBankPlayer : NSObject
{
    BOOL _playing;
    
    AVQueuePlayer * avPlayer[2];
    AVPlayerLayer * avPlayerLayer[2];
    
    id timeObserverToken[2];
    id timeOutTimeObserverToken[2];
    id fadeInObserverToken[2];
    id fadeOutObserverToken[2];
    id fadeOutEventObserverToken[2];
    id midiSendObserverToken[2];
//    VideoBankItem * uiUpdateBankItem[2];

}


@property int counter;
@property VideoBank * videoBank;

@property int bankSelection;
@property int numberOfBanksToPlay;
@property BOOL playing;
@property float opacity;
@property BOOL loop;
@property BOOL midi;
@property float playbackRate;
@property VideoBankItem * lastItem;

@property CALayer * layer;

@property NSString * currentTimeString;

@property int stopWhenReady;

-(id)initWithBank:(VideoBank*)bank;
-(void) qlabPlay;
-(void) qlabStop;


@end
