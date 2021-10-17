#ifndef LIBRETRO_HPP
#define LIBRETRO_HPP

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RETRO_DEVICE_NONE         0
#define RETRO_DEVICE_JOYPAD       1
#define RETRO_DEVICE_MOUSE        2
#define RETRO_DEVICE_LIGHTGUN     4

#define RETRO_DEVICE_ID_JOYPAD_B        0
#define RETRO_DEVICE_ID_JOYPAD_Y        1
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
#define RETRO_DEVICE_ID_JOYPAD_START    3
#define RETRO_DEVICE_ID_JOYPAD_UP       4
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
#define RETRO_DEVICE_ID_JOYPAD_A        8
#define RETRO_DEVICE_ID_JOYPAD_X        9
#define RETRO_DEVICE_ID_JOYPAD_L       10
#define RETRO_DEVICE_ID_JOYPAD_R       11

#define RETRO_DEVICE_ID_MOUSE_X      0
#define RETRO_DEVICE_ID_MOUSE_Y      1
#define RETRO_DEVICE_ID_MOUSE_LEFT   2
#define RETRO_DEVICE_ID_MOUSE_RIGHT  3

#define RETRO_DEVICE_ID_LIGHTGUN_X        0
#define RETRO_DEVICE_ID_LIGHTGUN_Y        1
#define RETRO_DEVICE_ID_LIGHTGUN_TRIGGER  2
#define RETRO_DEVICE_ID_LIGHTGUN_CURSOR   3
#define RETRO_DEVICE_ID_LIGHTGUN_TURBO    4
#define RETRO_DEVICE_ID_LIGHTGUN_PAUSE    5

#define RETRO_REGION_NTSC  0
#define RETRO_REGION_PAL   1

#define RETRO_MEMORY_SAVE_RAM            0
#define RETRO_MEMORY_RTC                 1
#define RETRO_MEMORY_SYSTEM_RAM          2
#define RETRO_MEMORY_VIDEO_RAM           3

struct retro_game_info
{
  const char *path;
  const void *data;
  size_t      size;
  const char *meta;
};


struct retro_system_info
{
  const char *library_name;
  const char *library_version;
  const char *valid_extensions;
  bool        need_fullpath;
  bool        block_extract;
};

struct retro_game_geometry
{
  unsigned base_width;
  unsigned base_height;
  unsigned max_width;
  unsigned max_height;
  float    aspect_ratio;
};

struct retro_system_timing
{
  double fps;
  double sample_rate;
};

struct retro_system_av_info
{
  struct retro_game_geometry geometry;
  struct retro_system_timing timing;
};

typedef bool (*retro_environment_t)(unsigned cmd, void *data);
typedef void (*retro_video_refresh_t)(const uint16_t *data, unsigned width, unsigned height);
typedef void (*retro_audio_sample_t)(uint16_t left, uint16_t right);
typedef size_t (*retro_audio_sample_batch_t)(const int16_t *data, size_t frames);
typedef void (*retro_input_poll_t)(void);
typedef int16_t (*retro_input_state_t)(bool port, unsigned device, unsigned index, unsigned id);

void retro_set_environment(retro_environment_t);
void retro_set_video_refresh(retro_video_refresh_t);
void retro_set_audio_sample(retro_audio_sample_t);
void retro_set_audio_sample_batch(retro_audio_sample_batch_t);
void retro_set_input_poll(retro_input_poll_t);
void retro_set_input_state(retro_input_state_t);

void retro_set_controller_port_device(bool port, unsigned device);

void retro_init(void);
void retro_deinit(void);

unsigned retro_api_version(void);

void retro_get_system_info(struct retro_system_info *info);
void retro_get_system_av_info(struct retro_system_av_info *info);

void retro_reset(void);
void retro_run(void);

unsigned retro_serialize_size(void);
bool retro_serialize(void *data, unsigned size);
bool retro_unserialize(const void *data, unsigned size);

void retro_cheat_reset(void);
void retro_cheat_set(unsigned index, bool enabled, const char *code);

bool retro_load_game(
  const struct retro_game_info *game
);

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
);

void retro_unload_game(void);

bool retro_get_region(void);
void* retro_get_memory_data(unsigned id);
unsigned retro_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
