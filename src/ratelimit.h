#ifndef slewratelimit_h
#define slewratelimit_h


class SlewRateLimit {
  public:
    SlewRateLimit(float limit);
    float process(float next_value);


  private:
    float _limit;
    float _current_output;
};

#endif
