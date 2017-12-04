/*
FILE: AudioEngine.h
PRIMARY AUTHOR: Kieran

Copyright � 2017 DigiPen (USA) Corporation.
*/
#pragma once

#include "Universal.h"

namespace FMOD
{
	class Channel;
	class System;
}

namespace Audio
{
	void Init();

	void Update();

	// Represents a currently playing sound.
	class SoundHandle
	{
	public:

		SoundHandle(FMOD::Channel *fmodChannel, const char *fileName);

		bool IsPlaying() const;

		bool IsLooping() const;

		void Stop();

		void SetPitch(float pitch);

		float GetPitch();

		SoundHandle() {}; // For meta.

		const char *GetFileName()
		{
			return m_fileName.c_str();
		}
	private:

		FMOD::Channel *m_fmodChannel;

		std::string m_fileName;

		META_NAMESPACE(Audio);
		META_REGISTER(SoundHandle)
		{
			META_DefineFunction(SoundHandle, IsPlaying, "IsPlaying");
			META_DefineFunction(SoundHandle, Stop, "Stop");

			META_DefineFunction(SoundHandle, GetFileName, "GetFileName");
		}
	};

	// Plays a given sound once.
	SoundHandle PlaySound(const char *fileName, float volume = 1.0f, float pitch = 1.0f, bool looping = false);

	FMOD::System *GetSystem();
}
