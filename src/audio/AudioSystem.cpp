#include "CommonPrecompiled.h"
#include "AudioSystem.h"

#include "fmod.hpp"

#define LATENCY_MS      (50) /* Some devices will require higher latency to avoid glitches */
#define DRIFT_MS        (1)
#define DEVICE_INDEX    (0)

void CheckFmod(FMOD_RESULT result, int line)
{
  if(result != FMOD_OK)
  {
    ERR("Fmod Error line " << line);
  }
}

#define CHECK_FMOD_ERR(result) CheckFmod(result, __LINE__)

class Mic
{
  FMOD::System* m_sys;
  FMOD::Sound* m_sound;
  FMOD::Channel* channel;
  u32 m_soundLen;
  u32 desiredLatency, adjustedLatency, actualLatency;
  u32 samplesPlayed, samplesRecorded, driftThreshold;
  u32 nativeRate;
public:
  Mic(FMOD::System* sys, int numchannels, int rate)
  {
    m_sys = sys;
    channel = nullptr;
    
    nativeRate = rate;
    driftThreshold = (rate * DRIFT_MS) / 1000;
    desiredLatency = (rate * LATENCY_MS) / 1000;
    adjustedLatency = desiredLatency;
    actualLatency = desiredLatency;          

    samplesPlayed = samplesRecorded = 0;

    FMOD_CREATESOUNDEXINFO exinfo = {0};
    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = numchannels;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = rate;
    exinfo.length           = rate * sizeof(short) * numchannels; /* 1 second buffer, size here doesn't change latency */
    
    FMOD_RESULT result = m_sys->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &m_sound);
    CHECK_FMOD_ERR(result);

    result = m_sys->recordStart(DEVICE_INDEX, m_sound, true);
    CHECK_FMOD_ERR(result);

    result = m_sound->getLength(&m_soundLen, FMOD_TIMEUNIT_PCM);
    CHECK_FMOD_ERR(result);
  }

  ~Mic()
  {
    m_sound->release();
  }

  void Update()
  {
    u32 recordPos = 0;
    FMOD_RESULT result = m_sys->getRecordPosition(DEVICE_INDEX, &recordPos);
    if (result != FMOD_ERR_RECORD_DISCONNECTED)
    {
      CHECK_FMOD_ERR(result);
    }

    static u32 lastRecordPos = 0;
    u32 recordDelta = (recordPos >= lastRecordPos) ? (recordPos - lastRecordPos) : (recordPos + m_soundLen - lastRecordPos);
    lastRecordPos = recordPos;
    samplesRecorded += recordDelta;

    static u32 minRecordDelta = (u32)-1;
    if (recordDelta && (recordDelta < minRecordDelta))
    {
      minRecordDelta = recordDelta; /* Smallest driver granularity seen so far */
      adjustedLatency = (recordDelta <= desiredLatency) ? desiredLatency : recordDelta; /* Adjust our latency if driver granularity is high */
    }
    
    /*
        Delay playback until our desired latency is reached.
    */
    if(!channel && samplesRecorded >= adjustedLatency)
    {
      result = m_sys->playSound(m_sound, 0, false, &channel);
      CHECK_FMOD_ERR(result);
    }

    if(channel)
    {
      /*
          Stop playback if recording stops.
      */
      bool isRecording = false;
      result = m_sys->isRecording(DEVICE_INDEX, &isRecording);
      if (result != FMOD_ERR_RECORD_DISCONNECTED)
      {
        CHECK_FMOD_ERR(result);
      }

      if (!isRecording)
      {
        result = channel->setPaused(true);
        CHECK_FMOD_ERR(result);
      }

      /*
          Determine how much has been played since we last checked.
      */
      u32 playPos = 0;
      result = channel->getPosition(&playPos, FMOD_TIMEUNIT_PCM);
      CHECK_FMOD_ERR(result);

      static u32 lastPlayPos = 0;
      u32 playDelta = (playPos >= lastPlayPos) ? (playPos - lastPlayPos) : (playPos + m_soundLen - lastPlayPos);
      lastPlayPos = playPos;
      samplesPlayed += playDelta;
      
      /*
          Compensate for any drift.
      */
      int latency = samplesRecorded - samplesPlayed;
      actualLatency = (0.97f * actualLatency) + (0.03f * latency);

      int playbackRate = nativeRate;
      if (actualLatency < (int)(adjustedLatency - driftThreshold)) 
      {
        /* Play position is catching up to the record position, slow playback down by 2% */
        playbackRate = nativeRate - (nativeRate / 50); 
      }
      else if (actualLatency > (int)(adjustedLatency + driftThreshold))
      {
        /* Play position is falling behind the record position, speed playback up by 2% */
        playbackRate = nativeRate + (nativeRate / 50);
      }

      channel->setFrequency((float)playbackRate);
      CHECK_FMOD_ERR(result);
    }
  }
};

Mic* lTest = nullptr;

namespace Audio
{
  AudioSystem::AudioSystem()
    : m_system(nullptr)
    , m_channel(nullptr)
  {}

  AudioSystem::~AudioSystem()
  {}

  bool AudioSystem::Initialize(ArgParser& args)
  {
    FMOD_RESULT result = FMOD::System_Create(&m_system);
    CHECK_FMOD_ERR(result);

    u32 version = 0;
    result = m_system->getVersion(&version);
    CHECK_FMOD_ERR(result);

    if(version < FMOD_VERSION)
    {
      ERR("FMOD lib version " << version << " doesn't match header version " << FMOD_VERSION);
    }

    void* extraDriverData = nullptr;
    result = m_system->init(100, FMOD_INIT_NORMAL, extraDriverData);
    CHECK_FMOD_ERR(result);

    int numDrivers = 0;
    result = m_system->getRecordNumDrivers(NULL, &numDrivers);
    CHECK_FMOD_ERR(result);

    if(numDrivers == 0)
    {
      ERR("No recording devices found/plugged in! Aborting.");
    }

    /*
        Determine latency in samples.
    */
    result = m_system->getRecordDriverInfo(DEVICE_INDEX, NULL, 0, NULL, &m_nativeRate, NULL, &m_nativeChannels, NULL);
    CHECK_FMOD_ERR(result);
    return true;
  }


  void AudioSystem::Destroy()
  {
    if(lTest)
      delete lTest;
    FMOD_RESULT result = m_system->release();
    CHECK_FMOD_ERR(result);
  }


  void AudioSystem::Test()
  {
    lTest = new Mic(m_system, m_nativeChannels, m_nativeRate);
  }


  void AudioSystem::Tick(f32 dt)
  {
    FMOD_RESULT result = m_system->update();
    CHECK_FMOD_ERR(result);
    
    if(lTest)
      lTest->Update();
  }


  void AudioSystem::HandleMessage(Message* msg)
  {
    
  }

}
