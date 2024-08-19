/**
    @file
    update_laser_angle: outputs DMX messages to update the y-axis angle of the laser array
*/

#include "ext.h"            // standard Max include, always required (except in Jitter)
#include "ext_obex.h"        // required for "new" style objects
#include "z_dsp.h"            // required for MSP objects
#include "math.h"


// struct to represent the object's state
typedef struct _update_laser_angle {
    t_pxobject        ob;            // The object itself (t_pxobject in MSP instead of t_object)
    
    long   *dmx_value_outlet_;
    long   *dmx_channel_outlet_;
    long   dmx_start_channel_;
    long   new_angle_;
} t_update_laser_angle;


// method prototypes
void update_laser_angle_bang(t_update_laser_angle *x);
void update_laser_angle_int(t_update_laser_angle *x, long n);
void update_laser_angle_in1(t_update_laser_angle *x, long n);
void update_laser_angle_assist(t_update_laser_angle *x, void *b, long m, long a, char *s);
void *update_laser_angle_new();
void update_laser_angle_output_dmx_messages(t_update_laser_angle *x);

static t_class *update_laser_angle_class = NULL;       // Global class pointer variable

//***********************************************************************************************

void ext_main(void *r)
{
    t_class *c;

    c = class_new("update_laser_angle", (method)update_laser_angle_new, (method)NULL, sizeof(t_update_laser_angle), 0L, A_DEFLONG, 0);

    class_addmethod(c, (method)update_laser_angle_bang, "bang", 0);
    class_addmethod(c, (method)update_laser_angle_int, "int", A_LONG, 0);
    class_addmethod(c, (method)update_laser_angle_in1, "in1", A_LONG, 0);
    class_addmethod(c, (method)update_laser_angle_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);
    update_laser_angle_class = c;
}

void *update_laser_angle_new()
{
    t_update_laser_angle *x;      // Local variable (pointer to a t_update_laser_angle data structure)

    x = (t_update_laser_angle *)object_alloc(update_laser_angle_class);     // Create a new instance of this object
    
    intin(x, 1);
    
    // Right-to-left float outlets:
    x->dmx_channel_outlet_ = intout((t_object *)x);
    x->dmx_value_outlet_ = intout((t_object *)x);
    
    // Initialise parameters:
    x->dmx_start_channel_ = 0;
    x->new_angle_ = 0;
    
    return(x); // Return a reference to the object instance
}

void update_laser_angle_free(t_update_laser_angle *x)
{
}


void update_laser_angle_assist(t_update_laser_angle *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)      // Inlets:
    {
        switch (a)
        {
            case 0:
                sprintf(s, "(int) New y-axis angle, (bang) sends output");
                break;
            case 1:
                sprintf(s, "(int) First DMX channel of the laser array");
                break;
        }
    }
    else        // Outlets:
    {
        switch (a)
        {
            case 0:
                sprintf(s, "(int) DMX value");
                break;
            case 1:
                sprintf(s, "(int) DMX channel");
                break;
        }
    }
}

void update_laser_angle_bang(t_update_laser_angle *x)
{
    update_laser_angle_output_dmx_messages(x);
}

void update_laser_angle_int(t_update_laser_angle *x, long n)
{
    x->new_angle_ = n;
}

void update_laser_angle_in1(t_update_laser_angle *x, long n)
{
    x->dmx_start_channel_ = n;
}

void update_laser_angle_output_dmx_messages(t_update_laser_angle *x)
{
    short channel = x->dmx_start_channel_ + 3; // y-axis position is the 4th DMX value
    
    outlet_int(x->dmx_channel_outlet_, channel); // Channel should be first (right outlet) so the left outlet bangs after the right updates
    outlet_int(x->dmx_value_outlet_, x->new_angle_);
}
