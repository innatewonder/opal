#pragma once

namespace FMOD
{
  class Channel;
  class System;
}

namespace Audio
{
  class AudioSystem : public System
  {
  public:
    AudioSystem();
    ~AudioSystem();

    virtual bool Initialize(ArgParser& args) override;
    virtual void Destroy() override;
    virtual void Test() override;
    virtual void Tick(f32 dt) override;
    virtual void HandleMessage(Message* msg) override;

  private:
    FMOD::System* m_system;
    FMOD::Channel* m_channel;

    s32 m_nativeRate;
    s32 m_nativeChannels;
  };
}
