#ifndef INPUTPARAM_H
#define INPUTPARAM_H

#include <string>

class inputparam
{
public:
    std::string MODE_COMMAND_TimePoint = "timepoint";
    std::string MODE_COMMAND_Timer = "timer";
    static constexpr int INPUT_UNASSIGNED = -1;

public:
    int mode;
    int hours;
    int minutes;
    int seconds;

public:
    inline void inhaleParameter(char* in_mode, char* in_hours, char* in_minutes, char* in_seconds)
    {
        int notNullPtr = 0;
        std::string tempMode;
        std::string tempHours;
        std::string tempMinutes;
        std::string tempSeconds;

        if(in_mode != nullptr)
        {
            tempMode = in_mode;
            notNullPtr++;
        }
        if(in_hours != nullptr)
        {
         tempHours = in_hours;
         notNullPtr++;
        }
        if(in_minutes)
        {
        tempMinutes = in_minutes;
        notNullPtr++;
        }
        if(in_seconds != nullptr)
        {
        tempSeconds = in_seconds;
        notNullPtr++;
        }

        if(!tempMode.empty())
        {
            if(tempMode == MODE_COMMAND_TimePoint)
            {
                this->mode = 0;
            }
            if(tempMode == MODE_COMMAND_Timer)
            {
                this->mode = 1;
            }
            else
            {
                this->mode = INPUT_UNASSIGNED;
            }
        }
        else
        {
            this->mode = INPUT_UNASSIGNED;
        }

        if(!tempHours.empty())
        {
            this->hours = std::atoi(in_hours);
        }
        else
        {
            this->hours = INPUT_UNASSIGNED;
        }

        if(!tempMinutes.empty())
        {
            this->minutes = std::atoi(in_minutes);
        }
        else
        {
            this->minutes = INPUT_UNASSIGNED;
        }

        if(!tempSeconds.empty())
        {
            this->seconds = std::atoi(in_seconds);
        }
        else
        {
            this->seconds = INPUT_UNASSIGNED;
        }
    }
    inline void reset()
    {
        this->mode = INPUT_UNASSIGNED;
        this->hours = INPUT_UNASSIGNED;
        this->minutes = INPUT_UNASSIGNED;
        this->seconds = INPUT_UNASSIGNED;
    }
};

#endif // INPUTPARAM_H
