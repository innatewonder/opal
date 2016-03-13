#include "CommonPrecompiled.h"

#include "NetworkingSystem.h"
#include "AudioSystem.h"


int main(void)
{
  auto audio = new Audio::AudioSystem();

  delete audio;

  return 0;
}