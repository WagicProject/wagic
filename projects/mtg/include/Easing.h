#ifndef _EASING_H_
#define _EASING_H_

/*! \brief A class for eased floats for use in animations
 *
 * Animations often defines values a floating point variable
 * should have at given times and interpolates between them to
 * calculate the value of that variable at any given intermediate
 * time step.
 *
 * The simplest case would be linear interpolation:
 * Suppose a float "position" should be a at time = 0 and
 * b at time = x. If the current time is y, the value of
 * "position" is then a + (b-a)*y/x.
 *
 * This class defines the interface needed to implement different
 * kind of interpolations with a common interface. See
 * http://www.gizma.com/easing/ for more information for a few
 * examples.
 */
class Easing
{
public:
    /*! \brief The value at the start of an animation.
     *
     * start_value is undefined if no animation is running.
     */
    float start_value;

    /*! \brief The amount the value should change during the animation.
     *
     * delta_value is undefined if no animation is running.
     */
    float delta_value;

    /*! \brief The current value.
     *
     * Use this member to read the value or to write the value without
     * to animate intermediate values and. Make sure that the easing
     * is not used once value is deleted.
     */
    float& value;

    /*! \brief The duration the animation should take
     *
     * It is not relevant which unit is used. This value is undefined
     * if no animation is running.
     */
    float duration;

    /*! \brief The accumulated time the animation did run until now.
     *
     * It is not relevant which unit is used. This values is undefined
     * if no animation is running.
     */
    float time_acc;

    /*! \brief Sets Easing::float to val and sets the animation as not running.
     *
     * Make sure that the easing is not used once value is deleted.
     *
     * \param val The value to ease
     */
    Easing(float& val): start_value(val), delta_value(0), value(val), duration(0), time_acc(0)
    {
    }

    /*! \brief Resets the animation to its initial value
     *
     * This method does set the value to the start value and sets the passed time to 0.
     * If there is no animation animation running, the resulting value is undefined.
     */
    void reset()
    {
        value = start_value;
        time_acc = 0;
    }

    /*! \brief Finishes the animation immediately
     *
     * Sets the value to the animations target value and the passed time to the
     * animations duration. If there is no animation running, the behaviour is undefined.
     */
    void finish()
    {
        value = start_value + delta_value;
        time_acc = duration;
    }

    /*! \brief Lets dt time pass
     *
     * Advances the animation by dt time units and updates the value accordingly.
     *
     * \val dt The amount of time to jump forward
     */
    void update(float dt)
    {
        if(duration > 0)
        {
            time_acc += dt;

            if(time_acc > duration)
            {
                time_acc = duration;
                value = start_value + delta_value;
            }
            else
            {
                updateValue();
            }
        }
    }

    /*! \brief Calculates the value from all other members.
     *
     * This method gets implemented by all specific easing classes.
     */
    virtual void updateValue() = 0;

    /*! \brief Starts the animation.
     *
     * Starts the interpolation from the current value (now) to
     * targetValue (in now + _duration).
     *
     * If the animation is currently running, it gets replaced.
     *
     * \param targetValue The value to interpolate to
     * \param _duration   The duration the interpolation should take
     */
    void start(float targetValue, float _duration)
    {
        start_value = value;
        delta_value = targetValue - start_value;
        time_acc = 0;
        duration = _duration;
    }

    /*! \brief Translates the current value and the target value by delta_value
     *
     * This method is mainly used for trickery. Suppose there is one object in the
     * middle of the screen that should move to the top until it is outside of the
     * screen and gets replaced by a second one entering the screen from the lower
     * side once the first one disappeared. This method can be used to simulate this
     * effect with one animation by translating (i.e. moving) the animation from the
     * top to the bottom:
     *
     * Object1 and object2 are the same object: object1 whose y position is bound to value
     * To start the transition, use start(SCREEN_HEIGHT, desired time); Once the first
     * object left the screen (i.e. object.y < 0), change objects appearance to object2
     * and translate the easing by (SCREEN_HEIGHT).
     *
     * \param delta_value The change in start_value and value
     */
    void translate(float delta_value)
    {
        start_value += delta_value;
        value += delta_value;
    }

    /*! \brief Returns if the passed time exceeds duration.
     *
     * If ther is no animation running, it is ensured that this is true.
     */
    bool finished()
    {
        return time_acc >= duration;
    }
};

/*! \brief This class defines an easing with quadratic acceleration and decceleration.
 */
class InOutQuadEasing : public Easing
{
public:
    /*! \brief Calls Easing::Easing(val).
     *
     * \see Easing::Easing(float& val)
     */
    InOutQuadEasing(float& val): Easing(val) {}

    /*! \brief Implements the value calculation.
     *
     * \see Easing::updateValue()
     */
    void updateValue()
    {
        float time_tmp = (time_acc * 2) / duration;
        if (time_tmp < 1)
        {
            value = delta_value * 0.5 * time_tmp * time_tmp + start_value;
        }
        else
        {
            time_tmp -= 1;
            value = - delta_value * 0.5 * (time_tmp * (time_tmp - 2) - 1) + start_value;
        }
    }
};


#endif //_EASING_H_
