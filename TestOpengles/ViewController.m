//
//  ViewController.m
//  TestOpengles
//
//  Created by ky on 16/7/20.
//  Copyright © 2016年 yellfun. All rights reserved.
//

#import "ViewController.h"
#import "MyView.h"
#import "GLView.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    GLView *view = [[GLView alloc] initWithFrame:CGRectMake(0, 0, 300, 400)];
    
    [self.view addSubview:view];
    
    NSString *glesPath = [[NSBundle mainBundle] pathForResource:@"font" ofType:@"ttf" inDirectory:@"glres"];
    
    NSString *str = [glesPath stringByDeletingLastPathComponent];

    
}

@end
