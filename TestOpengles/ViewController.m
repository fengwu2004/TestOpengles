//
//  ViewController.m
//  TestOpengles
//
//  Created by ky on 16/7/20.
//  Copyright © 2016年 yellfun. All rights reserved.
//

#import "ViewController.h"
#import "MyView.h"
#import "MapView.h"

@interface ViewController ()

@property (nonatomic, retain) MapView *glesMap;

@end

@implementation ViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    _glesMap = [[MapView alloc] initWithFrame:self.view.bounds];
    
    [self.view addSubview:_glesMap];
    
    [self.view sendSubviewToBack:_glesMap];
}

- (IBAction)onZoom:(id)sender {
    
    [_glesMap zoom];
}

- (IBAction)onRotate:(id)sender {
    
    [_glesMap rotate];
}

@end
