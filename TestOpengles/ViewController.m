//
//  ViewController.m
//  TestOpengles
//
//  Created by ky on 16/7/20.
//  Copyright © 2016年 yellfun. All rights reserved.
//

#import "ViewController.h"
#import "MyView.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
//    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
//    
//    if (!_context) {
//        
//        NSLog(@"opengles3初始化失败");
//    }
//    
//    GLKView *view = (GLKView *)self.view;
//    
//    view.context = _context;
//    
//    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
//    
//    [EAGLContext setCurrentContext:_context];
//    
//    glEnable(GL_DEPTH_TEST);
//    
//    glClearColor(0.1, 0.2, 0.3, 1);
    
    MyView *view = [[MyView alloc] initWithFrame:CGRectMake(0, 0, 300, 400)];
    
    [self.view addSubview:view];
}

@end
