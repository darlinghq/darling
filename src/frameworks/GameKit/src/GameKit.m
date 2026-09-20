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

#import <GameKit/GameKit.h>
#include <dispatch/dispatch.h>

NSString *const GKErrorDomain = @"com.apple.GameKit";

static NSError *gameCenterUnavailableError(void)
{
	return [NSError errorWithDomain:GKErrorDomain
	                           code:GKErrorNotSupported
	                       userInfo:@{ NSLocalizedDescriptionKey: @"Game Center is not available in Darling" }];
}

// Completion handlers run asynchronously, as they do on macOS.
static void completeLater(void (^handler)(id, NSError *))
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		handler(nil, gameCenterUnavailableError());
		[handler release];
	});
}

static void completeErrorLater(void (^handler)(NSError *))
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		handler(gameCenterUnavailableError());
		[handler release];
	});
}

@implementation GKPlayer

@synthesize playerID = _playerID, displayName = _displayName, alias = _alias;

+ (void)loadPlayersForIdentifiers:(NSArray<NSString *> *)identifiers withCompletionHandler:(void (^)(NSArray<GKPlayer *> *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

- (void)dealloc
{
	[_playerID release];
	[_displayName release];
	[_alias release];
	[super dealloc];
}

@end

@implementation GKLocalPlayer
{
	void (^_authenticateHandler)(NSViewController *, NSError *);
}

+ (GKLocalPlayer *)localPlayer
{
	static GKLocalPlayer *sLocalPlayer = nil;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		sLocalPlayer = [[GKLocalPlayer alloc] init];
	});
	return sLocalPlayer;
}

- (BOOL)isAuthenticated { return NO; }

- (void (^)(NSViewController *, NSError *))authenticateHandler
{
	return _authenticateHandler;
}

// Apps drive their whole Game Center flow from this handler and wait forever if
// it never fires. No sign-in UI exists, so it gets nil and an error.
- (void)setAuthenticateHandler:(void (^)(NSViewController *, NSError *))handler
{
	// Copy before releasing: a caller chaining onto the previous handler has
	// captured it without retaining, so releasing first can free a block the
	// new one is about to retain.
	void (^old)(NSViewController *, NSError *) = _authenticateHandler;
	_authenticateHandler = [handler copy];
	[old release];

	if (_authenticateHandler == nil)
		return;

	// The dispatch capture already retains this; the explicit pair keeps the
	// lifetime requirement visible if the dispatch is ever changed.
	void (^storedHandler)(NSViewController *, NSError *) = [_authenticateHandler retain];
	dispatch_async(dispatch_get_main_queue(), ^{
		storedHandler(nil, gameCenterUnavailableError());
		[storedHandler release];
	});
}

- (void)registerListener:(id)listener {}

- (void)dealloc
{
	[_authenticateHandler release];
	[super dealloc];
}

@end

@implementation GKAchievement

@synthesize identifier = _identifier, percentComplete = _percentComplete,
	showsCompletionBanner = _showsCompletionBanner, player = _player;

- (instancetype)initWithIdentifier:(NSString *)identifier
{
	self = [super init];
	if (self)
		_identifier = [identifier copy];
	return self;
}

+ (void)loadAchievementsWithCompletionHandler:(void (^)(NSArray<GKAchievement *> *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

+ (void)reportAchievements:(NSArray<GKAchievement *> *)achievements withCompletionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)dealloc
{
	[_identifier release];
	[_player release];
	[super dealloc];
}

@end

@implementation GKMatchRequest

@synthesize minPlayers = _minPlayers, maxPlayers = _maxPlayers, playerGroup = _playerGroup,
	playerAttributes = _playerAttributes, recipients = _recipients;

- (void)dealloc
{
	[_recipients release];
	[super dealloc];
}

@end

@implementation GKTurnBasedMatch

@synthesize matchID = _matchID, participants = _participants, status = _status,
	currentParticipant = _currentParticipant, matchData = _matchData;

+ (void)loadMatchesWithCompletionHandler:(void (^)(NSArray<GKTurnBasedMatch *> *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

- (void)loadMatchDataWithCompletionHandler:(void (^)(NSData *, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *))completionHandler);
}

- (void)endTurnWithNextParticipants:(NSArray<GKTurnBasedParticipant *> *)nextParticipants turnTimeout:(NSTimeInterval)timeout matchData:(NSData *)matchData completionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)endMatchInTurnWithMatchData:(NSData *)matchData completionHandler:(void (^)(NSError *))completionHandler
{
	completeErrorLater(completionHandler);
}

- (void)dealloc
{
	[_matchID release];
	[_participants release];
	[_currentParticipant release];
	[_matchData release];
	[super dealloc];
}

@end

@implementation GKGameCenterViewController

@synthesize gameCenterDelegate = _gameCenterDelegate, viewState = _viewState;

@end

@implementation GKTurnBasedMatchmakerViewController

@synthesize turnBasedMatchmakerDelegate = _turnBasedMatchmakerDelegate,
	showExistingMatches = _showExistingMatches;

- (instancetype)initWithMatchRequest:(GKMatchRequest *)request
{
	return [super init];
}

@end

@implementation GKDialogController

+ (GKDialogController *)sharedDialogController
{
	static GKDialogController *sController = nil;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		sController = [[GKDialogController alloc] init];
	});
	return sController;
}

// There is nothing to present. Returning NO tells the caller the panel did not
// open, rather than leaving it waiting for a dismissal that never comes.
- (BOOL)presentViewController:(NSViewController *)viewController
{
	return NO;
}

- (void)dismiss:(id)sender {}

@end
