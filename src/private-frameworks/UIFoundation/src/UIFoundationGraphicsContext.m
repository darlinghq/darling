/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import <UIFoundation/UIFoundationGraphicsContext.h>

// Held as a plain pointer so the atomic builtins accept it; retained.
static void *currentProvider;

void _UIFoundationSetGraphicsContextProvider(id<_UIFoundationGraphicsContextProvider> provider)
{
    [provider retain];
    id previous = (id) __atomic_exchange_n(&currentProvider, (void *) provider, __ATOMIC_ACQ_REL);
    [previous release];
}

id<_UIFoundationGraphicsContextProvider> _UIFoundationGraphicsContextProvider(void)
{
    return (id<_UIFoundationGraphicsContextProvider>) __atomic_load_n(&currentProvider, __ATOMIC_ACQUIRE);
}

CGContextRef _UIFoundationCurrentCGContext(void)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    return provider ? [provider currentCGContext] : NULL;
}

BOOL _UIFoundationCurrentContextIsFlipped(void)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    return provider ? [provider currentContextIsFlipped] : NO;
}

BOOL _UIFoundationCurrentContextIsDrawingToScreen(void)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    return provider ? [provider currentContextIsDrawingToScreen] : NO;
}

id _UIFoundationTextColorForRole(_UIFoundationTextColorRole role)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    if (![provider respondsToSelector: @selector(textColorForRole:)])
        return nil;
    return [provider textColorForRole: role];
}

id _UIFoundationDefaultTextFont(void)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    if (![provider respondsToSelector: @selector(defaultTextFont)])
        return nil;
    return [provider defaultTextFont];
}

id _UIFoundationImageWithData(NSData *data)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    if (data == nil || ![provider respondsToSelector: @selector(imageWithData:)])
        return nil;
    return [provider imageWithData: data];
}

id _UIFoundationNewTextAttachmentCell(void)
{
    id<_UIFoundationGraphicsContextProvider> provider = _UIFoundationGraphicsContextProvider();
    if (![provider respondsToSelector: @selector(newTextAttachmentCell)])
        return nil;
    return [provider newTextAttachmentCell];
}
