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
//
//  singleton code from one of Matt Galloway's tutorials:
//      http://iphone.galloway.me.uk/iphone-sdktutorials/singleton-classes/

#import "NetworkManager.h"

#include <CFNetwork/CFNetwork.h>
#import <UIKit/UIKit.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


static NetworkManager *sharedNetworkManager = nil;

@interface NetworkManager()
@property (nonatomic)           unsigned                   connectionCount;

// sender properties that don't need to be seen by the outside world
@property (nonatomic, retain)   NSNetService *    sendTargetService;
@property (nonatomic, retain)   NSOutputStream *  networkOutputStream;
@property (nonatomic, retain)   NSInputStream *   localInputStream;
@property (nonatomic, readonly) uint8_t *         sendBuffer;
@property (nonatomic, assign)   size_t            sendBufferOffset;
@property (nonatomic, assign)   size_t            sendBufferLimit;

// receiver properties that don't need to be seen by the outside world
@property (nonatomic, readonly) BOOL                isReceiving;
@property (nonatomic, retain)   NSNetService *      receiverService;
@property (nonatomic, assign)   CFSocketRef         listeningSocket;
@property (nonatomic, retain)   NSInputStream *     networkInputStream;
@property (nonatomic, retain)   NSOutputStream *    receivedFileStream;
@property (nonatomic, copy)     NSString *          receivedFilePath;

// forward declarations
- (NSNetService *)randomService;
- (void)increaseConnectionCount;
- (void)decreaseConnectionCount;
- (NSString *)pathForTemporaryFileWithPrefix:(NSString *)prefix;

@end


@implementation NetworkManager
#pragma mark Generic Networking Methods

@synthesize connectionCount;
@synthesize delegate;

- (void)increaseConnectionCount
{
    if (connectionCount++ == 0)
        [self.delegate didStartNetworking];
}

- (void)decreaseConnectionCount
{
    assert(self.connectionCount > 0);
    if (--self.connectionCount == 0)
        [self.delegate didStopNetworking];
}

#pragma mark Network Receiver Methods

@synthesize receiverService;
@synthesize networkInputStream;
@synthesize listeningSocket;
@synthesize receivedFileStream;
@synthesize receivedFilePath;

- (BOOL)isServing
{
    return (self.receiverService != nil);
}

- (BOOL)isReceiving
{
    return (self.networkInputStream != nil);
}

// Have to write our own setter for listeningSocket because CF gets grumpy 
// if you message NULL.

- (void)setListeningSocket:(CFSocketRef)newValue
{
    if (newValue != self->listeningSocket) {
        if (self->listeningSocket != NULL) {
            CFRelease(self->listeningSocket);
        }
        self->listeningSocket = newValue;
        if (self->listeningSocket != NULL) {
            CFRetain(self->listeningSocket);
        }
    }
}

- (void)_startReceive:(int)fd
{
    [self increaseConnectionCount];
    CFReadStreamRef     readStream;
    
    assert(fd >= 0);
    
    assert(self.networkInputStream == nil);      // can't already be receiving
    assert(self.receivedFileStream == nil);         // ditto
    assert(self.receivedFilePath == nil);           // ditto
    
    // Open a stream for the file we're going to receive into.
    self.receivedFilePath = [self pathForTemporaryFileWithPrefix:@"Receive"];
    assert(self.receivedFilePath != nil);
    
    self.receivedFileStream = [NSOutputStream outputStreamToFileAtPath:self.receivedFilePath append:NO];
    assert(self.receivedFileStream != nil);
    
    [self.receivedFileStream open];
    
    // Open a stream based on the existing socket file descriptor.  Then configure 
    // the stream for async operation.
    
    CFStreamCreatePairWithSocket(NULL, fd, &readStream, NULL);
    assert(readStream != NULL);
    
    self.networkInputStream = (NSInputStream *) readStream;
    
    CFRelease(readStream);
    
    [self.networkInputStream setProperty:(id)kCFBooleanTrue forKey:(NSString *)kCFStreamPropertyShouldCloseNativeSocket];
    
    self.networkInputStream.delegate = self;
    [self.networkInputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [self.networkInputStream open];
    
    // Tell the UI we're receiving.
    [self.delegate receiveDidStart];
}

- (void)_stopReceiveWithStatus:(NSString *)statusString
{
    if (self.networkInputStream != nil) {
        self.networkInputStream.delegate = nil;
        [self.networkInputStream removeFromRunLoop:[NSRunLoop currentRunLoop]
                                           forMode:NSDefaultRunLoopMode];
        [self.networkInputStream close];
        self.networkInputStream = nil;
    }
    if (self.receivedFileStream != nil) {
        [self.receivedFileStream close];
        self.receivedFileStream = nil;
    }
    if (statusString != nil)
        // receive error, set file path to nil
        self.receivedFilePath = nil;
    
    [self.delegate receiveDidStopWithStatus:statusString
                                andFilePath:self.receivedFilePath];
    self.receivedFilePath = nil;
    [self decreaseConnectionCount];
}

- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode
// An NSStream delegate callback that's called when events happen on our 
// network stream.
{
#pragma unused(aStream)
// TODO: uncomment this again once the receiver/sender split is complete    
//    assert(aStream == self.networkStream);
    
    switch (eventCode) {
          ///////////////////////
         // receiver handling //
        ///////////////////////
        case NSStreamEventOpenCompleted: {
            // opened connection
        } break;
        case NSStreamEventHasBytesAvailable: {
            // receiving
            NSInteger       bytesRead;
            uint8_t         buffer[32768];
            
            // Pull some data off the network.
            bytesRead = [self.networkInputStream read:buffer maxLength:sizeof(buffer)];
            if (bytesRead == -1) {
                [self _stopReceiveWithStatus:@"Network read error"];
            } else if (bytesRead == 0) {
                [self _stopReceiveWithStatus:nil];
            } else {
                NSInteger   bytesWritten;
                NSInteger   bytesWrittenSoFar;
                
                // Write to the file.
                bytesWrittenSoFar = 0;
                do {
                    bytesWritten = [self.receivedFileStream write:&buffer[bytesWrittenSoFar] maxLength:bytesRead - bytesWrittenSoFar];
                    assert(bytesWritten != 0);
                    if (bytesWritten == -1) {
                        [self _stopReceiveWithStatus:@"File write error"];
                        break;
                    } else {
                        bytesWrittenSoFar += bytesWritten;
                    }
                } while (bytesWrittenSoFar != bytesRead);
            }
        } break;
//        case NSStreamEventHasSpaceAvailable: {
//            assert(NO);     // should never happen for the output stream
//        } break;
        case NSStreamEventErrorOccurred: {
            [self _stopReceiveWithStatus:@"Stream open error"];
        } break;
        case NSStreamEventEndEncountered: {
            // ignore
        } break;
            
          /////////////////////
         // sender handling //
        /////////////////////
//        case NSStreamEventOpenCompleted: {
//            [self _updateSendStatus:@"Opened connection"];
//        } break;
//        case NSStreamEventHasBytesAvailable: {
//            assert(NO);     // should never happen for the output stream
//        } break;
        case NSStreamEventHasSpaceAvailable: {
            // If we don't have any data buffered, go read the next chunk of data.
            if (self.sendBufferOffset == self.sendBufferLimit) {
                NSInteger   bytesRead;
                
                bytesRead = [self.localInputStream read:self.sendBuffer
                                              maxLength:kSendBufferSize];
                if (bytesRead == -1) {
                    [self stopSendWithReason:@"File read error"];
                } else if (bytesRead == 0) {
                    [self stopSendWithReason:nil];
                } else {
                    self.sendBufferOffset = 0;
                    self.sendBufferLimit  = bytesRead;
                }
            }
            
            // If we're not out of data completely, send the next chunk.
            if (self.sendBufferOffset != self.sendBufferLimit) {
                NSInteger   bytesWritten;
                bytesWritten = [self.networkOutputStream
                                write:&self.sendBuffer[self.sendBufferOffset]
                                maxLength:self.sendBufferLimit - self.sendBufferOffset];
                assert(bytesWritten != 0);
                if (bytesWritten == -1) {
                    [self.delegate sendDidStopWithStatus:@"Network write error"];
                } else {
                    self.sendBufferOffset += bytesWritten;
                }
            }
        } break;
//        case NSStreamEventErrorOccurred: {
//            [self _stopSendWithStatus:@"Stream open error"];
//        } break;
//        case NSStreamEventEndEncountered: {
//            // ignore
//        } break;

        default: {
            assert(NO);
        } break;
    }
}

- (void)_acceptConnection:(int)fd
{
    int     junk;
    
    // If we already have a connection, reject this new one.  This is one of the 
    // big simplifying assumptions in this code.  A real server should handle 
    // multiple simultaneous connections.
    
    if ( self.isReceiving ) {
        junk = close(fd);
        assert(junk == 0);
    } else {
        [self _startReceive:fd];
    }
}

static void AcceptCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
// Called by CFSocket when someone connects to our listening socket.  
// This implementation just bounces the request up to Objective-C.
{
    NetworkManager *  obj;
    
#pragma unused(type)
    assert(type == kCFSocketAcceptCallBack);
#pragma unused(address)
    // assert(address == NULL);
    assert(data != NULL);
    
    obj = (NetworkManager *) info;
    assert(obj != nil);
    
#pragma unused(s)
    assert(s == obj->listeningSocket);
    
    [obj _acceptConnection:*(int *)data];
}

- (void)netService:(NSNetService *)sender didNotPublish:(NSDictionary *)errorDict
// A NSNetService delegate callback that's called if our Bonjour registration 
// fails.  We respond by shutting down the server.
//
// This is another of the big simplifying assumptions in this sample. 
// A real server would use the real name of the device for registrations, 
// and handle automatically renaming the service on conflicts.  A real 
// client would allow the user to browse for services.  To simplify things 
// we just hard-wire the service name in the client and, in the server, fail 
// if there's a service name conflict.
{
#pragma unused(sender)
    assert(sender == self.receiverService);
#pragma unused(errorDict)
    
    [self stopServerWithReason:@"Registration failed"];
}

- (void)startServer
{
    BOOL        success;
    int         err;
    int         fd;
    int         junk;
    struct sockaddr_in addr;
    int         port;
    
    // Create a listening socket and use CFSocket to integrate it into our 
    // runloop.  We bind to port 0, which causes the kernel to give us 
    // any free port, then use getsockname to find out what port number we 
    // actually got.
    
    port = 0;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    success = (fd != -1);
    
    if (success) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_len    = sizeof(addr);
        addr.sin_family = AF_INET;
        addr.sin_port   = 0;
        addr.sin_addr.s_addr = INADDR_ANY;
        err = bind(fd, (const struct sockaddr *) &addr, sizeof(addr));
        success = (err == 0);
    }
    if (success) {
        err = listen(fd, 5);
        success = (err == 0);
    }
    if (success) {
        socklen_t   addrLen;
        
        addrLen = sizeof(addr);
        err = getsockname(fd, (struct sockaddr *) &addr, &addrLen);
        success = (err == 0);
        
        if (success) {
            assert(addrLen == sizeof(addr));
            port = ntohs(addr.sin_port);
        }
    }
    if (success) {
        CFSocketContext context = { 0, self, NULL, NULL, NULL };
        
        self.listeningSocket = CFSocketCreateWithNative(
                                                        NULL, 
                                                        fd, 
                                                        kCFSocketAcceptCallBack, 
                                                        AcceptCallback, 
                                                        &context
                                                        );
        success = (self.listeningSocket != NULL);
        
        if (success) {
            CFRunLoopSourceRef  rls;
            
            CFRelease(self.listeningSocket);        // to balance the create
            
            fd = -1;        // listeningSocket is now responsible for closing fd
            
            rls = CFSocketCreateRunLoopSource(NULL, self.listeningSocket, 0);
            assert(rls != NULL);
            
            CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
            
            CFRelease(rls);
        }
    }
    
    // Now register our service with Bonjour.  See the comments in -netService:didNotPublish: 
    // for more info about this simplifying assumption.
    if (success) {
        self.receiverService = [[[NSNetService alloc]
                                 initWithDomain:@"local."
                                 type:@"_matchbox._tcp."
                                 name:[[UIDevice currentDevice] name]
                                 port:port]
                                autorelease];
        success = (self.receiverService != nil);
    }
    if (success) {
        self.receiverService.delegate = self;
        
        [self.receiverService publishWithOptions:NSNetServiceNoAutoRename];
        
        // continues in -netServiceDidPublish: or -netService:didNotPublish: ...
    }
    
    // Clean up after failure.
    
    if ( success ) {
        assert(port != 0);
        [self.delegate serverDidStartOnPort:port];
    } else {
        [self stopServerWithReason:@"Start failed"];
        if (fd != -1) {
            junk = close(fd);
            assert(junk == 0);
        }
    }
}

- (void)stopServerWithReason:(NSString *)reason
{
    if (self.isReceiving) {
        [self _stopReceiveWithStatus:@"Cancelled"];
    }
    if (self.receiverService != nil) {
        [self.receiverService stop];
        self.receiverService = nil;
    }
    if (self.listeningSocket != NULL) {
        CFSocketInvalidate(self.listeningSocket);
        self.listeningSocket = NULL;
    }
    [self.delegate serverDidStopWithReason:reason];
}


#pragma mark Network Sender Methods

@synthesize sendTargetService;
@synthesize networkOutputStream;
@synthesize localInputStream;
@synthesize sendBufferOffset;
@synthesize sendBufferLimit;

// Because buffer is declared as an array, you have to use a custom getter.  
// A synthesised getter doesn't compile.

- (uint8_t *)sendBuffer
{
    return self->sendBuffer;
}

- (BOOL)isSending
{
    return (self.networkOutputStream != nil);
}

- (void)startSendWithFilePath:(NSString *)filePath
{
    [self increaseConnectionCount];
    NSOutputStream *    output;
    BOOL                success;
    
    assert(filePath != nil);
    
    assert(self.networkOutputStream == nil);      // don't tap send twice in a row!
    assert(self.localInputStream == nil);         // ditto
    
    // Open a stream for the file we're going to send.
    self.localInputStream = [NSInputStream inputStreamWithFileAtPath:filePath];
    assert(self.localInputStream != nil);
    
    [self.localInputStream open];
    
    // Open a stream to the server, finding the server via Bonjour.  Then configure 
    // the stream for async operation.
    self.sendTargetService = [self randomService];
    if (!self.sendTargetService)
    {
        [self stopSendWithReason:@"No target service found"];
        return;
    }
    
    success = [self.sendTargetService getInputStream:NULL outputStream:&output];
    assert(success);
    
    self.networkOutputStream = output;
    
    // -[NSNetService getInputStream:outputStream:] currently returns the stream 
    // with a reference that we have to release (something that's counter to the 
    // standard Cocoa memory management rules <rdar://problem/6868813>).
    [output release];
    
    self.networkOutputStream.delegate = self;
    [self.networkOutputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [self.networkOutputStream open];
    [self.delegate sendDidStart];
    return;
}

- (void)stopSendWithReason:(NSString *)reason
{
    if (self.networkOutputStream != nil)
    {
        self.networkOutputStream.delegate = nil;
        [self.networkOutputStream removeFromRunLoop:[NSRunLoop currentRunLoop]
                                      forMode:NSDefaultRunLoopMode];
        [self.networkOutputStream close];
        self.networkOutputStream = nil;
    }
    if (self.sendTargetService != nil)
    {
        [self.sendTargetService stop];
        self.sendTargetService = nil;
    }
    if (self.localInputStream != nil)
    {
        [self.localInputStream close];
        self.localInputStream = nil;
    }
    self.sendBufferOffset = 0;
    self.sendBufferLimit  = 0;
    [self.delegate sendDidStopWithStatus:reason];
    [self decreaseConnectionCount];
}

#pragma mark Service Discovery Methods

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser
           didFindService:(NSNetService *)aNetService
               moreComing:(BOOL)moreComing
{
    #pragma unused(aNetServiceBrowser, moreComing)
    NSLog(@"adding %@", aNetService);
    [netServices addObject:aNetService];
    NSLog(@"%d services in service list", [netServices count]);
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser
         didRemoveService:(NSNetService *)aNetService
               moreComing:(BOOL)moreComing
{
    #pragma unused(aNetServiceBrowser, moreComing)
    NSLog(@"removing %@", aNetService);
    [netServices removeObject:aNetService];
    NSLog(@"%d services in service list", [netServices count]);
}

- (NSNetService *)randomService
{
    NSMutableArray *nonlocalServices = [[NSMutableArray alloc]
                                        initWithArray:netServices];
    [nonlocalServices removeObject:[self receiverService]];
    size_t serviceCount = [nonlocalServices count];
    if (serviceCount > 0)
    {
        size_t randomIndex = rand()%serviceCount;
        return [nonlocalServices objectAtIndex:randomIndex];
    }
    else
        return nil;
}

#pragma mark Singleton Methods

+ (id)sharedNetworkManager
{
    @synchronized(self)
    {
        if(sharedNetworkManager == nil)
            sharedNetworkManager = [[super allocWithZone:NULL] init];
    }
    return sharedNetworkManager;
}

+ (id)allocWithZone:(NSZone *)zone
{
    #pragma unused(zone)
    return [[self sharedNetworkManager] retain];
}

- (id)copyWithZone:(NSZone *)zone
{
    #pragma unused(zone)
    return self;
}

- (id)retain
{
    return self;
}

- (unsigned)retainCount
{
    return UINT_MAX; //denotes an object that cannot be released
}

- (void)release
{
    // never release
}

- (id)autorelease
{
    return self;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        netServices = [[NSMutableArray alloc] init];
        serviceBrowser = [[NSNetServiceBrowser alloc] init];
        [serviceBrowser setDelegate:self];
        [serviceBrowser searchForServicesOfType:@"_matchbox._tcp"
                                       inDomain:@"local"];
    }
    return self;
}

- (void)dealloc {
    // should never be called
    [super dealloc];
}

#pragma mark Utility Functions
- (NSString *)pathForTemporaryFileWithPrefix:(NSString *)prefix
{
    NSString *  result;
    CFUUIDRef   uuid;
    CFStringRef uuidStr;
    
    uuid = CFUUIDCreate(NULL);
    assert(uuid != NULL);
    
    uuidStr = CFUUIDCreateString(NULL, uuid);
    assert(uuidStr != NULL);
    
    result = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"%@-%@", prefix, uuidStr]];
    assert(result != nil);
    
    CFRelease(uuidStr);
    CFRelease(uuid);
    
    return result;
}

@end
