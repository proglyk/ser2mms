/**
  * @file port_pps.c
  * @author Ilia Proniashin, mail@proglyk.ru
  * @date 14-November-2025
  */

#include "port_pps.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct pps_s {
  char chip_path[256];
  uint8_t channel;
  bool enabled;
  bool exported;
};

static bool _write_sysfs(const char *path, const char *value);

#if (PORT_USE_STATIC)
static struct pps_s _self;
static int _self_is_used = 0;
#endif

/**
 * @brief Constructor
 */
pps_t pps_init(const char *chip_path, uint8_t channel, uint64_t period_ns, uint64_t duty_cycle_ns)
{
  //if (!chip_path || duty_cycle_ns > period_ns) return NULL;
  assert(chip_path);
  assert(duty_cycle_ns > period_ns);
  
#if PORT_USE_STATIC
  if (_self_is_used) return NULL;
  _self_is_used = 1;
  struct pps_s *self = &_self;
  memset((void*)self, 0, sizeof(struct pps_s));
#else
  struct pps_s *self = calloc(1, sizeof(struct pps_s));
  if (!self) return NULL;
#endif
  
  strncpy(self->chip_path, chip_path, sizeof(self->chip_path) - 1);
  self->channel = channel;
  self->enabled = false;
  self->exported = false;
  
  char path[512], value[32];
  
  // Export канала
  snprintf(path, sizeof(path), "%s/export", chip_path);
  snprintf(value, sizeof(value), "%u", channel);
  if (!_write_sysfs(path, value)) goto error;
  
  self->exported = true;
  usleep(100000); // 100 мс задержка для инициализации
  
  // Установка периода
  snprintf(path, sizeof(path), "%s/pwm%u/period", chip_path, channel);
  snprintf(value, sizeof(value), "%llu", period_ns);
  if (!_write_sysfs(path, value)) goto error;
  
  // Установка duty cycle
  snprintf(path, sizeof(path), "%s/pwm%u/duty_cycle", chip_path, channel);
  snprintf(value, sizeof(value), "%llu", duty_cycle_ns);
  if (!_write_sysfs(path, value)) goto error;
  
  printf("[pps_init] pwm%u: period=%llu ns, duty=%llu ns\n", channel, period_ns, duty_cycle_ns);
  return self;

error:
  if (self->exported) {
  snprintf(path, sizeof(path), "%s/unexport", chip_path);
  snprintf(value, sizeof(value), "%u", channel);
  _write_sysfs(path, value);
  }
#if PORT_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
  printf("[pps_init] err\n");
  return NULL;
}

/**
  * @brief Destructor
  */
void pps_del(pps_t self)
{
  assert(self);

  printf("[pps_del]\n");

  // Выключаем если включен
  if (self->enabled) {
    pps_enable(self, false);
  }

  // Unexport канала
  if (self->exported) {
    char path[512], value[8];
    snprintf(path, sizeof(path), "%s/unexport", self->chip_path);
    snprintf(value, sizeof(value), "%u", self->channel);
    _write_sysfs(path, value);
  }

#if PORT_USE_STATIC
  _self_is_used = 0;
#else
  free(self);
#endif
}

/**
  * @brief Enable/Disable 1PPS output
  */
bool pps_enable(pps_t self, bool enable)
{
  char path[512];
  
  assert(self);
  snprintf(path, sizeof(path), "%s/pwm%u/enable", self->chip_path, self->channel);
  
  if (_write_sysfs(path, enable ? "1" : "0")) {
    self->enabled = enable;
    printf("[pps_enable] pwm%u: %s\n", self->channel, enable ? "enabled" : "disabled");
    return true;
  }
  
  return false;
}

/**
  * @brief Вспомогательная функция: запись строки в sysfs файл
  */
static bool _write_sysfs(const char *path, const char *value) {
  int fd = open(path, O_WRONLY);
  if (fd < 0) return false;
  
  size_t len = strlen(value);
  ssize_t written = write(fd, value, len);
  close(fd);
  
  return (written == (ssize_t)len);
}
