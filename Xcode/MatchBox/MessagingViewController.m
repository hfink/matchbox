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

#import "MessagingViewController.h"
#import "NetworkManager.h"

@implementation MessagingViewController

@synthesize delegate=delegate_;
@synthesize uiProgressIndicator;
@synthesize uiMessageLabel;
@synthesize messagingSucceeded=messagingSucceeded_;
@synthesize items=items_;
@synthesize mode=mode_;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self setMessagingSucceeded:NO];
        numberOfFilesToReceive_ = 0;
        currentFileIndexBeingSent_ = 0;
        receivedFiles_ = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [[self items] release];
    [receivedFiles_ release];
    [uiMessageLabel release];
    [uiProgressIndicator release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
}
*/

- (void)viewDidUnload
{
    [self setUiMessageLabel:nil];
    [self setUiProgressIndicator:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (IBAction)cancelMessaging:(id)sender {
    self.messagingSucceeded = NO;
    [self.delegate messagingViewControllerDidFinish:self];
}

#pragma mark Messaging intizialization
- (void)startSendingFiles:(NSArray*)files
{
    [self setItems:files];
    
    [[self uiMessageLabel] setText:@"Sending Game Status..."];
    [[self uiProgressIndicator] setProgress:.0f];
    
    [self.uiMessageLabel setNeedsDisplay];
    [self.uiProgressIndicator setNeedsDisplay];
    
    self.messagingSucceeded = NO;
    currentFileIndexBeingSent_ = 0;
    
    NetworkManager * m = [NetworkManager sharedNetworkManager];
    
    if ([self.items count] > 0) {
        
        NSObject * obj = [self.items objectAtIndex:0];
        
        if (![obj isKindOfClass:[NSString class]]) {
            [NSException raise:@"error" 
                        format:@"Items array does not contain string types."];
        }
        
        NSString * file = (NSString*)obj;
        
        [m startSendWithFilePath:file];
        
    } else {
        [delegate_ messagingViewControllerDidFinish:self];
    }
}

- (void)setNumberOfExceptedFiles:(int)number
{
 
    [receivedFiles_ removeAllObjects];
    numberOfFilesToReceive_ = number;
    totalNumberOfFilesToReceive_ = number;
    
    [self.uiMessageLabel setNeedsDisplay];
    [self.uiProgressIndicator setNeedsDisplay];    
    
    self.messagingSucceeded = NO;
    
}

- (NSArray*)receivedFiles
{
    return receivedFiles_;
}

#pragma mark NetworkManagerDelegate required methods

- (void)sendDidStart 
{
    NSLog(@"Send did start.");
    [delegate_ sendDidStart];
}

- (void)sendDidStopWithStatus:(NSString *)status // status==nil means success
{
    [delegate_ sendDidStopWithStatus:status];
    
    NSLog(@"Send did stop with %@.", status);
    
    if (status != nil) {
        NSLog(@"Could not send file %@", 
                [self.items objectAtIndex:currentFileIndexBeingSent_]);
        self.messagingSucceeded = NO;
        [delegate_ messagingViewControllerDidFinish:self];
    }
    
    //Ok, transfer succeeded, increase index of file to send
    currentFileIndexBeingSent_++;
    [[self uiProgressIndicator] setProgress:(float)currentFileIndexBeingSent_ / [self.items count]];
    
    if (currentFileIndexBeingSent_ < [self.items count]) {
        NetworkManager * m = [NetworkManager sharedNetworkManager];    
        [NSThread sleepForTimeInterval:4.0];
        [m startSendWithFilePath:(NSString*)[self.items objectAtIndex:currentFileIndexBeingSent_]];
    } else {
        
        //we are done, hurray!
        [[self uiProgressIndicator] setProgress:1.0];
        [[self uiProgressIndicator] setNeedsDisplay];
        [[self uiMessageLabel] setTextColor:[UIColor greenColor]];
        [[self uiMessageLabel] setText:@"Done"];
        [[self uiMessageLabel] setNeedsDisplay];
        
        [NSThread sleepForTimeInterval:4.0];        
        
        NSLog(@"Successfully sent all files.");
        self.messagingSucceeded = YES;
        [delegate_ messagingViewControllerDidFinish:self];
        
    }
         
}

- (void)receiveDidStart
{
    NSLog(@"Receive did start");
    //We don't want to forward this one
}

- (void)receiveDidStopWithStatus:(NSString *)status // status==nil means success
                     andFilePath:(NSString *)path // only non-nil on success
{
    NSLog(@"Receive did stop with %@", status);
    
    if (status == nil) {
        
        //We expect that the sender send the files in the correct order
        
        [receivedFiles_ addObject:path];
        NSLog(@"Successfully received: %@", path);
        
        numberOfFilesToReceive_--;
        
        float p = (totalNumberOfFilesToReceive_ - numberOfFilesToReceive_) / (float)totalNumberOfFilesToReceive_;
        
        [uiProgressIndicator setProgress:p];
        
        NSLog(@"Still need to get: %d", numberOfFilesToReceive_);
        
        if (numberOfFilesToReceive_ == 0) {
            
            [[self uiMessageLabel] setTextColor:[UIColor greenColor]];
            [[self uiMessageLabel] setText:@"Done"];
            [[self uiMessageLabel] setNeedsDisplay];
            
            [NSThread sleepForTimeInterval:4.0];            
            
            //we are done, hurray!
            self.messagingSucceeded = YES;
            [delegate_ messagingViewControllerDidFinish:self];                
        }
        
    } else {
        NSLog(@"Receive failed with status %@.", status);
        self.messagingSucceeded = NO;
        [delegate_ messagingViewControllerDidFinish:self];
    }
    
    [delegate_ receiveDidStopWithStatus:status andFilePath:path];
}

- (void)serverDidStartOnPort:(int)port
{
    NSLog(@"server did start");
    [delegate_ serverDidStartOnPort:port];
}

- (void)serverDidStopWithReason:(NSString *)reason
{
    NSLog(@"server did stop");
    [delegate_ serverDidStopWithReason:reason];
}

- (void)didStartNetworking
{
    NSLog(@"did start netwrok");
    [delegate_ didStartNetworking];
}

- (void)didStopNetworking
{
    NSLog(@"did stop network.");
    [delegate_ didStopNetworking];
}

@end
