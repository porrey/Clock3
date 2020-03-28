#ifndef MODE_H
#define MODE_H

class Mode
{
  public:
    // ***
    // *** Defines the various modes.
    // ***
    enum MODE { MODE_DISPLAY_TIME = 0, MODE_TZ = 1, MODE_DST = 2, MODE_CHIME = 3, MODE_BATTERY = 4 , MODE_MAX = 5 };

    // ***
    // *** Creates an instance with the specified intial mode.
    // ***
    Mode(uint8_t initialMode)
    {
      this->_mode = initialMode;
    };

    // ***
    // *** Gets the current mode.
    // ***
    const uint8_t getMode()
    {
      return this->_mode;
    }

    // ***
    // *** Sets the current mode.
    // ***
    void setMode(uint8_t mode)
    {
      this->_mode = mode;
      this->_modeChanged = true;
    }

    // ***
    // *** Increments the current mode to
    // *** the next mode.
    // ***
    void increment()
    {
      this->setMode(++this->_mode % MODE_MAX);
    }

    // ***
    // *** Gets a value indicating if the mode
    // *** has changed or not.
    // ***
    const bool modeChanged()
    {
      return this->_modeChanged ;
    }

    // ***
    // *** Sets the mode changed flag.
    // ***
    void modeChanged(bool changed)
    {
      this->_modeChanged = changed;
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

  protected:
    // ***
    // *** Stores the current mode.
    // ***
    uint8_t _mode;

    // ***
    // *** The mode changed flag.
    // ***
    bool _modeChanged = false;

    // ***
    // *** The setup changed flag.
    // ***
    bool _setupChanged = false;
};
#endif
