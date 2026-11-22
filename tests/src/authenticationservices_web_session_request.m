// CFLAGS: -fobjc-arc -framework AuthenticationServices -framework Foundation
// Copyright (C) 2026 Darling Team
//
// ASWebAuthenticationSessionRequest and ASWebAuthenticationSessionWebBrowserSessionManager, the browser
// side of a web authentication session, behave as the public documentation describes:
//   https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionrequest
//   https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionrequestdelegate
//   https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionwebbrowsersessionmanager
//   https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionwebbrowsersessionhandling
// The documentation does not define the archive keys of a request. The tests that build a request name the
// keys this implementation chose; they are not a statement about macOS. The behaviours the documentation
// leaves open (decoding without a URL or UUID fails, a copy is a new object, the delegate is weak and is
// called on the calling thread without dropping repeated calls, the manager is a singleton and keeps its
// handler) were observed on macOS 26.7.3 (black-box observation) and are pinned here.
// Exits non-zero on failure. No display needed.
#import <Foundation/Foundation.h>
#import <AuthenticationServices/ASWebAuthenticationSession.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequest.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequestDelegate.h>
#import <AuthenticationServices/ASWebAuthenticationSessionWebBrowserSessionHandling.h>
#import <AuthenticationServices/ASWebAuthenticationSessionWebBrowserSessionManager.h>
#include <stdio.h>

// The implementation is 64-bit only; the 32-bit slice keeps the stub.
#if !__LP64__
int main(void) {
	printf("SKIP: web authentication session request is 64-bit only\n");
	return 0;
}
#else

static int failures;

static void expect(BOOL ok, const char *label) {
	printf("%s %s\n", ok ? "PASS" : "FAIL", label);
	failures += !ok;
}

// The archive keys below are this implementation's own.
static ASWebAuthenticationSessionRequest *makeRequest(NSURL *url, NSUUID *uuid, BOOL ephemeral,
		NSDictionary *headers, NSString *scheme) {
	NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
	[archiver encodeObject:url forKey:@"requestURL"];
	[archiver encodeObject:uuid forKey:@"requestUUID"];
	[archiver encodeBool:ephemeral forKey:@"ephemeralSession"];
	[archiver encodeObject:headers forKey:@"extraHeaders"];
	[archiver encodeObject:scheme forKey:@"callbackScheme"];
	[archiver finishEncoding];
	NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData:archiver.encodedData error:NULL];
	ASWebAuthenticationSessionRequest *request = [[ASWebAuthenticationSessionRequest alloc] initWithCoder:unarchiver];
	[unarchiver finishDecoding];
	return request;
}

@interface RecordingDelegate : NSObject <ASWebAuthenticationSessionRequestDelegate>
@property (strong) ASWebAuthenticationSessionRequest *completedRequest;
@property (strong) NSURL *callbackURL;
@property (strong) ASWebAuthenticationSessionRequest *canceledRequest;
@property (strong) NSError *error;
@end

@implementation RecordingDelegate
- (void)authenticationSessionRequest:(ASWebAuthenticationSessionRequest *)request didCompleteWithCallbackURL:(NSURL *)url {
	self.completedRequest = request;
	self.callbackURL = url;
}
- (void)authenticationSessionRequest:(ASWebAuthenticationSessionRequest *)request didCancelWithError:(NSError *)error {
	self.canceledRequest = request;
	self.error = error;
}
@end

// The delegate methods are optional.
@interface SilentDelegate : NSObject <ASWebAuthenticationSessionRequestDelegate>
@end
@implementation SilentDelegate
@end

@interface RecordingHandler : NSObject <ASWebAuthenticationSessionWebBrowserSessionHandling>
@property (strong) ASWebAuthenticationSessionRequest *began;
@property (strong) ASWebAuthenticationSessionRequest *canceled;
@end

@implementation RecordingHandler
- (void)beginHandlingWebAuthenticationSessionRequest:(ASWebAuthenticationSessionRequest *)request { self.began = request; }
- (void)cancelWebAuthenticationSessionRequest:(ASWebAuthenticationSessionRequest *)request { self.canceled = request; }
@end

int main(void) {
	@autoreleasepool {
		NSURL *url = [NSURL URLWithString:@"https://idp.example.com/authorize?state=1"];
		NSURL *callbackURL = [NSURL URLWithString:@"myscheme://done?code=abc"];
		NSUUID *uuid = [NSUUID UUID];
		NSDictionary *headers = @{ @"X-Test": @"1" };

		// Interpreting a request: the properties report what the request was created with.
		ASWebAuthenticationSessionRequest *request = makeRequest(url, uuid, YES, headers, @"myscheme");
		expect(request != nil, "request decodes from a keyed archive");
		expect([request.URL isEqual:url], "URL");
		expect([request.UUID isEqual:uuid], "UUID");
		expect(request.shouldUseEphemeralSession, "shouldUseEphemeralSession");
		expect([request.additionalHeaderFields isEqualToDictionary:headers], "additionalHeaderFields");
		expect([request.callbackURLScheme isEqualToString:@"myscheme"], "callbackURLScheme");

		ASWebAuthenticationSessionRequest *plain = makeRequest(url, uuid, NO, nil, nil);
		expect(!plain.shouldUseEphemeralSession, "shouldUseEphemeralSession NO");
		expect(plain.additionalHeaderFields == nil, "additionalHeaderFields is nullable");
		expect(plain.callbackURLScheme == nil, "callbackURLScheme is nullable");

		// An archive without a URL or UUID does not decode (init?(coder:) is failable; macOS 26.7.3 agrees).
		expect(makeRequest(nil, uuid, NO, nil, nil) == nil, "no URL fails to decode");
		expect(makeRequest(url, nil, NO, nil, nil) == nil, "no UUID fails to decode");
		expect([ASWebAuthenticationSessionRequest supportsSecureCoding], "supportsSecureCoding");

		// Finishing a request informs the delegate before the method returns (macOS 26.7.3 agrees).
		RecordingDelegate *delegate = [RecordingDelegate new];
		request.delegate = delegate;
		expect(request.delegate == delegate, "delegate getter");
		[request completeWithCallbackURL:callbackURL];
		expect(delegate.completedRequest == request && [delegate.callbackURL isEqual:callbackURL],
			"completeWithCallbackURL: reaches the delegate with the request and the URL");
		expect(delegate.canceledRequest == nil, "completion is not reported as a cancellation");

		RecordingDelegate *cancelDelegate = [RecordingDelegate new];
		plain.delegate = cancelDelegate;
		NSError *error = [NSError errorWithDomain:ASWebAuthenticationSessionErrorDomain
		                                     code:ASWebAuthenticationSessionErrorCodeCanceledLogin
		                                 userInfo:nil];
		[plain cancelWithError:error];
		expect(cancelDelegate.canceledRequest == plain && cancelDelegate.error == error,
			"cancelWithError: reaches the delegate with the request and the error");
		expect(cancelDelegate.completedRequest == nil, "cancellation is not reported as a completion");

		// Repeated and crossed calls are all forwarded: macOS 26.7.3 has no once-only rule, and forwards a nil
		// callback URL as nil.
		RecordingDelegate *repeat = [RecordingDelegate new];
		ASWebAuthenticationSessionRequest *again = makeRequest(url, uuid, NO, nil, nil);
		again.delegate = repeat;
		[again completeWithCallbackURL:callbackURL];
		repeat.callbackURL = nil;
		[again completeWithCallbackURL:callbackURL];
		expect([repeat.callbackURL isEqual:callbackURL], "a second completion reaches the delegate");
		repeat.error = nil;
		[again cancelWithError:error];
		expect(repeat.error == error, "a cancellation after a completion reaches the delegate");
		repeat.callbackURL = callbackURL;
		[again completeWithCallbackURL:nil];
		expect(repeat.callbackURL == nil, "a completion after a cancellation reaches the delegate, with a nil URL as nil");

		// The delegate methods are optional and the delegate may be nil.
		ASWebAuthenticationSessionRequest *quiet = makeRequest(url, uuid, NO, nil, nil);
		[quiet completeWithCallbackURL:callbackURL];
		[quiet cancelWithError:error];
		SilentDelegate *silent = [SilentDelegate new];
		quiet.delegate = silent;
		[quiet completeWithCallbackURL:callbackURL];
		[quiet cancelWithError:error];
		expect(YES, "completion and cancellation without a responding delegate do not crash");

		// The delegate property is weak.
		ASWebAuthenticationSessionRequest *weakHolder = makeRequest(url, uuid, NO, nil, nil);
		@autoreleasepool {
			RecordingDelegate *shortLived = [RecordingDelegate new];
			weakHolder.delegate = shortLived;
		}
		expect(weakHolder.delegate == nil, "delegate is not retained");

		// NSCopying: a copy is a new object (macOS 26.7.3 agrees); the documentation lists the conformance only.
		ASWebAuthenticationSessionRequest *copy = [request copy];
		expect(copy != nil && copy != request, "copy is a new object");
		expect([copy.URL isEqual:url] && [copy.UUID isEqual:uuid] && copy.shouldUseEphemeralSession &&
		       [copy.additionalHeaderFields isEqualToDictionary:headers] &&
		       [copy.callbackURLScheme isEqualToString:@"myscheme"], "copy keeps the request's values");

		// The session manager is a singleton that holds the browser's handler.
		ASWebAuthenticationSessionWebBrowserSessionManager *manager = ASWebAuthenticationSessionWebBrowserSessionManager.sharedManager;
		expect(manager != nil && manager == ASWebAuthenticationSessionWebBrowserSessionManager.sharedManager,
			"sharedManager returns one shared instance");
		RecordingHandler *handler = [RecordingHandler new];
		manager.sessionHandler = handler;
		expect(manager.sessionHandler == handler, "sessionHandler getter returns what was set");
		[manager.sessionHandler beginHandlingWebAuthenticationSessionRequest:request];
		[manager.sessionHandler cancelWebAuthenticationSessionRequest:plain];
		expect(handler.began == request && handler.canceled == plain, "handler receives begin and cancel requests");
		// Nothing in Darling opens a browser for Authentication Services, so this is NO.
		expect(!manager.wasLaunchedByAuthenticationServices, "wasLaunchedByAuthenticationServices is NO");
	}
	if (failures == 0)
		printf("PASS: web authentication session request and browser session manager\n");
	return failures != 0;
}

#endif
