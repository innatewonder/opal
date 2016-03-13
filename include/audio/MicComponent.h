#pragma once

namespace Audio
{
  class MicComponent : public Component
  {
  public:
    MicComponent();
    ~MicComponent();

    virtual void Initialize();

  private:
  };
}
