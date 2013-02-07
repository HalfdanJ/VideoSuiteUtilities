//
//  BlackMagicController.m
//  ViljensTriumf
//
//  Created by Jonas Jongejan on 05/10/12.
//
//

#import "BlackMagicController.h"

@implementation BlackMagicController


-(id)initWithNumItems:(int)numItems{
    self = [self init];
    if (self) {
        
        self.items = [NSMutableArray array];
        
        IDeckLinkIterator*          deckLinkIterator = NULL;
        IDeckLink*                  deckLink = NULL;
        
        // Create an iterator
        deckLinkIterator = CreateDeckLinkIteratorInstance();
        
        if(deckLinkIterator){
            // List all DeckLink devices
            while (deckLinkIterator->Next(&deckLink) == S_OK)
            {
                deviceList.push_back(deckLink);
            }
        }
        
        for(int index=0;index<numItems;index++){
            //    for(int index=0;index<deviceList.size();index++){
            int mode = 2;
        /*    if(index == 1){
                mode = 10;
            }*/
            BlackMagicItem * newItem = [[BlackMagicItem alloc] initWithDecklink:deviceList[index] mode:mode];
            if(newItem){
                newItem.index = index;
                [self.items addObject:newItem];
            }
        }
        
    }
    return self;
    
}

-(NSArray*)getDeviceNameList{
    NSMutableArray*		nameList = [NSMutableArray array];
	int					deviceIndex = 0;
	
	while (deviceIndex < deviceList.size())
	{
		CFStringRef	cfStrName;
		
		// Get the name of this device
		if (deviceList[deviceIndex]->GetDisplayName(&cfStrName) == S_OK)
		{
			[nameList addObject:(__bridge NSString *)cfStrName];
			CFRelease(cfStrName);
		}
		else
		{
			[nameList addObject:@"DeckLink"];
		}
        
		deviceIndex++;
	}
	
	return nameList;
}
/*
 -(DecklinkCallback *)callbacks:(int)num{
 return callbacks[num];
 }*/

@end
