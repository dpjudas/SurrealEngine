#pragma once

#include "AudioSource.h"

#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include "Math/vec.h"

class AudioSource;
class USound;

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize);

	virtual ~AudioDevice() = default;
  virtual void AddSound(USound* sound) = 0;
  virtual void RemoveSound(USound* sound) = 0;
  virtual bool IsPlaying(int channel) = 0;
	virtual int PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) = 0;
  virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
  virtual void PlayMusicBuffer() = 0;
  virtual void UpdateSound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) = 0;
  virtual void UpdateMusicBuffer() = 0;
	virtual void StopSound(int channel) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetSoundVolume(float volume) = 0;
	virtual void Update() = 0;

protected:
  template<class T> class RingQueue
  {
  public:
    RingQueue<T>()
    {
      Data = nullptr;
      Len = 0;
      Num = 0;
      Current = 0;
    }

    RingQueue<T>(size_t n)
    {
      Data = static_cast<T*>(malloc(sizeof(T) * n));
      Len = n;
      Num = 0;
      Current = 0;
    }

    RingQueue<T>(size_t n, const T& Value)
    {
      Data = static_cast<T*>(malloc(sizeof(T) * n));
      for (int i = 0; i < n; i++)
        Data[i] = Value;

      Len = n;
      Num = 0;
      Current = 0;
    }

    ~RingQueue<T>()
    {
      free(Data);
    }

    bool Empty()
    {
      return (Num == 0);
    }

    size_t Size()
    {
      return Num;
    }

    T& Front()
    {
      return Data[Current];
    }

    T& GetNextFree()
    {
      size_t Index = Current + Num;
      if (Index >= Len)
        Index -= Len;

      return Data[Index];
    }

    bool Push(const T& Val)
    {
      if (Num == Len)
        return false;

      GetNextFree() = Val;
      Num++;

      return true;
    }

    bool Push(T& Val)
    {
      if (Num == Len)
        return false;

      GetNextFree() = Val;
      Num++;

      return true;
    }

    T& Pop()
    {
      T& Out = Data[Current];
      if (Num > 0)
      {
        Current++;
        if (Current >= Len)
          Current = 0;

        Num--;
      }
      return Out;
    }

    bool Resize(size_t NewSize)
    {
      T* NewData = static_cast<T*>(realloc(Data, sizeof(T) * NewSize));
      if (NewData == NULL)
        return false;

      Data = NewData;
      Len = NewSize;
      return true;
    }

    void Clear()
    {
      Current = 0;
      Num = 0;
    }

  private:
    T* Data;
    size_t Len;
    size_t Current;
    size_t Num;
  };

	int frequency = 48000;
  std::vector<USound*> sounds;
  std::unique_ptr<AudioSource> music;

  std::mutex playbackMutex;
  std::unique_ptr<std::thread> playbackThread;
  bool bExit;

  RingQueue<float*> musicQueue;
  int musicBufferCount;
  int musicBufferSize;
  float* musicBuffer;
  bool musicUpdate = false;
  bool bMusicTransition = false;
  bool bMusicPlaying = false;
  float currentMusicVolume;
  float targetMusicVolume;
  float fadeRate = 0.0;
};
