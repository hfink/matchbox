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

const float kMatchBoxDefaultComparisonThreshold = 3.09f;
const float kMatchBoxDefaultTrimBeginThreshold = -27.0f;
const float kMatchBoxDefaultTrimEndThreshold = -40.0f;

NSString * const kMatchBoxDefaultComparisonThresholdKey = @"MBComparison";
NSString * const kMatchBoxDefaultTrimBeginThresholdKey = @"MBBeginThresh";
NSString * const kMatchBoxDefaultTrimEndThresholdKey = @"MBEndThresh";


@implementation SettingsViewController

@synthesize uiRestoreDefaultsCell = _uiRestoreDefaultsCell;
@synthesize delegate=_delegate;
@synthesize uiComparisonThresholdCell = _uiComparisonThresholdCell;
@synthesize uiEndThresholdCell = _uiEndThresholdCell;
@synthesize uiBeginThresholdCell = _uiBeginThresholdCell;
@synthesize uiBeginThresholdLabel = _uiBeginThresholdLabel;
@synthesize uiBeginThresholdSlider = _uiBeginThresholdSlider;
@synthesize uiComparisonThresholdSlider = _uiComparisonThresholdSlider;
@synthesize uiComparisonThresholdLabel = _uiComparisonThresholdLabel;
@synthesize uiEndThresholdLabel = _uiEndThresholdLabel;
@synthesize uiEndThresholdSlider = _uiEndThresholdSlider;

- (void)dealloc
{
    [_uiComparisonThresholdCell release];
    [_uiComparisonThresholdSlider release];
    [_uiComparisonThresholdLabel release];    
    
    [_uiEndThresholdCell release];
    [_uiEndThresholdLabel release];    
    [_uiEndThresholdSlider release];    
    
    [_uiBeginThresholdCell release];
    [_uiBeginThresholdLabel release];
    [_uiBeginThresholdSlider release];

    [_uiRestoreDefaultsCell release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

+ (void)checkAndInitializeUserDefaults:(BOOL)restore
{
    
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    
    if (restore || ![d objectForKey:kMatchBoxDefaultComparisonThresholdKey])
    {
        [d setFloat:kMatchBoxDefaultComparisonThreshold 
             forKey:kMatchBoxDefaultComparisonThresholdKey];
    }
    
    if (restore || ![d objectForKey:kMatchBoxDefaultTrimBeginThresholdKey])
    {
        [d setFloat:kMatchBoxDefaultTrimBeginThreshold 
             forKey:kMatchBoxDefaultTrimBeginThresholdKey];
    }    
    
    if (restore || ![d objectForKey:kMatchBoxDefaultTrimEndThresholdKey])
    {
        [d setFloat:kMatchBoxDefaultTrimEndThreshold 
             forKey:kMatchBoxDefaultTrimEndThresholdKey];
    }        
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor viewFlipsideBackgroundColor];  
    
    [SettingsViewController checkAndInitializeUserDefaults:NO];
    
    [self updateAllUIElements];
}

- (void)updateAllUIElements
{
    //Default values
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    
    float compThresh = [d floatForKey:kMatchBoxDefaultComparisonThresholdKey];
    float beginThresh = [d floatForKey:kMatchBoxDefaultTrimBeginThresholdKey];
    float endThresh = [d floatForKey:kMatchBoxDefaultTrimEndThresholdKey];
    
    [[self uiComparisonThresholdSlider] setValue:compThresh];
    [[self uiBeginThresholdSlider] setValue:beginThresh];
    [[self uiEndThresholdSlider] setValue:endThresh]; 
    
    NSString * str = 
    [NSString stringWithFormat:@"%.2f",compThresh];
    
    [[self uiComparisonThresholdLabel] setText:str];            
    
    str = [NSString stringWithFormat:@"%.1f db",beginThresh];
    [[self uiBeginThresholdLabel] setText:str];    
    
    str = [NSString stringWithFormat:@"%.1f db",endThresh];
    [[self uiEndThresholdLabel] setText:str];    
}

- (void)viewDidUnload
{
    [self setUiComparisonThresholdCell:nil];
    [self setUiComparisonThresholdLabel:nil];    
    [self setUiComparisonThresholdSlider:nil];    
        
    [self setUiBeginThresholdCell:nil];
    [self setUiBeginThresholdLabel:nil];
    [self setUiBeginThresholdSlider:nil];
    
    [self setUiEndThresholdCell:nil];
    [self setUiEndThresholdLabel:nil];   
    [self setUiEndThresholdSlider:nil];    
    [self setUiRestoreDefaultsCell:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

#pragma mark - Actions

- (IBAction)done:(id)sender
{
    [self.delegate settingsViewControllerDidFinish:self];
}

- (IBAction)changeBeginThreshold:(id)sender {
    UISlider * slider = (UISlider*)sender;
    
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    [d setFloat:[slider value] forKey:kMatchBoxDefaultTrimBeginThresholdKey];
    
    NSString * str = 
    [NSString stringWithFormat:@"%.1f db",[slider value]];
    
    [[self uiBeginThresholdLabel] setText:str];    
}

- (IBAction)changeComparisonThreshold:(id)sender {
    UISlider * slider = (UISlider*)sender;
    
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    [d setFloat:[slider value] forKey:kMatchBoxDefaultComparisonThresholdKey];
    
    NSString * str = [NSString stringWithFormat:@"%.2f",[slider value]];
    
    [[self uiComparisonThresholdLabel] setText:str];        
}

- (IBAction)changeEndThreshold:(id)sender {
    UISlider * slider = (UISlider*)sender;
    
    NSUserDefaults * d = [NSUserDefaults standardUserDefaults];
    [d setFloat:[slider value] forKey:kMatchBoxDefaultTrimEndThresholdKey];
    
    NSString * str = [NSString stringWithFormat:@"%.1f db",[slider value]];
    
    [[self uiEndThresholdLabel] setText:str];       
}

- (IBAction)restoreDefaults:(id)sender {
    
    [SettingsViewController checkAndInitializeUserDefaults:YES];
    [self updateAllUIElements];
    
}

#pragma mark TableViewData source methods
- (UITableViewCell *)tableView:(UITableView *)tableView 
         cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ([indexPath section] == 0)
        return _uiComparisonThresholdCell;
    else if ([indexPath section] == 1)
        return _uiBeginThresholdCell;
    else if ([indexPath section] == 2)
        return _uiEndThresholdCell;
    else if ([indexPath section] == 3)
        return _uiRestoreDefaultsCell;    
    
    return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 4;
}

- (NSInteger)tableView:(UITableView *)tableView 
    numberOfRowsInSection:(NSInteger)section {
    return 1;
}

- (NSString *)tableView:(UITableView *)tableView 
    titleForHeaderInSection:(NSInteger)section
{
    if (section == 0)
        return @"Comparison Threshold";
    else if (section == 1)
        return @"Begin Trimming Threshold";
    else if (section == 2)
        return @"End Trimming Threshold";
    
    return nil;
}

@end
