//
//  IPAppDelegate.h
//  IP
//
//  Created by Heinrich Fink on 5/18/11.
//  Copyright 2011 Heinrich Fink. All rights reserved.
//

#import <UIKit/UIKit.h>

@class IPViewController;

@interface IPAppDelegate : NSObject <UIApplicationDelegate> {

}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@property (nonatomic, retain) IBOutlet IPViewController *viewController;

@end
