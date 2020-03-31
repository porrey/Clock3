#ifndef MODE_H
#define MODE_H

// ***
// *** Defines the various modes.
// ***
typedef enum MODE : uint8_t
{
  MODE_DISPLAY_TIME = 0,
  MODE_TZ = 1,
  MODE_DST = 2,
  MODE_CHIME = 3,
  MODE_MAX = 4
} Mode_t;

class Mode
{
  public:
    // ***
    // *** Creates an instance with the specified default mode
    // *** and mode timeout (in seconds);
    // ***
    Mode(Mode_t defaultMode, uint8_t timeout)
    {
      this->_defaultMode = defaultMode;
      this->_mode = defaultMode;

      // ***
      // *** Convert timeout to milliseconds.
      // ***
      this->_timeout = timeout * 1000;
    };

    // ***
    // *** Gets the current mode.
    // ***
    const Mode_t mode()
    {
      return this->_mode;
    }

    // ***
    // *** Sets the current mode.
    // ***
    void mode(Mode_t mode)
    {
      this->_mode = mode;
      this->modeChanged(true);
    }

    // ***
    // *** Increments the current mode to
    // *** the next mode.
    // ***
    void increment()
    {
      Mode_t nextMode = this->_mode + 1;
      nextMode = nextMode % Mode_t::MODE_MAX;
      this->mode(nextMode);
    }

    // ***
    // *** Gets a value indicating if the mode
    // *** has changed or not.
    // ***
    const bool modeChanged()
    {
      return this->_modeChanged;
    }

    // ***
    // *** Sets the mode changed flag.
    // ***
    void modeChanged(bool changed)
    {
      this->_modeChanged = changed;
      this->_timer = millis();
    }

    // ***
    // *** Gets a value indicating if the setup
    // *** has changed or not.
    // ***
    const bool setupChanged()
    {
      return this->_setupChanged;
    }

    // ***
    // *** Sets the setup changed flag.
    // ***
    void setupChanged(bool changed)
    {
      this->_setupChanged = changed;
      this->_timer = millis();
    }

    // ***
    // *** Gets a value that indicates if either
    // *** the mode or the setup has changed.
    // ***
    const bool anyChanged()
    {
      return this->modeChanged() || this->setupChanged();
    }

    // ***
    // *** Resets the mode and setup changed flags.
    // ***
    void reset()
    {
      this->_modeChanged = false;
      this->_setupChanged = false;
    }

    // ***
    // *** Called in loop() to check mode timeout.
    // ***
    bool process()
    {
      bool returnValue = false;

      // ***
      // *** Chck if indefault more.
      // ***
      if (this->_mode != this->_defaultMode)
      {
        // ***
        // *** Check if the timeout period has elapsed.
        // ***
        if ((millis() - this->_timer) > this->_timeout)
        {
          // ***
          // *** Switch back to the default mode.
          // ***
          this->mode(this->_defaultMode);
          returnValue = true;
        }
      }

      return returnValue;
    }

  protected:
    // ***
    // *** Stores the current mode.
    // ***
    Mode_t _mode;

    // ***
    // *** This is the mode the clock
    // *** should be in normally.
    // ***
    Mode_t _defaultMode;

    // ***
    // *** The mode changed flag.
    // ***
    bool _modeChanged = false;

    // ***
    // *** The setup changed flag.
    // ***
    bool _setupChanged = false;

    // ***
    // *** The amount of time, in seconds, that the mode
    // *** will switch back to the default mode if no changes
    // *** are detected.
    // ***
    uint16_t _timeout;

    // ***
    // *** Tracks the last mode or setup change.
    // ***
    uint64_t _timer;
};
#endif
