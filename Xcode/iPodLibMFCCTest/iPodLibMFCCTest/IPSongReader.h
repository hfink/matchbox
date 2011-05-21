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

//Also make note, that we always seek to the middle of the file!!!
- (id)initWithURL:(NSURL*)url 
     forDuration:(float)seconds 
        withBlock:(BOOL (^)(CMSampleBufferRef)) consumer_block;

//Make note that this is blocking until all is finished or something happened
//This interface really should be enhanced for canceling, etc...
//returns true if the complete range was successfully consumed
- (BOOL)consumeRange;

//continue in here...

@end
