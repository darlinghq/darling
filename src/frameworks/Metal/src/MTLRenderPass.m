#import <Metal/MTLRenderPass.m>

MTLClearColor MTLClearColorMake(double red, double green, double blue, double alpha) {
    MTLClearColor item = { red, green, blue, alpha, };
    return item;
}
