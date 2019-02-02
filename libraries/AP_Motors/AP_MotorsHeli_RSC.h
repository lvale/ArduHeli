#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>            // ArduPilot Mega Vector/Matrix math Library
#include <RC_Channel/RC_Channel.h>
#include <SRV_Channel/SRV_Channel.h>

// rotor controller states
enum RotorControlState {
    ROTOR_CONTROL_STOP = 0,
    ROTOR_CONTROL_IDLE,
    ROTOR_CONTROL_ACTIVE
};

// rotor control modes
enum RotorControlMode {
    ROTOR_CONTROL_MODE_DISABLED = 0,
    ROTOR_CONTROL_MODE_SPEED_PASSTHROUGH,
    ROTOR_CONTROL_MODE_SPEED_SETPOINT,
    ROTOR_CONTROL_MODE_OPEN_LOOP_POWER_OUTPUT,
    ROTOR_CONTROL_MODE_CLOSED_LOOP_POWER_OUTPUT
};

class AP_MotorsHeli_RSC {
public:
    friend class AP_MotorsHeli_Single;
    friend class AP_MotorsHeli_Dual;

    AP_MotorsHeli_RSC(SRV_Channel::Aux_servo_function_t aux_fn,
                      uint8_t default_channel) :
        _aux_fn(aux_fn),
        _default_channel(default_channel)
    {};

    // init_servo - servo initialization on start-up
    void        init_servo();

    // set_control_mode - sets control mode
    void        set_control_mode(RotorControlMode mode) { _control_mode = mode; }

    // set_critical_speed
    void        set_critical_speed(float critical_speed) { _critical_speed = critical_speed; }

    // get_critical_speed
    float       get_critical_speed() const { return _critical_speed; }

    // set_idle_output
    float       get_idle_output() { return _idle_output; }
    void        set_idle_output(float idle_output) { _idle_output = idle_output; }

    // set engine governor parameters
    void        set_governor_disengage(float governor_disengage) {_governor_disengage = governor_disengage; }
    void        set_governor_droop_setting(float governor_droop_setting) { _governor_droop_setting = governor_droop_setting; }
    void        set_governor_output(float governor_output) {_governor_output = governor_output; }
    void        set_governor_setpoint(float governor_setpoint) { _governor_setpoint = governor_setpoint; }
    void        set_governor_tc(float governor_tc) {_governor_tc = governor_tc; }

    // get_desired_speed
    float       get_desired_speed() const { return _desired_speed; }

    // set_desired_speed
    void        set_desired_speed(float desired_speed) { _desired_speed = desired_speed; }

    // get_control_speed
    float       get_control_output() const { return _control_output; }

    // get_rotor_speed - estimated rotor speed when no governor or rpm sensor is used
    float       get_rotor_speed() const;
    
    // set_rotor_rpm - when speed sensor is available for governor
    void        set_rotor_rpm(int16_t rotor_rpm) {_rotor_rpm = rotor_rpm; }
    
    // get_governor_output
    float       get_governor_output() const { return _governor_output; }

    // is_runup_complete
    bool        is_runup_complete() const { return _runup_complete; }

    // set_ramp_time
    void        set_ramp_time(int8_t ramp_time) { _ramp_time = ramp_time; }

    // set_runup_time
    void        set_runup_time(int8_t runup_time) { _runup_time = runup_time; }

    // set_throttle_curve
    void        set_throttle_curve(float thrcrv[5], uint16_t slewrate);

    // set_collective. collective for throttle curve calculation
    void        set_collective(float collective) { _collective_in = collective; }

    // output - update value to send to ESC/Servo
    void        output(RotorControlState state);

private:
    uint64_t        _last_update_us;

    // channel setup for aux function
    SRV_Channel::Aux_servo_function_t _aux_fn;
    uint8_t         _default_channel;

    // internal variables
    RotorControlMode _control_mode = ROTOR_CONTROL_MODE_DISABLED;   // motor control mode, Passthrough or Setpoint
    float           _critical_speed = 0.0f;         // rotor speed below which flight is not possible
    float           _idle_output = 0.0f;            // motor output idle speed
    float           _desired_speed = 0.0f;          // latest desired rotor speed from pilot
    float           _control_output = 0.0f;         // latest logic controlled output
    float           _rotor_ramp_output = 0.0f;      // scalar used to ramp rotor speed between _rsc_idle_output and full speed (0.0-1.0f)
    float           _rotor_runup_output = 0.0f;     // scalar used to store status of rotor run-up time (0.0-1.0f)
    int8_t          _ramp_time = 0;                 // time in seconds for the output to the main rotor's ESC to reach full speed
    int8_t          _runup_time = 0;                // time in seconds for the main rotor to reach full speed.  Must be longer than _rsc_ramp_time
    bool            _runup_complete = false;        // flag for determining if runup is complete
    float           _thrcrv_poly[4][4];             // spline polynomials for throttle curve interpolation
    uint16_t        _power_slewrate = 0;            // slewrate for throttle (percentage per second)
    float           _collective_in;                 // collective in for throttle curve calculation, range 0-1.0f
    int16_t         _rotor_rpm;                     // rotor rpm from speed sensor for governor
    float           _governor_disengage = 0.0f;     // throttle percentage where governor disenages to allow return to flight idle
    float           _governor_droop_setting = 0.0f; // governor droop setting, range 0-100%
    float           _governor_output = 0.0f;        // governor output for rotor speed control
    int16_t         _governor_setpoint = 0.0f;      // governor speed setpoint, range 800-3500 rpm
    float           _governor_status = 0.0f;        // RSC governor status for soft-start
    float           _governor_tc = 0.0f;            // governor throttle curve gain, range 50-100%

    // update_rotor_ramp - slews rotor output scalar between 0 and 1, outputs float scalar to _rotor_ramp_output
    void            update_rotor_ramp(float rotor_ramp_input, float dt);

    // update_rotor_runup - function to slew rotor runup scalar, outputs float scalar to _rotor_runup_ouptut
    void            update_rotor_runup(float dt);

    // write_rsc - outputs pwm onto output rsc channel. servo_out parameter is of the range 0 ~ 1
    void            write_rsc(float servo_out);

    // calculate_desired_throttle - uses throttle curve and collective input to determine throttle setting
    float           calculate_desired_throttle(float collective_in);
};
