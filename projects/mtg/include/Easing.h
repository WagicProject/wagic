#ifndef _EASING_H_
#define _EASING_H_

//see http://www.gizma.com/easing/ for more information
class Easing
{
public:
    float start_value;
    float delta_value;
    float value;
    float duration;
    float time_acc;


    Easing(float val): start_value(val), delta_value(0), value(val), duration(0), time_acc(0)
    {
    }

    void reset()
    {
        value = start_value;
        time_acc = 0;
    }

    void finish()
    {
        value = start_value + delta_value;
        time_acc = 0; duration = 0;
    }

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

    virtual void updateValue() = 0;

    void start(float targetValue, float _duration)
    {
        start_value = value;
        delta_value = targetValue - start_value;
        time_acc = 0;
        duration = _duration;
    }

    void translate(float delta_value)
    {
        start_value += delta_value;
        value += delta_value;
    }

    bool finished()
    {
        return time_acc >= duration;
    }
};

class InOutQuadEasing : public Easing
{
public:
    InOutQuadEasing(float val): Easing(val) {}

    void updateValue()
    {
        float time_tmp = time_acc * 2 / duration;
        if (time_tmp < 1)
        {
            value = delta_value/2*time_tmp*time_tmp + start_value;
        }
        else
        {
            time_tmp -= 1;
            value = -delta_value/2 * (time_tmp*(time_tmp-2) - 1) + start_value;
        }
    }
};


#endif //_EASING_H_
