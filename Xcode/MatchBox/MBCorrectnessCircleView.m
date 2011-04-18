//Copyright (c) 2011 Sebastian Böhm sebastian@sometimesfood.org
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
#import "MBCorrectnessCircleView.h"


@implementation MBCorrectnessCircleView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self initialize];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self) {
        [self initialize];
    }
    return self;    
}

- (void)initialize
{         
    self.layer.cornerRadius = 15;
    self.layer.backgroundColor = [[UIColor greenColor] CGColor];
    self.layer.shadowOpacity = 0.4f;
    CGSize offset;
    offset.width = 0;
    offset.height = 3;
    self.layer.shadowOffset = offset;
//    self.layer.borderWidth = 4;
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
//- (void)drawRect:(CGRect)rect
//{
//    CGContextRef context = UIGraphicsGetCurrentContext();
//    CGContextSaveGState(context);
//    CGSize offset;
//    offset.width = 2;
//    offset.height = 2;
//    CGContextSetShadow(context, offset,2.0f);
////    [[UIColor whiteColor] set];
//    UIColor * clr = [UIColor colorWithCGColor:self.layer.borderColor];
//    [clr set];
//    [circle_ fill];
//    
//    CGContextRestoreGState(context);
//}


- (void)dealloc
{
    [super dealloc];
}

@end
