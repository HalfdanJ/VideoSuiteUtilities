//
//  BlackMagicController.h
//  ViljensTriumf
//
//  Created by Jonas Jongejan on 05/10/12.
//
//

#import <Foundation/Foundation.h>
#import "BlackMagicItem.h"

@interface BlackMagicController : NSObject{
    std::vector<IDeckLink*>		deviceList;

}

    @property NSMutableArray * items;
    



-(DecklinkCallback*)callbacks:(int)num;


@end
