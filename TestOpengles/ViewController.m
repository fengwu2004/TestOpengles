//
//  ViewController.m
//  TestOpengles
//
//  Created by ky on 16/7/20.
//  Copyright © 2016年 yellfun. All rights reserved.
//

#import "ViewController.h"
#import "MyView.h"
#import "MapViewController.h"

@interface ViewController ()

@property (nonatomic, retain) MapViewController *glesMap;

@end

@implementation ViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    _glesMap = [[MapViewController alloc] init];
    
    [self.view addSubview:_glesMap.view];
    
    [self addChildViewController:_glesMap];
    
    [_glesMap didMoveToParentViewController:self];
    
    _glesMap.view.frame = self.view.bounds;
    
    [self.view sendSubviewToBack:_glesMap.view];
}

- (IBAction)onZoom:(id)sender {
    
    [_glesMap zoom];
}

- (IBAction)onRotate:(id)sender {
    
    [_glesMap rotate];
}

@end
