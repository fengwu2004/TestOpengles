//
//  GLView.h
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import <GLKit/GLKit.h>
#import <UIKit/UIKit.h>

@interface MapViewController : GLKViewController

@property (nonatomic, retain) EAGLContext *context;

- (void)zoom;

- (void)rotate;

@end
