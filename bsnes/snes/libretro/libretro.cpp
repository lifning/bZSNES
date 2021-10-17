#include "libretro.hpp"
#include <snes.hpp>

#include <nall/snes/info.hpp>
using namespace nall;

namespace {
  SNES::Input::Device retro_to_snes_device_id(unsigned id) {
    switch(id) {
      case RETRO_DEVICE_NONE: return SNES::Input::Device::None;
      case RETRO_DEVICE_JOYPAD: return SNES::Input::Device::Joypad;
      case RETRO_DEVICE_MOUSE: return SNES::Input::Device::Mouse;
      case RETRO_DEVICE_LIGHTGUN: return SNES::Input::Device::SuperScope;
      default: return SNES::Input::Device::None;
    }
  }
  unsigned snes_to_retro_device_id(SNES::Input::Device id) {
    switch(id) {
      case SNES::Input::Device::None: return RETRO_DEVICE_NONE;
      case SNES::Input::Device::Joypad: return RETRO_DEVICE_JOYPAD;
      case SNES::Input::Device::Mouse: return RETRO_DEVICE_MOUSE;
      case SNES::Input::Device::SuperScope: return RETRO_DEVICE_LIGHTGUN;
      default: return RETRO_DEVICE_NONE;
    }
  }
}

struct Interface : public SNES::Interface {
  retro_video_refresh_t pvideo_refresh;
  retro_audio_sample_t paudio_sample;
  retro_audio_sample_batch_t paudio_sample_batch;
  retro_input_poll_t pinput_poll;
  retro_input_state_t pinput_state;

  void video_refresh(const uint16_t *data, unsigned width, unsigned height) {
    if(pvideo_refresh) {
      unsigned pitch = (height <= 239) ? 1024 : 512;
      return pvideo_refresh(data, width, height, pitch);
    }
  }

  void audio_sample(uint16_t left, uint16_t right) {
    if(paudio_sample) {
      paudio_sample((int16_t)left, (int16_t)right);
    } else if(paudio_sample_batch) {
      const int16_t samples[2] = { (int16_t)left, (int16_t)right };
      paudio_sample_batch(samples, 1);
    }
  }

  void input_poll() {
    if(pinput_poll) return pinput_poll();
  }

  int16_t input_poll(bool port, SNES::Input::Device device, unsigned index, unsigned id) {
    if(pinput_state) return pinput_state(port, snes_to_retro_device_id(device), index, id);
    return 0;
  }

  Interface() : pvideo_refresh(0), paudio_sample(0), paudio_sample_batch(0), pinput_poll(0), pinput_state(0) {
  }
};

static Interface interface;

void retro_set_environment(retro_environment_t environment) {
  (void)environment;
}

void retro_set_video_refresh(retro_video_refresh_t video_refresh) {
  interface.pvideo_refresh = video_refresh;
}

void retro_set_audio_sample(retro_audio_sample_t audio_sample) {
  interface.paudio_sample = audio_sample;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t audio_sample_batch) {
  interface.paudio_sample_batch = audio_sample_batch;
}

void retro_set_input_poll(retro_input_poll_t input_poll) {
  interface.pinput_poll = input_poll;
}

void retro_set_input_state(retro_input_state_t input_state) {
  interface.pinput_state = input_state;
}

void retro_set_controller_port_device(bool port, unsigned device) {
  SNES::input.port_set_device(port, retro_to_snes_device_id(device));
}

void retro_init(void) {
  SNES::system.init(&interface);
  SNES::input.port_set_device(0, SNES::Input::Device::Joypad);
  SNES::input.port_set_device(1, SNES::Input::Device::Joypad);
}

void retro_deinit(void) {
  SNES::system.term();
}

unsigned retro_api_version(void) {
  return 1;
}

void retro_get_system_info(struct retro_system_info *info) {
  info->library_name = "bZSNES";
  info->library_version = "v1.51";
  info->valid_extensions = "sfc|smc";
  info->need_fullpath = false;
  info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  info->timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0   / 425568.0;
  info->timing.sample_rate = 32040.5;
  info->geometry.base_width = 256;
  info->geometry.base_height = 224;
  info->geometry.max_width = 512;
  info->geometry.max_height = 448;
  info->geometry.aspect_ratio = 8.0 / 7.0;
}

void retro_reset(void) {
  SNES::system.reset();
}

void retro_run(void) {
  SNES::system.run();
}

unsigned retro_serialize_size(void) {
  return SNES::system.serialize_size();
}

bool retro_serialize(void *data, unsigned size) {
  SNES::system.runtosave();
  serializer s = SNES::system.serialize();
  if(s.size() > size) return false;
  memcpy(data, s.data(), s.size());
  return true;
}

bool retro_unserialize(const void *data, unsigned size) {
  serializer s((uint8_t*)data, size);
  return SNES::system.unserialize(s);
}

void retro_cheat_reset(void) {
  SNES::cheat.reset();
  SNES::cheat.synchronize();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
  SNES::cheat[index] = code;
  SNES::cheat[index].enabled = enabled;
  SNES::cheat.synchronize();
}

bool retro_load_game(
  const struct retro_game_info *game
) {
  retro_cheat_reset();
  if(game->data) SNES::memory::cartrom.copy((uint8_t*)game->data, game->size);
  string xmlrom = (game->meta && *(game->meta)) ? string(game->meta) : snes_information((uint8_t*)game->data, game->size).xml_memory_map;
  SNES::cartridge.load(SNES::Cartridge::Mode::Normal, { xmlrom });
  SNES::system.power();
  return true;
}

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
) {
  return false;
}

void retro_unload_game(void) {
  SNES::cartridge.unload();
}

bool retro_get_region(void) {
  return SNES::system.region() == SNES::System::Region::NTSC ? 0 : 1;
}

void* retro_get_memory_data(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      return SNES::memory::cartram.data();
    case RETRO_MEMORY_RTC:
      return SNES::memory::cartrtc.data();
    case RETRO_MEMORY_SYSTEM_RAM:
      return SNES::memory::wram.data();
    case RETRO_MEMORY_VIDEO_RAM:
      return SNES::memory::vram.data();
    default:break;
  }

  return 0;
}

unsigned retro_get_memory_size(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;
  unsigned size = 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      size = SNES::memory::cartram.size();
      break;
    case RETRO_MEMORY_RTC:
      size = SNES::memory::cartrtc.size();
      break;
    case RETRO_MEMORY_SYSTEM_RAM:
      size = SNES::memory::wram.size();
      break;
    case RETRO_MEMORY_VIDEO_RAM:
      size = SNES::memory::vram.size();
      break;
  }

  if(size == -1U) size = 0;
  return size;
}
