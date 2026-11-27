/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef AUDIOHARDWARESTREAMPWINPUT_H
#define AUDIOHARDWARESTREAMPWINPUT_H

#include "AudioHardwareStreamPW.h"

class AudioHardwareStreamPWInput : public AudioHardwareStreamPW
{
public:
	using AudioHardwareStreamPW::AudioHardwareStreamPW;
private:
	void processBuffer(const darling_pw_buffer& buffer, void* cookie) override;
};

#endif /* AUDIOHARDWARESTREAMPWINPUT_H */
