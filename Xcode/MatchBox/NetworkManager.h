//Copyright (c) 2011 Sebastian Böhm sebastian@sometimesfood.org
//                   Heinrich Fink hf@hfink.eu
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

#import <Foundation/Foundation.h>
#import "NetworkManagerDelegate.h"

enum {
    kSendBufferSize = 65536
};

@interface NetworkManager : NSObject
    <NSNetServiceBrowserDelegate, NSStreamDelegate, NSNetServiceDelegate>
{
@private
    unsigned                    connectionCount;
    id<NetworkManagerDelegate>  delegate;
    
    // service discovery members
    NSMutableArray *netServices;
    NSNetServiceBrowser *serviceBrowser;
    
    // send members
    NSNetService *              sendTargetService;
    NSOutputStream *            networkOutputStream;
    NSInputStream *             localInputStream;
    uint8_t                     sendBuffer[kSendBufferSize];
    size_t                      sendBufferOffset;
    size_t                      sendBufferLimit;
    
    // receive members
    NSNetService *              receiverService;
    CFSocketRef                 listeningSocket;
    NSInputStream *             networkInputStream;
    NSOutputStream *            receivedFileStream;
    NSString *                  receivedFilePath;
}

#pragma mark Class Methods
+ (id)sharedNetworkManager;

#pragma mark Send Methods
- (void)startSendWithFilePath:(NSString *)filePath;
- (void)stopSendWithReason:(NSString *)reason;

#pragma mark Receiver Methods
- (void)startServer;
- (void)stopServerWithReason:(NSString *)reason;

#pragma mark Network Status Properties
@property (nonatomic, readonly) BOOL isSending;
@property (nonatomic, readonly) BOOL isServing;

#pragma mark Delegate Property
@property (nonatomic, retain)   id<NetworkManagerDelegate> delegate;

@end
