/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef _COMPONENT_DISPATCH_H
#define _COMPONENT_DISPATCH_H
#include "AudioComponentManager.h"
#include <CarbonCore/Components.h>

template <typename First, typename... Rest>
void assignParams(ComponentParameters* cp, size_t totalArgs, size_t argumentIndex, First arg, Rest... rest)
{
#if __LP64__
    cp->params[argumentIndex] = long(arg);
#else
    cp->params[totalArgs - argumentIndex - 1] = long(arg);
#endif

    if constexpr (sizeof...(Rest) > 0)
        assignParams(cp, totalArgs, argumentIndex+1, rest...);
}

template <typename ...Args>
OSStatus dispatchCall(AudioComponentInstance inUnit, SInt16 sel, Args... args)
{
    if (AudioComponentManager::isOurInstance(inUnit))
    {
        AudioComponentPlugInInterface* iface = AudioComponentManager::instance()->instanceInterface(inUnit);
        AudioComponentMethod method = iface->Lookup(sel);
        if (method != nullptr)
            return method(iface, args...);
        else
            return badComponentSelector;
    }
    else
    {
        ComponentParameters* cp = (ComponentParameters*) alloca(sizeof(ComponentParameters) + sizeof...(Args) * sizeof(long));
        constexpr size_t totalArgs = sizeof...(Args)+1;

        assignParams(cp, totalArgs, 0, inUnit);

        if constexpr (totalArgs > 1)
            assignParams(cp, totalArgs, 1, args...);

        cp->paramSize = totalArgs * sizeof(long);
        cp->what = sel;
        cp->flags = 0;

        return CallComponentDispatch(cp);
    }
}

#endif

