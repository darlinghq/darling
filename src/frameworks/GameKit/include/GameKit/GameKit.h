/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GameKit_H_
#define _GameKit_H_

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

// Stub of GameKit. Darling has no Game Center service: the local player is
// never authenticated and every request fails with GKErrorNotSupported, as on
// macOS when Game Center is unavailable.
//
// Scoped to what a measured consumer binds against (2026-09-20: Chess.app, both
// slices), not to Apple's full API -- every entry point here fails, so an
// unreferenced declaration adds no behaviour and only implies support that does
// not exist. Protocols follow the same rule and none qualifies, since nothing is
// presented and no event is delivered; delegates are plain `id`.

extern NSString *const GKErrorDomain;

// Only the code this stub ever returns. Callers compare against their own SDK's
// constants, so the numeric value is the part that matters.
typedef NS_ENUM(NSInteger, GKErrorCode) {
	GKErrorNotSupported = 16,
};

// Declared as the type of the readonly `status` property; this stub never
// reports anything but Unknown.
typedef NS_ENUM(NSInteger, GKTurnBasedMatchStatus) {
	GKTurnBasedMatchStatusUnknown = 0,
};

// Received from callers via -setViewState:, so the case names are the vocabulary
// a caller writes against and the full set is kept.
typedef NS_ENUM(NSInteger, GKGameCenterViewControllerState) {
	GKGameCenterViewControllerStateDefault = -1,
	GKGameCenterViewControllerStateLeaderboards = 0,
	GKGameCenterViewControllerStateAchievements = 1,
	GKGameCenterViewControllerStateChallenges = 2,
	GKGameCenterViewControllerStateLocalPlayerProfile = 3,
	GKGameCenterViewControllerStateDashboard = 4,
};

@class GKTurnBasedParticipant;

@interface GKPlayer : NSObject {
@package
	NSString *_playerID;
	NSString *_displayName;
	NSString *_alias;
}
@property (readonly, retain) NSString *playerID;
@property (readonly, retain) NSString *displayName;
@property (readonly, retain) NSString *alias;
+ (void)loadPlayersForIdentifiers:(NSArray<NSString *> *)identifiers withCompletionHandler:(void (^)(NSArray<GKPlayer *> *players, NSError *error))completionHandler;
@end

@interface GKLocalPlayer : GKPlayer {
@package
	BOOL _authenticated;
	void (^_authenticateHandler)(NSViewController *viewController, NSError *error);
}
@property (readonly, getter=isAuthenticated) BOOL authenticated;
@property (copy) void (^authenticateHandler)(NSViewController *viewController, NSError *error);
+ (GKLocalPlayer *)localPlayer;
// Listeners are accepted and dropped: nothing in this stub ever delivers an event.
- (void)registerListener:(id)listener;
@end

@interface GKAchievement : NSObject {
@package
	NSString *_identifier;
	double _percentComplete;
	BOOL _showsCompletionBanner;
	GKPlayer *_player;
}
@property (readonly, copy) NSString *identifier;
@property (assign) double percentComplete;
@property (assign) BOOL showsCompletionBanner;
// Chess sends a bare `player`; the receiver is undeterminable from the binary.
// Always nil, but omitting it would raise doesNotRecognizeSelector:.
@property (readonly, retain) GKPlayer *player;
- (instancetype)initWithIdentifier:(NSString *)identifier;
+ (void)loadAchievementsWithCompletionHandler:(void (^)(NSArray<GKAchievement *> *achievements, NSError *error))completionHandler;
+ (void)reportAchievements:(NSArray<GKAchievement *> *)achievements withCompletionHandler:(void (^)(NSError *error))completionHandler;
@end

@interface GKMatchRequest : NSObject {
@package
	NSUInteger _minPlayers;
	NSUInteger _maxPlayers;
	NSUInteger _playerGroup;
	uint32_t _playerAttributes;
	NSArray<GKPlayer *> *_recipients;
}
@property (assign) NSUInteger minPlayers;
@property (assign) NSUInteger maxPlayers;
@property (assign) NSUInteger playerGroup;
@property (assign) uint32_t playerAttributes;
@property (retain) NSArray<GKPlayer *> *recipients;
@end

@interface GKTurnBasedMatch : NSObject {
@package
	NSString *_matchID;
	NSArray<GKTurnBasedParticipant *> *_participants;
	GKTurnBasedMatchStatus _status;
	GKTurnBasedParticipant *_currentParticipant;
	NSData *_matchData;
}
@property (readonly, retain) NSString *matchID;
@property (readonly, retain) NSArray<GKTurnBasedParticipant *> *participants;
@property (readonly) GKTurnBasedMatchStatus status;
@property (readonly, retain) GKTurnBasedParticipant *currentParticipant;
@property (readonly, retain) NSData *matchData;
+ (void)loadMatchesWithCompletionHandler:(void (^)(NSArray<GKTurnBasedMatch *> *matches, NSError *error))completionHandler;
- (void)loadMatchDataWithCompletionHandler:(void (^)(NSData *matchData, NSError *error))completionHandler;
- (void)endTurnWithNextParticipants:(NSArray<GKTurnBasedParticipant *> *)nextParticipants turnTimeout:(NSTimeInterval)timeout matchData:(NSData *)matchData completionHandler:(void (^)(NSError *error))completionHandler;
- (void)endMatchInTurnWithMatchData:(NSData *)matchData completionHandler:(void (^)(NSError *error))completionHandler;
@end

@interface GKGameCenterViewController : NSViewController {
@package
	id _gameCenterDelegate;
	GKGameCenterViewControllerState _viewState;
}
@property (assign) id gameCenterDelegate;
@property (assign) GKGameCenterViewControllerState viewState;
@end

@interface GKTurnBasedMatchmakerViewController : NSViewController {
@package
	id _turnBasedMatchmakerDelegate;
	BOOL _showExistingMatches;
}
@property (assign) id turnBasedMatchmakerDelegate;
@property (assign) BOOL showExistingMatches;
- (instancetype)initWithMatchRequest:(GKMatchRequest *)request;
@end

// On macOS Game Center panels are hosted by GKDialogController rather than
// presented by the app itself.
@interface GKDialogController : NSObject {
@package
	NSWindow *_parentWindow;
}
+ (GKDialogController *)sharedDialogController;
- (BOOL)presentViewController:(NSViewController *)viewController;
- (void)dismiss:(id)sender;
@end

#endif
