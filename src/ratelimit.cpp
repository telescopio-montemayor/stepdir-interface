#include "ratelimit.h"



SlewRateLimit::SlewRateLimit(float limit) {
  _limit = limit;
  _current_output = 0;
}


float SlewRateLimit::process(float next_value) {
  float delta = next_value - _current_output;

  if (!_limit) {
    _current_output = next_value;
    return _current_output;
  }

  if (delta > _limit) {
    _current_output = _current_output + _limit;
  }

  else if (delta < -1.0 *_limit) {
    _current_output = _current_output - _limit;
  }

  else {
    _current_output = next_value;
  }

  return _current_output;
}
