#pragma once

namespace Audio
{
  class MusicComponent : public Component
  {
  public:
    MusicComponent();
    ~MusicComponent();

    virtual void Initialize() override;

  private:
  };
}
