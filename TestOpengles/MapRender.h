//
//  MapRender.h
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface MapRender : NSObject

- (id)initRender:(NSString*)mapFile;

- (void)setSize:(CGFloat)width height:(CGFloat)height;

- (void)zoom:(CGFloat)zoomValue;

- (void)rotate:(CGFloat)angle;

- (void)pan:(CGFloat)x y:(CGFloat)y;

/**
 * 传入触摸事件
 * action 的取值:
 * TOUCH_ACTION_DOWN 单点按下
 * TOUCH_ACTION_MOVE 单点/多点移动
 * TOUCH_ACTION_UP   单点抬起
 * TOUCH_MULTI_DOWN  多点按下
 * TOUCH_MULTI_UP    多点抬起
 *
 * 单点事件的时候, 只需填充x0, y0
 * 多点事件时, x0, y0, x1, y1都要填充, 目前也只支持两点手势
 * 如果有必要再扩展
 */
- (void)handleTouch:(int)action firstFinger:(CGPoint)firstFinger secondFinger:(CGPoint)secondFinger;

- (void)draw;

@end
