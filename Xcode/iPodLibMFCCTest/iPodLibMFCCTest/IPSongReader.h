//
//  IPSongReader.h
//  iPodLibMFCCTest
//
//  Created by Heinrich Fink on 5/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CMTimeRange.h>
#import <CoreMedia/CMSampleBuffer.h>

@interface IPSongReader : NSObject {
    
}

//Make note that the block will be called synchronously, and that CMSampleBufferRef
//has to be retained in order to be used somewhere else...

- (id)initWithURL:(NSURL*)url 
     forTimeRange:(CMTimeRange)timeRange 
        withBlock:(BOOL (^)(CMSampleBufferRef)) handler;

- (void)startReadingRange;
- (void)stopReadingRange;

//continue in here...

@end
