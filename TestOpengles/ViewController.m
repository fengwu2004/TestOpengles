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

@property (nonatomic, retain) GLView *glesMap;

@end

@implementation ViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    _glesMap = [[GLView alloc] initWithFrame:CGRectMake(0, 0, 300, 400)];
    
    [self.view addSubview:_glesMap];
}

- (IBAction)onZoom:(id)sender {
    
    [_glesMap zoom];
}

@end
