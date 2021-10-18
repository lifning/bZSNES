#include "libretro.hpp"
#include <libsnes/libsnes.hpp>
#include <snes.hpp>

#include <nall/snes/info.hpp>
using namespace nall;

namespace {
  SNES::Input::Device retro_to_snes_device_type(unsigned id) {
    switch(id) {
      case RETRO_DEVICE_NONE: return SNES::Input::Device::None;
      case RETRO_DEVICE_JOYPAD: return SNES::Input::Device::Joypad;
      case RETRO_DEVICE_MOUSE: return SNES::Input::Device::Mouse;
      case RETRO_DEVICE_JOYPAD_MULTITAP: return SNES::Input::Device::Multitap;
      case RETRO_DEVICE_LIGHTGUN:
      case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:
        return SNES::Input::Device::SuperScope;
      case RETRO_DEVICE_LIGHTGUN_JUSTIFIER: return SNES::Input::Device::Justifier;
      case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS: return SNES::Input::Device::Justifiers;
      default: return SNES::Input::Device::None;
    }
  }
  unsigned snes_to_retro_device_type(SNES::Input::Device id) {
    switch(id) {
      case SNES::Input::Device::None: return RETRO_DEVICE_NONE;
      case SNES::Input::Device::Joypad: return RETRO_DEVICE_JOYPAD;
      case SNES::Input::Device::Mouse: return RETRO_DEVICE_MOUSE;
      case SNES::Input::Device::Multitap: return RETRO_DEVICE_JOYPAD_MULTITAP;
      case SNES::Input::Device::SuperScope: return RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE;
      case SNES::Input::Device::Justifier: return RETRO_DEVICE_LIGHTGUN_JUSTIFIER;
      case SNES::Input::Device::Justifiers: return RETRO_DEVICE_LIGHTGUN_JUSTIFIERS;
      default: return RETRO_DEVICE_NONE;
    }
  }
  unsigned retro_to_snes_memory_id(unsigned id) {
    switch(id) {
      case RETRO_MEMORY_SAVE_RAM: return SNES_MEMORY_CARTRIDGE_RAM;
      case RETRO_MEMORY_RTC: return SNES_MEMORY_CARTRIDGE_RTC;
      case RETRO_MEMORY_SNES_BSX_RAM: return SNES_MEMORY_BSX_RAM;
      case RETRO_MEMORY_SNES_BSX_PRAM: return SNES_MEMORY_BSX_PRAM;
      case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM: return SNES_MEMORY_SUFAMI_TURBO_A_RAM;
      case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM: return SNES_MEMORY_SUFAMI_TURBO_B_RAM;
      case RETRO_MEMORY_SNES_GAME_BOY_RAM: return SNES_MEMORY_GAME_BOY_RAM;
      case RETRO_MEMORY_SNES_GAME_BOY_RTC: return SNES_MEMORY_GAME_BOY_RTC;
      default: return -1U;
    }
  }
}

struct RetroInterface : public SNES::Interface {
  retro_video_refresh_t pvideo_refresh;
  retro_audio_sample_t paudio_sample;
  retro_audio_sample_batch_t paudio_sample_batch;
  retro_input_poll_t pinput_poll;
  retro_input_state_t pinput_state;
  const static size_t SAMPLE_BUFFER_SIZE = 32040/60;
  int16_t sample_buffer[SAMPLE_BUFFER_SIZE];
  size_t sample_buffer_index;

  void video_refresh(const uint16_t *data, unsigned width, unsigned height) {
    if(pvideo_refresh) {
      // from SSNES lines_to_pitch
      unsigned pitch = ((height == 448) || (height == 478)) ? 1024 : 2048;
      return pvideo_refresh(data, width, height, pitch);
    }
  }

  void audio_sample(uint16_t left, uint16_t right) {
    if(paudio_sample && !paudio_sample_batch) {
      paudio_sample((int16_t)left, (int16_t)right);
    } else if(paudio_sample_batch) {
      sample_buffer[sample_buffer_index++] = (int16_t)left;
      sample_buffer[sample_buffer_index++] = (int16_t)right;
      if (sample_buffer_index >= SAMPLE_BUFFER_SIZE) {
        sample_buffer_index = 0;
        paudio_sample_batch(sample_buffer, SAMPLE_BUFFER_SIZE/2);
      }
    }
  }

  void input_poll() {
    if(pinput_poll) return pinput_poll();
  }

  int16_t input_poll(bool port, SNES::Input::Device device, unsigned index, unsigned id) {
    if(pinput_state) return pinput_state(port, snes_to_retro_device_type(device), index, id);
    return 0;
  }

  RetroInterface() : pvideo_refresh(0), paudio_sample(0), paudio_sample_batch(0), pinput_poll(0), pinput_state(0), sample_buffer_index(0) {
  }
};

static RetroInterface retroInterface;

void retro_set_environment(retro_environment_t environment) {
  (void)environment;
}

void retro_set_video_refresh(retro_video_refresh_t video_refresh) {
  retroInterface.pvideo_refresh = video_refresh;
}

void retro_set_audio_sample(retro_audio_sample_t audio_sample) {
  retroInterface.paudio_sample = audio_sample;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t audio_sample_batch) {
  retroInterface.paudio_sample_batch = audio_sample_batch;
}

void retro_set_input_poll(retro_input_poll_t input_poll) {
  retroInterface.pinput_poll = input_poll;
}

void retro_set_input_state(retro_input_state_t input_state) {
  retroInterface.pinput_state = input_state;
}

void retro_set_controller_port_device(bool port, unsigned device) {
  SNES::input.port_set_device(port, retro_to_snes_device_type(device));
}

void retro_init(void) {
  SNES::system.init(&retroInterface);
  SNES::input.port_set_device(0, SNES::Input::Device::Joypad);
  SNES::input.port_set_device(1, SNES::Input::Device::Joypad);
}

unsigned retro_api_version(void) {
  return 1;
}

void retro_get_system_info(struct retro_system_info *info) {
  info->library_name = SNES::Info::Name;
  info->library_version = SNES::Info::Version;
  info->valid_extensions = "sfc";
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

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
) {
  switch (game_type) {
    case RETRO_GAME_TYPE_BSX:
      if (num_info != 2) return false;
      return snes_load_cartridge_bsx(
        info[0].meta, (uint8_t*)info[0].data, info[0].size,
        info[1].meta, (uint8_t*)info[1].data, info[1].size
      );
    case RETRO_GAME_TYPE_BSX_SLOTTED:
      if (num_info != 2) return false;
      return snes_load_cartridge_bsx_slotted(
        info[0].meta, (uint8_t*)info[0].data, info[0].size,
        info[1].meta, (uint8_t*)info[1].data, info[1].size
      );
    case RETRO_GAME_TYPE_SUPER_GAME_BOY:
      if (num_info != 2) return false;
      return snes_load_cartridge_super_game_boy(
        info[0].meta, (uint8_t*)info[0].data, info[0].size,
        info[1].meta, (uint8_t*)info[1].data, info[1].size
      );
    case RETRO_GAME_TYPE_SUFAMI_TURBO:
      if (num_info != 2) return false;
      return snes_load_cartridge_sufami_turbo(
        info[0].meta, (uint8_t*)info[0].data, info[0].size,
        info[1].meta, (uint8_t*)info[1].data, info[1].size,
        info[2].meta, (uint8_t*)info[2].data, info[2].size
      );
    default: return false;
  }
}

void* retro_get_memory_data(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;

  switch(id) {
    case RETRO_MEMORY_SYSTEM_RAM:
      return SNES::memory::wram.data();
    case RETRO_MEMORY_VIDEO_RAM:
      return SNES::memory::vram.data();
    default:
      return snes_get_memory_data(retro_to_snes_memory_id(id));
  }
}

unsigned retro_get_memory_size(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;
  unsigned size = 0;

  switch(id) {
    case RETRO_MEMORY_SYSTEM_RAM: size = SNES::memory::wram.size(); break;
    case RETRO_MEMORY_VIDEO_RAM: size = SNES::memory::vram.size(); break;
    default: return snes_get_memory_size(retro_to_snes_memory_id(id));
  }

  if(size == -1U) size = 0;
  return size;
}

// undifferentiated stubs from libsnes:

void retro_deinit(void) { return snes_term(); }
void retro_reset(void) { return snes_reset(); }
void retro_run(void) { return snes_run(); }

unsigned retro_serialize_size(void) { return snes_serialize_size(); }
bool retro_serialize(void *data, unsigned size) {
  return snes_serialize((uint8_t*)data, size);
}
bool retro_unserialize(const void *data, unsigned size) {
  return snes_unserialize((uint8_t*)data, size);
}

void retro_cheat_reset(void) { return snes_cheat_reset(); }
void retro_cheat_set(unsigned index, bool enabled, const char *code) {
  return snes_cheat_set(index, enabled, code);
}

bool retro_load_game(const struct retro_game_info *game) {
  return snes_load_cartridge_normal(game->meta, (uint8_t*)game->data, game->size);
}

void retro_unload_game(void) { return snes_unload_cartridge(); }
unsigned retro_get_region(void) { return snes_get_region(); }
