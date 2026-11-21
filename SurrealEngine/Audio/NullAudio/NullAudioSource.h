#pragma once

class NullAudioSource
{
public:
    void Play();
    void Stop();
    bool isPlaying() const;
private:
    bool m_isPlaying = false;
};