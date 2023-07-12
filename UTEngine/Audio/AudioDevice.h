#pragma once

#include <memory>
#include <vector>

class AudioSource;
class USound;

class ActiveSound
{
public:
  int channel = 0;
  bool play = false;
  bool update = false;
  USound* sound = nullptr;
  float volume = 0.0f;
  float pan = 0.0f;
  float pitch = 0.0f;

  double pos = 0;
};

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize);

  AudioDevice(int inFrequency, int numVoices, int musicBufferCount, int musicBufferSize);
  void AddSound(USound* sound);
  void RemoveSound(USound* sound);
  void PlayMusic(std::unique_ptr<AudioSource> source);
  void UpdateMusic();

	virtual ~AudioDevice() = default;
	virtual int PlaySound(int channel, USound* sound, float volume, float pan, float pitch) = 0;
	virtual void UpdateSound(int channel, USound* sound, float volume, float pan, float pitch) = 0;
  virtual void PlayMusicBuffer() = 0;
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
  std::vector<ActiveSound> activeSounds;
  std::unique_ptr<AudioSource> music;

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
