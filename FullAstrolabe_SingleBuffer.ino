#include "src/PCA9554_Qualia.h"
#include "src/PanelConfig.h"
#include "src/PotentiometerCalibrator.h"
#include "src/CalibrationPoints.h"
#include "src/StarChart.h"
#include "src/Tympanum.h"
#include "src/CircleMath.h"
#include "src/AstrolabeInterface.h"

#include "CircularBuffer.hpp"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include <array>
#include "esp_heap_caps.h"

#define SATURN_COLOR 0x9ac3
#define MARS_COLOR 0xc104
#define VENUS_COLOR 0Xffd2
#define MERCURY_COLOR 0x18ad
#define JUPITER_COLOR 0x0505
#define SUN_COLOR 0xffff
#define MOON_COLOR 0xffff
#define BG_COLOR 0x0000
#define STAR_COLOR 0xe73c
#define LINE_COLOR 0x4228
#define HIGHLIGHT_COLOR 0x7b48
#define OUTLINE_COLOR 0xffff

#define SATURN 4
#define MARS 2
#define VENUS 1
#define MERCURY 0
#define JUPITER 3
#define SUN 6
#define MOON 5
#define ANTISOLAR 7

#define SUN_SIZE 18
#define MOON_SIZE 16
#define PLANET_SIZE 12
#define ANTISOLAR_SIZE 6

// Create an instance of the Adafruit_XCA9554 class
PCA9554_Qualia expander;

//global framebuffers
void* framebuffer = nullptr;
uint16_t* fb;
uint16_t* fb_bg;

int post_change_frames = 4;  //draw n frames after angle stops changing
uint8_t frame_change_counter = 0;
uint8_t loop_entry_counter = 5;  //get around frame change counter on first n loops

//star map drawing variables
float angle = 0;
float old_angle = 0;
float old_eph_angle = 0;
float erase_angle = 0;
int old_center_x, old_center_y, center_x, center_y;
int i, j;
int n_stars;
const float k_star_size = 0.23;
const float scale_factor = 193.75;  //3.875'' map = 3.6 in stereographic projection units, 180 pixels/in  3.875/3.6*180
float star_r, star_size, star_long;
bool angle_changed;


//planet drawing variables
PlanetData data, old_data;
const int n_planets = 8;
float planet_r, planet_long;
uint16_t planet_color;
StereoCoords planet_coords;
float planet_size;
int sun_center_x, sun_center_y;
float sun_long = 0;
float phase_angle;
std::array<std::array<int, 2>, n_planets> planet_erase_locs;
RetrogradeInfo ri;
std::array<SphereVector, 7> planets_altaz;
std::array<SphereVector, 12> stars_altaz;


//sun and moon drawing variables
const int sun_inner_r = 2;
const int sun_ring_r = 14;
const int sun_ring_thickness = 1;
const int moon_ring_thickness = 1;
const int moon_ring_spacing = 1;
const int planet_thickness = 1;
const int antisolar_inner_r = 1;

//potentiometer measurement
float sum_pot = 0;
float average_pot;
const int n_sample = 60;
PotentiometerCalibrator* calibrator;
const float calibration_k = 1.000;
std::vector<std::array<float, 2>> points;


//potentiometer buffering
const int n_buffer = 25;
const int buffer_size = n_buffer * n_sample;
CircularBuffer<int, buffer_size> buffer;

//time calculations
TimeCalculator* time_calculator;
int sidereal_day_offset = 0;
double jd, old_jd_back, old_jd_front;
DateTime dt;
bool DST = true;
int UTC_offset = -8;

//tympanum
Tympanum* tympanum;
const int horizon_thickness = 4;
const int default_thickness = 1;
std::array<float, n_houses>cusps;
float latitude = 10.0; //insert your own default latitude
float longitude = 0.0; //insert your own default longitude

//display
AstrolabeInterface* interface;
InterfaceParams p;


esp_lcd_panel_handle_t panel_handle = NULL;

uint16_t color;

unsigned long timer;

long timer_cusp_calc;
long timer_eph;
long timer_eph_sample;
long timer_interface_update;
long timer_draw;

volatile bool encoder_irq;
volatile bool button_irq;
EncoderReturn encoder_return;
bool date_changed;
bool date_change_eph_request;
bool angle_changed_eph_old;

Adafruit_PCF8574 pcf;

std::array<Circle2D, n_almucantar> almucantar;
std::array<Circle2D, n_azimuth> azimuth;
std::array<Circle2D, n_hours> hours;
std::array<Circle2D, 3> tropics;
Circle2D horizon_unscaled;

Circle2D horizon;
Circle2D horizon_outside;

Settings new_settings;

void encoderIRQ() {
  encoder_irq = true;
}

void buttonIRQ() {
  button_irq = true;
}

void drawVerticalLine(int x, uint16_t color, int thickness, uint16_t* fb) {
  int left_bound = x - thickness < 0 ? 0 : x - thickness;
  int right_bound = x + thickness > 720 ? 720 : x + thickness;
  for (int i = 0; i <= 720; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      fb[i * 720 + j] = color;
    }
  }
}

void drawCircle(int x, int y, int r, uint16_t color, uint16_t* fb, bool erase = false) {
  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;

  int r_limit = r * r;
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      if ((i - y) * (i - y) + (j - x) * (j - x) <= r_limit) {
        fb[i * 720 + j] = erase ? fb_bg[i * 720 + j] : color;
      }
    }
  }
}

void drawCircleHollow(int x, int y, int r, uint16_t color, uint16_t* fb, int thickness, bool erase = false) {
  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;

  int r_squared;
  int r_limit = r * r;
  int r_outline_limit = (r - thickness) * (r - thickness);
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared <= r_limit && r_squared >= r_outline_limit) {
        fb[i * 720 + j] = erase ? fb_bg[i * 720 + j] : color;
      }
    }
  }
}

void drawPlanet(int x, int y, int r, uint16_t color, uint16_t* fb) {
  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;
  int thickness = planet_thickness;

  int r_squared;
  int r_limit = r * r;
  int r_outline_limit = (r - thickness) * (r - thickness);
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared > r_limit) continue;
      if (r_squared >= r_outline_limit) {
        fb[i * 720 + j] = OUTLINE_COLOR;
      } else {
        fb[i * 720 + j] = color;
      }
    }
  }
}

void drawAntisolar(int x, int y, int r, uint16_t* fb) {
  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;
  int thickness = planet_thickness;

  int antisolar_inner_limit = antisolar_inner_r * antisolar_inner_r;
  int r_squared;
  int r_limit = r * r;
  int r_outline_limit = (r - thickness) * (r - thickness);
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared > r_limit) continue;
      if (r_squared >= r_outline_limit) {
        fb[i * 720 + j] = OUTLINE_COLOR;
        continue;
      }
      if (r_squared <= antisolar_inner_limit) {
        fb[i * 720 + j] = SUN_COLOR;
        continue;
      }
      fb[i * 720 + j] = BG_COLOR;
    }
  }
}

void drawSun(int x, int y, uint16_t* fb) {

  int r = SUN_SIZE;

  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;

  int r_squared;
  int r_limit = r * r;
  int sun_inner_limit = sun_inner_r * sun_inner_r;
  int sun_ring_limit_outer = sun_ring_r * sun_ring_r;
  int sun_ring_limit_inner = (sun_ring_r - sun_ring_thickness) * (sun_ring_r - sun_ring_thickness);
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared > r_limit) continue;
      if (r_squared <= sun_inner_limit) {
        fb[i * 720 + j] = BG_COLOR;
        continue;
      }
      if (r_squared >= sun_ring_limit_inner && r_squared <= sun_ring_limit_outer) {
        fb[i * 720 + j] = BG_COLOR;
        continue;
      }
      fb[i * 720 + j] = SUN_COLOR;
    }
  }
}

void drawMoon(int x, int y, int sun_x, int sun_y, float phase_angle, uint16_t* fb) {
  int thickness = moon_ring_thickness;
  int spacing = moon_ring_spacing;
  float normalized_phase_angle = getNormalizedAngle(phase_angle);
  float sun_moon_angle = atan2((sun_y - y), (sun_x - x));
  float k = cos(normalized_phase_angle);
  float r = MOON_SIZE - thickness - spacing;
  Point2D a = { .x = x + r * k * cos(sun_moon_angle), .y = y + r * k * sin(sun_moon_angle) };
  Point2D b = { .x = x + r * cos(sun_moon_angle + PI / 2), .y = y + r * sin(sun_moon_angle + PI / 2) };
  Point2D c = { .x = x + r * cos(sun_moon_angle + 3 * PI / 2), .y = y + r * sin(sun_moon_angle + 3 * PI / 2) };
  Circle2D d = getCircleFromThreePoints(a, b, c);
  bool right_side_phase = (normalized_phase_angle < PI / 2 || normalized_phase_angle > 3 * PI / 2);  //if the moon is less than first quarter or more than third quarter

  int r_outer = MOON_SIZE;
  int left_bound = x - r_outer < 0 ? 0 : x - r_outer;
  int right_bound = x + r_outer > 720 ? 720 : x + r_outer;
  int upper_bound = y + r_outer > 720 ? 720 : y + r_outer;
  int lower_bound = y - r_outer < 0 ? 0 : y - r_outer;
  int r_squared;
  double distance_from_d_center;

  int r_limit = r_outer * r_outer;
  int border_limit = (r + spacing) * (r + spacing);
  int r_inner_limit = r * r;
  int d_limit = d.r * d.r;
  int d_x = d.x;
  int d_y = d.y;
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared > r_limit) continue;
      if (r_squared >= border_limit) {
        fb[i * 720 + j] = MOON_COLOR;
        continue;
      }
      if (r_squared > r_inner_limit) {
        fb[i * 720 + j] = BG_COLOR;
        continue;
      }
      distance_from_d_center = (i - d_y) * (i - d_y) + (j - d_x) * (j - d_x);
      if (right_side_phase && (distance_from_d_center >= d_limit)) {
        fb[i * 720 + j] = MOON_COLOR;
        continue;
      } else if (!right_side_phase && (distance_from_d_center <= d_limit)) {
        fb[i * 720 + j] = MOON_COLOR;
        continue;
      }
      fb[i * 720 + j] = BG_COLOR;
    }
  }
}

void drawBoundedArc(int x, int y, int r, Circle2D bound, bool inside, uint16_t color, uint16_t* fb, int thickness) {
  int left_bound = x - r < 0 ? 0 : x - r;
  int right_bound = x + r > 720 ? 720 : x + r;
  int upper_bound = y + r > 720 ? 720 : y + r;
  int lower_bound = y - r < 0 ? 0 : y - r;

  int r_squared;
  int r_limit = r * r;
  int r_outline_limit = (r - thickness) * (r - thickness);
  int circle_distance;
  int bound_x = bound.x;
  int bound_y = bound.y;
  int bound_r = bound.r;
  for (int i = lower_bound; i <= upper_bound; i++) {
    for (int j = left_bound; j <= right_bound; j++) {
      circle_distance = (i - bound_y) * (i - bound_y) + (j - bound_x) * (j - bound_x);
      if (inside ^ (circle_distance < bound_r * bound_r)) continue;
      r_squared = (i - y) * (i - y) + (j - x) * (j - x);
      if (r_squared <= r_limit && r_squared >= r_outline_limit) {
        fb[i * 720 + j] = color;
      }
    }
  }
}

RetrogradeInfo getRetrogradeInfo(double jd_a, double jd_b, PlanetData pd_a, PlanetData pd_b) {
  if (jd_a > jd_b) {
    return RetrogradeInfo{
      .mercury_r = isRetrograde(pd_a.mercury_ecl_latlong.lon, pd_b.mercury_ecl_latlong.lon),
      .venus_r = isRetrograde(pd_a.venus_ecl_latlong.lon, pd_b.venus_ecl_latlong.lon),
      .mars_r = isRetrograde(pd_a.mars_ecl_latlong.lon, pd_b.mars_ecl_latlong.lon),
      .jupiter_r = isRetrograde(pd_a.jupiter_ecl_latlong.lon, pd_b.jupiter_ecl_latlong.lon),
      .saturn_r = isRetrograde(pd_a.saturn_ecl_latlong.lon, pd_b.saturn_ecl_latlong.lon)
    };
  }
  else {
    return RetrogradeInfo{
      .mercury_r = isRetrograde(pd_b.mercury_ecl_latlong.lon, pd_a.mercury_ecl_latlong.lon),
      .venus_r = isRetrograde(pd_b.venus_ecl_latlong.lon, pd_a.venus_ecl_latlong.lon),
      .mars_r = isRetrograde(pd_b.mars_ecl_latlong.lon, pd_a.mars_ecl_latlong.lon),
      .jupiter_r = isRetrograde(pd_b.jupiter_ecl_latlong.lon, pd_a.jupiter_ecl_latlong.lon),
      .saturn_r = isRetrograde(pd_b.saturn_ecl_latlong.lon, pd_a.saturn_ecl_latlong.lon)
    };
  }
}

bool isRetrograde(float lon_current, float lon_past) {
  if (lon_current > lon_past + PI) return true;
  return lon_past > lon_current;
}

void initializeSettings(double _latitude, double _longitude, int _offset, bool _DST) {
  tympanum->setLatitude(_latitude);
  time_calculator->setSettings(sidereal_day_offset, _longitude, _offset, _DST);
  latitude = _latitude;
  longitude = _longitude;
  UTC_offset = _offset;
  
  int i;
  almucantar = tympanum->getAlmucantar();
  azimuth = tympanum->getAzimuth();
  hours = tympanum->getHours();
  tropics = tympanum->getTropics();
  horizon_unscaled = tympanum->getHorizon();

  horizon = Circle2D{ .x = 360, .y = 360 - scale_factor * horizon_unscaled.y, .r = scale_factor * horizon_unscaled.r };
  horizon_outside = Circle2D{ .x = 360, .y = 360 - scale_factor * almucantar[0].y, .r = horizon_thickness + scale_factor * almucantar[0].r };

  int x, y, r, line_thickness;
  uint16_t line_color;

  for (i = 0; i < 720; i++) {
    for (j = 0; j < 720; j++) {
      fb_bg[i * 720 + j] = BG_COLOR;
    }
  }

  int y_center_almucantar;
  int r_center_almucantar;
  for (i = 0; i < n_almucantar; i++) {
    y = 360 - scale_factor * almucantar[i].y;
    r = scale_factor * almucantar[i].r;

    if (i == 0) {
      line_thickness = horizon_thickness;
      line_color = HIGHLIGHT_COLOR;
    } else {
      line_thickness = default_thickness;
      line_color = LINE_COLOR;
    }
    drawCircleHollow(360, y, r + line_thickness, line_color, fb_bg, line_thickness);
    if (i == n_almucantar - 1) {
      y_center_almucantar = y;
      r_center_almucantar = r;
    }
  }


  for (i = 0; i < n_azimuth; i++) {
    if (i == n_azimuth / 2) {
      continue;
    }
    line_thickness = default_thickness;
    x = 360 + scale_factor * azimuth[i].x;
    y = 360 - scale_factor * azimuth[i].y;
    r = scale_factor * azimuth[i].r;
    drawBoundedArc(x, y, r + line_thickness, horizon, true, LINE_COLOR, fb_bg, line_thickness);
  }


  for (i = 0; i < 3; i++) {
    line_thickness = default_thickness;
    r = scale_factor * tropics[i].r;
    drawBoundedArc(360, 360, r + line_thickness, horizon_outside, false, LINE_COLOR, fb_bg, line_thickness);
  }

  for (i = 0; i < n_hours; i++) {
    line_thickness = default_thickness;
    x = 360 + scale_factor * hours[i].x;
    y = 360 - scale_factor * hours[i].y;
    r = scale_factor * hours[i].r;
    drawBoundedArc(x, y, r + line_thickness, horizon_outside, false, LINE_COLOR, fb_bg, line_thickness);
    x = 360 - scale_factor * hours[i].x;
    drawBoundedArc(x, y, r + line_thickness, horizon_outside, false, LINE_COLOR, fb_bg, line_thickness);
  }

  r = scale_factor * tropics[0].r;
  int tropic_limit = r * r;
  int horizon_limit = horizon_outside.r * horizon_outside.r;
  int r_squared, distance_horizon;
  for (i = 360 - r; i <= 360 + r; i++) {
    for (j = 360 - r; j <= 360 + r; j++) {
      r_squared = (i - 360) * (i - 360) + (j - 360) * (j - 360);
      if (r_squared > tropic_limit) continue;
      distance_horizon = (i - horizon_outside.y) * (i - horizon_outside.y) + (j - horizon_outside.x) * (j - horizon_outside.x);
      if (distance_horizon > horizon_limit) {
        fb_bg[i * 720 + j] = BG_COLOR;
      }
    }
  }
  drawVerticalLine(360, LINE_COLOR, 0, fb_bg);
  drawCircle(360, y_center_almucantar, r_center_almucantar, BG_COLOR, fb_bg);


  for (i = 0; i < 720; i++) {
    for (j = 0; j < 720; j++) {
      fb[i * 720 + j] = fb_bg[i * 720 + j];
    }
  }
}

void setCalibration(float _calibration_k) {
  int n = calibration_points.size();
  for (i = 0; i < n; i++) {
    std::array<float, 2> point = {_calibration_k * calibration_points[i], PI / 4 * i };
    points[i] = point;
  }
  calibrator->setPoints(points);
}

void setup() {

  pcf.begin(0x20, &Wire);

  for (i = 0; i < calibration_points.size(); i++) {
    std::array<float, 2> point = {calibration_k * calibration_points[i], PI / 4 * i };
    points.push_back(point);
  }

  calibrator = new PotentiometerCalibrator(points);
  sum_pot = 0;
  for (i = 0; i < buffer_size; i++) {
    buffer.unshift(analogRead(A0));
    sum_pot += buffer[0];
  }
  average_pot = sum_pot / buffer_size;
  angle = -(calibrator->getAngle(average_pot));

  n_stars = star_data.size();

  tympanum = new Tympanum();
  time_calculator = new TimeCalculator(sidereal_day_offset);
  data = getPlanetData(time_calculator->getJulianDate(angle));
  jd = time_calculator->getJulianDate(angle);
  dt = time_calculator->getDateTime(jd);
  InterfaceParams p = { .dt = dt };
  Settings settings = {.latitude = latitude, .longitude = longitude, .UTC_offset = UTC_offset, .DST = DST};
  interface = new AstrolabeInterface(p, &pcf, settings, calibration_k);
  interface->initializeLoading();

  pinMode(ENCODER_INTERRUPT, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_INTERRUPT), encoderIRQ, FALLING);

  pinMode(BUTTON_INTERRUPT, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT), buttonIRQ, FALLING);

  timer = millis();
  timer_eph_sample = millis();
  timer_interface_update = millis();

  expander.begin();
  expander.initHD40015C40();
  expander.setBacklight(HIGH);

  esp_lcd_new_rgb_panel(&panel_config_single, &panel_handle);
  esp_lcd_panel_reset(panel_handle);
  esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 1, &framebuffer);

  fb = (uint16_t*)framebuffer;
  fb_bg = (uint16_t*)heap_caps_malloc(720 * 720 * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  esp_lcd_rgb_panel_event_callbacks_t cbs = {
    .on_vsync = draw_to_buffer,
  };


  initializeSettings(latitude, longitude, UTC_offset, DST);
  
  esp_lcd_panel_init(panel_handle);
  esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, nullptr);

  interface->exitLoading();
}

void loop() {
  date_changed = false;
  angle_changed = false;

  if (encoder_irq) {
    encoder_irq = false;
    encoder_return = interface->serviceEncoder();
    if (encoder_return.date_offset != 0) {
      sidereal_day_offset += encoder_return.date_offset;
      time_calculator->setOffset(sidereal_day_offset);
      date_changed = true;
      date_change_eph_request = true;
    }
    if (encoder_return.settings_changed) {
        new_settings = interface->getSettings();
        initializeSettings(new_settings.latitude, new_settings.longitude, new_settings.UTC_offset, new_settings.DST);
        loop_entry_counter = 2;
    }
    if (encoder_return.calibration_changed) {
      setCalibration(interface->getCalibration());
    }
  }

  if (button_irq) {
    button_irq = false;
    interface->serviceButton();
  }

  sum_pot = 0;
  for (i = 0; i < buffer_size; i++) {
    if (i < n_sample) {
      buffer.unshift(analogRead(A0));
      sum_pot += buffer[0];
    } else {
      sum_pot += buffer[i];
    }
  }
  average_pot = (float)sum_pot / buffer_size;
  angle = -(calibrator->getAngle(average_pot));

  if (abs(old_angle - angle) > 0.02 || loop_entry_counter > 0) {
    old_angle = angle;
    frame_change_counter = post_change_frames;
    angle_changed = true;
  } else {
    angle = old_angle;
  }

  if (loop_entry_counter > 0) loop_entry_counter--;

  if (millis() > timer_eph_sample + 50) {
    old_jd_back = jd;
    jd = time_calculator->getJulianDate(angle);
    if (abs(old_eph_angle - angle) > 0.06 || (!angle_changed && angle_changed_eph_old)) {
      angle_changed_eph_old = angle_changed;
      old_data = data;
      old_jd_front = old_jd_back;
      data = getPlanetData(jd);
      old_eph_angle = angle;
      frame_change_counter = post_change_frames;
    } else if (date_change_eph_request) {
      old_data = data;
      old_jd_front = old_jd_back;
      data = getPlanetData(jd);
      frame_change_counter = post_change_frames;
      date_change_eph_request = false;
    }
    ri = getRetrogradeInfo(jd, old_jd_front, data, old_data);
    timer_eph_sample = millis();

    for (i = 0; i < 19; i ++) {
      switch(i) {
        case 0:
          planets_altaz[i] = getAltAz(angle, data.sun_equ_latlong, latitude);
          break;
        case 1:
          planets_altaz[i] = getAltAz(angle, data.moon_equ_latlong, latitude);
          break;
        case 2:
          planets_altaz[i] = getAltAz(angle, data.mercury_equ_latlong, latitude);
          break;
        case 3:
          planets_altaz[i] = getAltAz(angle, data.venus_equ_latlong, latitude);
          break;
        case 4:
          planets_altaz[i] = getAltAz(angle, data.mars_equ_latlong, latitude);
          break;
        case 5:
          planets_altaz[i] = getAltAz(angle, data.jupiter_equ_latlong, latitude);
          break;
        case 6:
          planets_altaz[i] = getAltAz(angle, data.saturn_equ_latlong, latitude);
          break;
        default:
          stars_altaz[i-7] = getAltAz(angle, fixed_star_data[i-7], latitude);
          break;
      }
    }
  }

  if (millis() > timer_interface_update + 501) {
    dt = time_calculator->getDateTime(jd);
    cusps = tympanum->getHouseCusps(angle);
    InterfaceParams p = { .dt = dt, .pd = data, .cusps = cusps, .ri = ri, .planets_altaz = planets_altaz, .stars_altaz = stars_altaz};
    interface->process(p);
    timer_interface_update = millis();
  }

  if (frame_change_counter > 0) {
    frame_change_counter--;
    for (i = 0; i < n_stars; i++) {
      star_long = -star_data[i][0];
      star_r = star_data[i][1];
      star_size = star_data[i][2];
      old_center_x = 360 + cos(erase_angle + star_long) * star_r * scale_factor;
      old_center_y = 360 + sin(erase_angle + star_long) * star_r * scale_factor;
      center_x = 360 + cos(angle + star_long) * star_r * scale_factor;
      center_y = 360 + sin(angle + star_long) * star_r * scale_factor;
      drawCircle(old_center_x, old_center_y, star_size * k_star_size, BG_COLOR, fb, true);
      drawCircle(center_x, center_y, star_size * k_star_size, STAR_COLOR, fb);
    }

    erase_angle = angle;

    for (i = 0; i < n_planets; i++) {
      switch (i) {
        case MOON:
          planet_coords = getStereoCoords(data.moon_equ_latlong.lat, data.moon_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          phase_angle = planet_long - sun_long;
          planet_r = planet_coords.r;
          planet_color = MOON_COLOR;
          planet_size = MOON_SIZE;
          break;
        case SUN:
          planet_coords = getStereoCoords(data.sun_equ_latlong.lat, data.sun_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          sun_long = planet_long;
          planet_r = planet_coords.r;
          planet_color = SUN_COLOR;
          planet_size = SUN_SIZE;
          break;
        case ANTISOLAR:
          planet_coords = getStereoCoords(-(data.sun_equ_latlong.lat), PI + data.sun_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = BG_COLOR;
          planet_size = ANTISOLAR_SIZE;
          break;
        case MERCURY:
          planet_coords = getStereoCoords(data.mercury_equ_latlong.lat, data.mercury_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = MERCURY_COLOR;
          planet_size = PLANET_SIZE;
          break;
        case VENUS:
          planet_coords = getStereoCoords(data.venus_equ_latlong.lat, data.venus_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = VENUS_COLOR;
          planet_size = PLANET_SIZE;
          break;
        case MARS:
          planet_coords = getStereoCoords(data.mars_equ_latlong.lat, data.mars_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = MARS_COLOR;
          planet_size = PLANET_SIZE;
          break;
        case JUPITER:
          planet_coords = getStereoCoords(data.jupiter_equ_latlong.lat, data.jupiter_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = JUPITER_COLOR;
          planet_size = PLANET_SIZE;
          break;
        case SATURN:
          planet_coords = getStereoCoords(data.saturn_equ_latlong.lat, data.saturn_equ_latlong.lon);
          planet_long = -planet_coords.theta;
          planet_r = planet_coords.r;
          planet_color = SATURN_COLOR;
          planet_size = PLANET_SIZE;
          break;
      }
      old_center_x = planet_erase_locs[i][0];
      old_center_y = planet_erase_locs[i][1];
      center_x = 360 + cos(angle + planet_long) * planet_r * scale_factor;
      center_y = 360 + sin(angle + planet_long) * planet_r * scale_factor;

      drawCircle(old_center_x, old_center_y, planet_size, BG_COLOR, fb, true);
      planet_erase_locs[i] = { center_x, center_y };
      switch (i) {
        case MERCURY:
        case VENUS:
        case MARS:
        case JUPITER:
        case SATURN:
          drawPlanet(center_x, center_y, planet_size, planet_color, fb);
          break;
        case ANTISOLAR:
          drawAntisolar(center_x, center_y, planet_size, fb);
          break;
        case SUN:
          drawSun(center_x, center_y, fb);
          sun_center_x = center_x;
          sun_center_y = center_y;
          break;
        case MOON:
          drawMoon(center_x, center_y, sun_center_x, sun_center_y, phase_angle, fb);
          break;
      }
    }
  }
}

bool draw_to_buffer(esp_lcd_panel_handle_t panel_handle, const esp_lcd_rgb_panel_event_data_t* edata, void* user_ctx) {
  esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 720, 720, framebuffer);
  return true;
}
