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

#import <UIKit/UIKit.h>
#import "NetworkManagerDelegate.h"

@protocol MessagingViewControllerDelegate;

typedef enum {
    kMatchBoxMessagingModeReceive,
    kMatchBoxMessagingModeSend,
    kMatchBoxMessagingModeCount
} MatchBoxMessagingMode;

@interface MessagingViewController : UIViewController <NetworkManagerDelegate> 
{
    int numberOfFilesToReceive_;
    int totalNumberOfFilesToReceive_;
    int currentFileIndexBeingSent_;
    NSMutableArray * receivedFiles_;
    UILabel *uiMessageLabel;
    UIProgressView *uiProgressIndicator;
}

@property (nonatomic, retain) IBOutlet UIProgressView *uiProgressIndicator;
@property (nonatomic, retain) IBOutlet UILabel *uiMessageLabel;
@property (nonatomic, assign) BOOL messagingSucceeded;
@property (nonatomic, assign) MatchBoxMessagingMode mode;
@property (nonatomic, retain) NSArray* items;

@property (nonatomic, assign) id <MessagingViewControllerDelegate, 
                                  NetworkManagerDelegate> delegate;


- (IBAction)cancelMessaging:(id)sender;

//Pass an array of absolute file paths.
- (void)startSendingFiles:(NSArray*)files;

//Set the number of files we expect to come in
- (void)setNumberOfExceptedFiles:(int)number;

- (NSArray*)receivedFiles;

@end



@protocol MessagingViewControllerDelegate
- (void)messagingViewControllerDidFinish:(MessagingViewController *)controller;
@end
