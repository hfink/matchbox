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


@protocol NetworkManagerDelegate <NSObject>

@required

#pragma mark Sender Status Messages
- (void)sendDidStart;
- (void)sendDidStopWithStatus:(NSString *)status; // status==nil means success

#pragma mark Receiver Status Messages
- (void)receiveDidStart;
- (void)receiveDidStopWithStatus:(NSString *)status // status==nil means success
                     andFilePath:(NSString *)path; // only non-nil on success

@optional

#pragma mark Server Status Messages
- (void)serverDidStartOnPort:(int)port;
- (void)serverDidStopWithReason:(NSString *)reason; // status==nil means "Abort"

#pragma mark Generic Status Messages
- (void)didStartNetworking;
- (void)didStopNetworking;

@end
