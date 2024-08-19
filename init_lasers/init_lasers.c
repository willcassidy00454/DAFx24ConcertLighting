/**
    @file
    init_lasers: initialises the lasers from a startong DMX channel with RGB values and intensity, as well as pre-defined laser parameters
*/

#include "ext.h"            // standard Max include, always required (except in Jitter)
#include "ext_obex.h"        // required for "new" style objects
#include "z_dsp.h"            // required for MSP objects
#include "math.h"


// struct to represent the object's state
typedef struct _init_lasers {
    t_pxobject        ob;            // The object itself (t_pxobject in MSP instead of t_object)
    
    long   *dmx_value_outlet_;
    long   *dmx_channel_outlet_;
    long   dmx_start_channel_;
    short  dmx_values_[9];
} t_init_lasers;


// method prototypes
void init_lasers_bang(t_init_lasers *x);
void init_lasers_in1(t_init_lasers *x, long n);
void init_lasers_assist(t_init_lasers *x, void *b, long m, long a, char *s);
void *init_lasers_new();
void init_lasers_output_dmx_messages(t_init_lasers *x);

static t_class *init_lasers_class = NULL;       // Global class pointer variable

//***********************************************************************************************

void ext_main(void *r)
{
    t_class *c;

    c = class_new("init_lasers", (method)init_lasers_new, (method)NULL, sizeof(t_init_lasers), 0L, A_DEFLONG, 0);

    class_addmethod(c, (method)init_lasers_bang, "bang", 0);
    class_addmethod(c, (method)init_lasers_in1, "in1", A_LONG, 0);
    class_addmethod(c, (method)init_lasers_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);
    init_lasers_class = c;
}

void *init_lasers_new()
{
    t_init_lasers *x;      // Local variable (pointer to a t_init_lasers data structure)

    x = (t_init_lasers *)object_alloc(init_lasers_class);     // Create a new instance of this object
    
    intin(x, 1);
    
    // Right-to-left float outlets:
    x->dmx_channel_outlet_ = intout((t_object *)x);
    x->dmx_value_outlet_ = intout((t_object *)x);
    
    // Initialise parameters:
    x->dmx_start_channel_ = 0;
    
    short mode = 150; // 155-199 for static, 200-255 for dynamic pattern
    short pattern_selection = 62; // fast hor scan=62, slow hor scan=68
    short x_axis_pos = 0; // 0-10 for centred x-axis, 11-255 for x-axis positioning
    short y_axis_pos = 0; // 0-10 for centred y-axis, 11-255 for y-axis positioning
    short scanning_speed = 0; // fastest scan=0
    short dyn_scanning_speed = 0; // unused
    short zoom = 0; // widest fan angle=0
    short colour = 10; // combination=0-9, blue=10-11, red=12-13, green=14-15, magenta=22-27, cyan=28-33, yellow=34-39, repeats after
    short colour_segment = 0; // unused
 
    x->dmx_values_[0] = mode;
    x->dmx_values_[1] = pattern_selection;
    x->dmx_values_[2] = x_axis_pos;
    x->dmx_values_[3] = y_axis_pos;
    x->dmx_values_[4] = scanning_speed;
    x->dmx_values_[5] = dyn_scanning_speed;
    x->dmx_values_[6] = zoom;
    x->dmx_values_[7] = colour;
    x->dmx_values_[8] = colour_segment;
    
    return(x); // Return a reference to the object instance
}

void init_lasers_free(t_init_lasers *x)
{
}


void init_lasers_assist(t_init_lasers *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)      // Inlets:
    {
        switch (a)
        {
            case 0:
                sprintf(s, "(bang) sends output");
                break;
            case 4:
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

void init_lasers_bang(t_init_lasers *x)
{
    init_lasers_output_dmx_messages(x);
}

void init_lasers_in1(t_init_lasers *x, long n)
{
    x->dmx_start_channel_ = n;
}

void init_lasers_output_dmx_messages(t_init_lasers *x)
{
    short absolute_channel_counter = x->dmx_start_channel_;
    
    // Output the 9 DMX values for one laser fixture
    for (short relative_dmx_channel = 0; relative_dmx_channel < 9; ++relative_dmx_channel)
    {
        outlet_int(x->dmx_channel_outlet_, absolute_channel_counter); // Channel should be first (right outlet) so the left outlet bangs after the right updates
        outlet_int(x->dmx_value_outlet_, x->dmx_values_[relative_dmx_channel]);
        
        absolute_channel_counter++;
    }
}
