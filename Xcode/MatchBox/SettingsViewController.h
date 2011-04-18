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
#import <UIKit/UIKit.h>

@protocol SettingsViewControllerDelegate;

extern const float kMatchBoxDefaultComparisonThreshold;// = 2.3f;
extern const float kMatchBoxDefaultTrimBeginThreshold;// = -27.0f;
extern const float kMatchBoxDefaultTrimEndThreshold;// = -40.0f;

extern NSString * const kMatchBoxDefaultComparisonThresholdKey;
extern NSString * const kMatchBoxDefaultTrimBeginThresholdKey;
extern NSString * const kMatchBoxDefaultTrimEndThresholdKey;

@interface SettingsViewController : UIViewController <UITableViewDataSource,
                                                      UITableViewDelegate> 

{
    
    UITableViewCell *_uiComparisonThresholdCell;
    UITableViewCell *_uiEndThresholdCell;
    UITableViewCell *_uiBeginThresholdCell;
    UILabel *_uiBeginThresholdLabel;
    UISlider *_uiBeginThresholdSlider;
    UISlider *_uiComparisonThresholdSlider;
    UILabel *_uiComparisonThresholdLabel;
    UILabel *_uiEndThresholdLabel;
    UISlider *_uiEndThresholdSlider;
    
    UITableViewCell *_uiRestoreDefaultsCell;
}

@property (nonatomic, retain) IBOutlet UITableViewCell *uiRestoreDefaultsCell;
@property (nonatomic, assign) id <SettingsViewControllerDelegate> delegate;
@property (nonatomic, retain) IBOutlet UITableViewCell *uiComparisonThresholdCell;
@property (nonatomic, retain) IBOutlet UITableViewCell *uiEndThresholdCell;
@property (nonatomic, retain) IBOutlet UITableViewCell *uiBeginThresholdCell;
@property (nonatomic, retain) IBOutlet UILabel *uiBeginThresholdLabel;
@property (nonatomic, retain) IBOutlet UISlider *uiBeginThresholdSlider;
@property (nonatomic, retain) IBOutlet UISlider *uiComparisonThresholdSlider;
@property (nonatomic, retain) IBOutlet UILabel *uiComparisonThresholdLabel;
@property (nonatomic, retain) IBOutlet UILabel *uiEndThresholdLabel;
@property (nonatomic, retain) IBOutlet UISlider *uiEndThresholdSlider;

- (IBAction)done:(id)sender;
- (IBAction)changeBeginThreshold:(id)sender;
- (IBAction)changeComparisonThreshold:(id)sender;
- (IBAction)changeEndThreshold:(id)sender;
- (IBAction)restoreDefaults:(id)sender;


- (void)updateAllUIElements;

+ (void)checkAndInitializeUserDefaults:(BOOL)restore;

@end


@protocol SettingsViewControllerDelegate
- (void)settingsViewControllerDidFinish:(SettingsViewController *)controller;
@end
