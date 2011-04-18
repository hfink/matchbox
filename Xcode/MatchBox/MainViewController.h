//Copyright (c) 2011 Sebastian Bšhm sebastian@sometimesfood.org
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

#import "SettingsViewController.h"
#import "SOVoiceRecorder.h"
#import "MBCorrectnessView.h"
#import "MessagingViewController.h"
#import "NetworkManagerDelegate.h"

#include "WordMatch.h"

//Describes the mode the view is currently in. Player's and Creator's mode
//are just slightly different.
typedef enum {
    kMatchBoxCreatorsMode,
    kMatchBoxPlayersMode,
    kMatchBoxControlModeCount
} MatchBoxControlMode;

extern NSString * const kMatchBoxInfoDictKeyPeak;
extern NSString * const kMatchBoxInfoDictKeyNormalizationFactor;
extern NSString * const kMatchBoxInfoDictKeyThresholdBeginTime;
extern NSString * const kMatchBoxInfoDictKeyThresholdEndTime;

@interface MainViewController : UIViewController <  SettingsViewControllerDelegate, 
                                                    UIImagePickerControllerDelegate,
                                                    UINavigationControllerDelegate,
                                                    UIActionSheetDelegate,
                                                    AVAudioPlayerDelegate,
                                                    MessagingViewControllerDelegate,
                                                    NetworkManagerDelegate> 
{
    
    float comparisonThreshold_;
    float trimBeginThreshold_;
    float trimEndThreshold_;
    
    MatchBoxControlMode currentMode_;    
    
    NSURL * myAudioFileURL_;
    NSURL * myAudioFilePropertiesURL_;        
    NSURL * myImageFileURL_;
    WMAudioFilePreProcessInfo myFileProcessInfo_;        
    
    NSURL * othersAudioFileURL_;       
    NSURL * othersImageFileURL_; 
    WMAudioFilePreProcessInfo othersFileProcessInfo_;    
    
    AVAudioPlayer* audioPlayer_;    
    SOVoiceRecorder* voiceRecorder_; 
    
    NSFileManager* fileManager_;    
    
    SInt32 numberOfTriesLeft_;        
    
    MBCorrectnessView *uiCorrectnessIndicatorView;   
    
    UIImage* startImage_;
    
    BOOL isTesting_;
    
    NSDate * lastReceiveOperation; 
    
    //UIKit objects that we manage by ourselves
    UIActionSheet* actionSheetPerform_;
    UIActionSheet* actionSheetConfirmSolution_;    
    UIAlertView* alertLost_;
    UIAlertView* alertWon_;
    UIAlertView* alertSayItAgain_;    
    
    //UIKit objects which are properties and used as outlets
    
    UIToolbar *uiCreatorsToolbar;
    UIToolbar *uiPlayersToolbar;
    UIImageView *uiImageView;
    UIBarButtonItem *uiActionSheetBarButtonItem;
    UIBarButtonItem *uiPlayersControlDoneButton;
    UINavigationBar *uiNavigationBar;
    UIToolbar *uiMainToolbar;
    UIBarButtonItem *uiMyFilePlayButton;
    UIBarButtonItem *uiMyFileRecordButton;
    UILabel *uiNumberOfTriesLeftLabel;
    UIBarButtonItem *uiPlaySolutionButton;
    UIBarButtonItem *uiDescribeAndCompareButton;
    UIView *uiCorrectnessCircle;
}

@property (nonatomic, retain) IBOutlet UIView *uiCorrectnessCircle;
@property (nonatomic, retain) NSDate* lastReceiveOperation;

@property (nonatomic, retain) UIImage * startImage;

@property (nonatomic, retain) IBOutlet MBCorrectnessView *uiCorrectnessIndicatorView;
@property (nonatomic, retain) UIImagePickerController * imagePicker;
@property (nonatomic, retain) AVAudioPlayer * audioPlayer;

#pragma mark Interface builder outlets

@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiDescribeAndCompareButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiPlaySolutionButton;
@property (nonatomic, retain) IBOutlet UILabel *uiNumberOfTriesLeftLabel;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiMyFileRecordButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiMyFilePlayButton;
@property (nonatomic, retain) IBOutlet UIToolbar *uiMainToolbar;
@property (nonatomic, retain) IBOutlet UINavigationBar *uiNavigationBar;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiPlayersControlDoneButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *uiActionSheetBarButtonItem;
@property (nonatomic, retain) IBOutlet UIToolbar *uiCreatorsToolbar;
@property (nonatomic, retain) IBOutlet UIToolbar *uiPlayersToolbar;
@property (nonatomic, retain) IBOutlet UIImageView *uiImageView;

#pragma mark IBActions

- (IBAction)describeAndCompare:(id)sender;
- (IBAction)playSolution:(id)sender;
- (IBAction)playMyFile:(id)sender;
- (IBAction)recordIntoMyFile:(id)sender;
- (IBAction)showInfo:(id)sender;
- (IBAction)pickImageWithCamera:(id)sender;
- (IBAction)pickImageFromLibrary:(id)sender;
- (IBAction)presentActionSheet:(id)sender;
- (IBAction)doneWithPlayMode:(id)sender;

- (void)loadInitialState;
- (void)checkForCompletenessAndEnableActionSheet;
- (void)prepareAudioPlayerForURL:(NSURL*)url;
- (void)switchCurrentModeTo:(MatchBoxControlMode)mode;
- (void)playbackOthersFile;
- (void)loadSafeState;
- (void)setCorrectnessLevel:(CGFloat) level 
              withAnimation:(BOOL) animate;

#pragma mark Stubs for Sending/Receiving Game States
- (void)prepareAndSendGameStatus;
//- (void)receiveGameStatus;
//- (void)copyReceivedGameStatusToOthers;

#pragma mark Utility Methods

+ (NSDictionary*)createDictionaryWithProcessInfo:(WMAudioFilePreProcessInfo)info;
+ (WMAudioFilePreProcessInfo)getProcessInfoFromDictionary:(NSDictionary*)dict;

@end
