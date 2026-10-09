#ifndef DARLING_UIKIT_COMPOSITIONAL_LAYOUT_H
#define DARLING_UIKIT_COMPOSITIONAL_LAYOUT_H

#import <Foundation/NSObject.h>
#import <CoreGraphics/CGBase.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSCollectionLayoutDimension : NSObject <NSCopying>
+ (instancetype)absoluteDimension:(CGFloat)value;
+ (instancetype)estimatedDimension:(CGFloat)value;
+ (instancetype)fractionalWidthDimension:(CGFloat)value;
+ (instancetype)fractionalHeightDimension:(CGFloat)value;
@property(nonatomic, readonly) CGFloat dimension;
@property(nonatomic, readonly) BOOL isAbsolute;
@property(nonatomic, readonly) BOOL isEstimated;
@property(nonatomic, readonly) BOOL isFractionalWidth;
@property(nonatomic, readonly) BOOL isFractionalHeight;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface NSCollectionLayoutSize : NSObject <NSCopying>
+ (instancetype)sizeWithWidthDimension:(NSCollectionLayoutDimension *)width
                      heightDimension:(NSCollectionLayoutDimension *)height;
@property(nonatomic, readonly) NSCollectionLayoutDimension *widthDimension;
@property(nonatomic, readonly) NSCollectionLayoutDimension *heightDimension;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface NSCollectionLayoutSpacing : NSObject <NSCopying>
+ (instancetype)fixedSpacing:(CGFloat)value;
+ (instancetype)flexibleSpacing:(CGFloat)value;
@property(nonatomic, readonly) CGFloat spacing;
@property(nonatomic, readonly) BOOL isFixedSpacing;
@property(nonatomic, readonly) BOOL isFlexibleSpacing;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

@interface NSCollectionLayoutEdgeSpacing : NSObject <NSCopying>
+ (instancetype)spacingForLeading:(nullable NSCollectionLayoutSpacing *)leading
                             top:(nullable NSCollectionLayoutSpacing *)top
                        trailing:(nullable NSCollectionLayoutSpacing *)trailing
                          bottom:(nullable NSCollectionLayoutSpacing *)bottom;
@property(nonatomic, readonly, nullable) NSCollectionLayoutSpacing *leading;
@property(nonatomic, readonly, nullable) NSCollectionLayoutSpacing *top;
@property(nonatomic, readonly, nullable) NSCollectionLayoutSpacing *trailing;
@property(nonatomic, readonly, nullable) NSCollectionLayoutSpacing *bottom;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
#endif
