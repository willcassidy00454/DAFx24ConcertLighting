/**
    @file
    audio_levels_to_dmx~: reads 10 audio levels (float -1 to 1) and outputs a stream of DMX messages (0-255) with changing intensity, given a certain global colour.
*/

#include "ext.h"            // standard Max include, always required (except in Jitter)
#include "ext_obex.h"        // required for "new" style objects
#include "z_dsp.h"            // required for MSP objects
#include "math.h"


// struct to represent the object's state
typedef struct _audio_levels_to_dmx {
    t_pxobject        ob;            // The object itself (t_pxobject in MSP instead of t_object)
    
    long   *dmx_value_outlet_;
    long   *dmx_channel_outlet_;
    float   ls_levels_[10];
    float   audio_level_floor_;
    float   audio_level_ceiling_;
    long    rgb_[3];
    long    num_fixtures_;
    long    mapping_mode_;
    long    channel_offset_;
    float   sharp_crossfade_exponent_;
    long    NUM_LOUDSPEAKERS_;
} t_audio_levels_to_dmx;


// method prototypes
void audio_levels_to_dmx_bang(t_audio_levels_to_dmx *x);
void audio_levels_to_dmx_list(t_audio_levels_to_dmx *x, t_symbol *s, long argc, t_atom *argv);
void audio_levels_to_dmx_ft1(t_audio_levels_to_dmx *x, double f);
void audio_levels_to_dmx_ft2(t_audio_levels_to_dmx *x, double f);
void audio_levels_to_dmx_ft3(t_audio_levels_to_dmx *x, double f);
void audio_levels_to_dmx_in1(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_in2(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_in3(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_in4(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_in5(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_in6(t_audio_levels_to_dmx *x, long n);
void audio_levels_to_dmx_assist(t_audio_levels_to_dmx *x, void *b, long m, long a, char *s);
void *audio_levels_to_dmx_new();
void audio_levels_to_dmx_output_dmx_messages(t_audio_levels_to_dmx *x);
void audio_levels_to_dmx_scale_and_clamp(t_audio_levels_to_dmx *x, float *value, float from_min, float from_max, float to_min, float to_max);
float audio_levels_to_dmx_cosine_interp(float value);

static t_class *audio_levels_to_dmx_class = NULL;       // Global class pointer variable

//***********************************************************************************************

void ext_main(void *r)
{
    t_class *c;

    c = class_new("audio_levels_to_dmx", (method)audio_levels_to_dmx_new, (method)NULL, sizeof(t_audio_levels_to_dmx), 0L, A_DEFLONG, 0);

    class_addmethod(c, (method)audio_levels_to_dmx_bang, "bang", 0);
    class_addmethod(c, (method)audio_levels_to_dmx_list, "list", A_GIMME, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_ft1, "ft1", A_FLOAT, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_ft2, "ft2", A_FLOAT, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in1, "in1", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in2, "in2", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in3, "in3", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in4, "in4", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in5, "in5", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_in6, "in6", A_LONG, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_ft3, "ft3", A_FLOAT, 0);
    class_addmethod(c, (method)audio_levels_to_dmx_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);
    audio_levels_to_dmx_class = c;
}

void *audio_levels_to_dmx_new()
{
    t_audio_levels_to_dmx *x;      // Local variable (pointer to a t_audio_levels_to_dmx data structure)

    x = (t_audio_levels_to_dmx *)object_alloc(audio_levels_to_dmx_class);     // Create a new instance of this object
    
    floatin(x, 3);
    intin(x, 6);
    intin(x, 5);
    intin(x, 4);
    intin(x, 3);
    intin(x, 2);
    intin(x, 1);
    floatin(x, 2);
    floatin(x, 1);
    
    // Right-to-left float outlets:
    x->dmx_channel_outlet_ = intout((t_object *)x);
    x->dmx_value_outlet_ = intout((t_object *)x);
    
    // Initialise parameters:
    for (short i = 0; i < 10; ++i)
        x->ls_levels_[i] = 0.0f;
    
    x->audio_level_ceiling_ = 1.0f;
    x->audio_level_floor_ = 0.0f;
    
    for (short i = 0; i < 3; ++i)
        x->rgb_[i] = 0;
    
    x->num_fixtures_ = 60;
    x->mapping_mode_ = 1;
    x->sharp_crossfade_exponent_ = 5.0f;
    x->NUM_LOUDSPEAKERS_ = 10;
    x->channel_offset_ = 0;
    
    return(x);      // Return a reference to the object instance
}

void audio_levels_to_dmx_free(t_audio_levels_to_dmx *x)
{
}


void audio_levels_to_dmx_assist(t_audio_levels_to_dmx *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)      // Inlets:
    {
        switch (a)
        {
            case 0:
                sprintf(s, "(float list 10) Loudspeaker levels, (bang) sends output");
                break;
            case 1:
                sprintf(s, "(float) Audio level floor");
                break;
            case 2:
                sprintf(s, "(float) Audio level ceiling");
                break;
            case 3:
                sprintf(s, "(int) Red");
                break;
            case 4:
                sprintf(s, "(int) Green");
                break;
            case 5:
                sprintf(s, "(int) Blue");
                break;
            case 6:
                sprintf(s, "(int) Number of light fixtures");
                break;
            case 7:
                sprintf(s, "(int) Mapping mode: 1 = equal magnitude, 2 = linear crossfade, 3 = cosine crossfade, 4 = sharp crossfade");
                break;
            case 8:
                sprintf(s, "(int) Channel offset");
                break;
            case 9:
                sprintf(s, "(float) Sharp crossfade exponent");
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

void audio_levels_to_dmx_bang(t_audio_levels_to_dmx *x)
{
    audio_levels_to_dmx_output_dmx_messages(x);
}

void audio_levels_to_dmx_list(t_audio_levels_to_dmx *x, t_symbol *s, long argc, t_atom *argv)
{
    t_atom *atom_iterator = argv;

    // increment ap each time to get to the next atom
    for (short element_pos = 0; element_pos < argc; ++element_pos, ++atom_iterator)
    {
        if (atom_gettype(atom_iterator) == A_FLOAT)
            x->ls_levels_[element_pos] = atom_getfloat(atom_iterator);
    }
}

void audio_levels_to_dmx_ft1(t_audio_levels_to_dmx *x, double f)
{
    x->audio_level_floor_ = f;
}

void audio_levels_to_dmx_ft2(t_audio_levels_to_dmx *x, double f)
{
    x->audio_level_ceiling_ = f;
}

void audio_levels_to_dmx_in1(t_audio_levels_to_dmx *x, long n)
{
    x->rgb_[0] = n;
}

void audio_levels_to_dmx_in2(t_audio_levels_to_dmx *x, long n)
{
    x->rgb_[1] = n;
}

void audio_levels_to_dmx_in3(t_audio_levels_to_dmx *x, long n)
{
    x->rgb_[2] = n;
}

void audio_levels_to_dmx_in4(t_audio_levels_to_dmx *x, long n)
{
    x->num_fixtures_ = n;
}

void audio_levels_to_dmx_in5(t_audio_levels_to_dmx *x, long n)
{
    if (n > 0 && n < 5)
        x->mapping_mode_ = n;
    else
        x->mapping_mode_ = 1;
}

void audio_levels_to_dmx_in6(t_audio_levels_to_dmx *x, long n)
{
    x->channel_offset_ = n;
}

void audio_levels_to_dmx_ft3(t_audio_levels_to_dmx *x, double f)
{
    x->sharp_crossfade_exponent_ = CLAMP(f, 1.0f, 30.0f);
}

void audio_levels_to_dmx_output_dmx_messages(t_audio_levels_to_dmx *x)
{
    // For each fixture set (num fixtures / num loudspeakers)
        // For each fixture
            // For each DMX channel
                // Calculate intensity
                // Scale the global RGB values
                // Output the DMX value and channel number
    
    // Each fixture set sits between two loudspeakers
    
    short dmx_channel = 1;
    short num_fixtures_per_set = (short)((float)x->num_fixtures_ / (float)x->NUM_LOUDSPEAKERS_);//x->num_fixture_sets_);
    float intensity = 0.0f;
    
    for (short fixture_set_index = 0; fixture_set_index < x->NUM_LOUDSPEAKERS_; ++fixture_set_index)
    {
        short ls_index = (fixture_set_index + x->channel_offset_) % x->NUM_LOUDSPEAKERS_;//x->num_fixture_sets_;
        float loudspeaker_level_left = x->ls_levels_[ls_index];
        float loudspeaker_level_right = x->ls_levels_[(ls_index + 1) % x->NUM_LOUDSPEAKERS_];//x->num_fixture_sets_]; // Wrap the right LS channel back to 0 if operating on the last pair
        
        audio_levels_to_dmx_scale_and_clamp(x, &loudspeaker_level_left, x->audio_level_floor_, x->audio_level_ceiling_, 0.0f, 1.0f);
        audio_levels_to_dmx_scale_and_clamp(x, &loudspeaker_level_right, x->audio_level_floor_, x->audio_level_ceiling_, 0.0f, 1.0f);
        
        if (x->mapping_mode_ == 1)
            intensity = (loudspeaker_level_left + loudspeaker_level_right) / 2.0f; // Same intensity for all lights in each fixture set
        
        for (short fixture_index = 0; fixture_index < num_fixtures_per_set; ++fixture_index)
        {
            if (x->mapping_mode_ > 1)
            {
                float increasing_lerp = (float)fixture_index / (float)(num_fixtures_per_set); // This will linearly interpolate from 0-(almost)1
                float decreasing_lerp = 1.0f - increasing_lerp; // This will linearly interpolate from 1-(almost)0
                
                if (x->mapping_mode_ > 2)
                {
                    intensity = (loudspeaker_level_left * audio_levels_to_dmx_cosine_interp(decreasing_lerp)
                                 + loudspeaker_level_right * audio_levels_to_dmx_cosine_interp(increasing_lerp)) / 2.0f;
                    
                    if (x->mapping_mode_ == 4)
                        intensity = (intensity + powf(intensity, x->sharp_crossfade_exponent_)) / 2.0f;
                }
                else
                {
                    intensity = (loudspeaker_level_left * decreasing_lerp + loudspeaker_level_right * increasing_lerp) / 2.0f;
                }
            }
            
            intensity = CLAMP(intensity, 0.0f, 1.0f);
            
            // Output RGB values for current intensity
            for (short rgb_index = 0; rgb_index < 3; ++rgb_index)
            {
                int value = (int)(roundf(intensity * (float)x->rgb_[rgb_index])); // R/G/B output values are scaled by the intensity
                
                outlet_int(x->dmx_channel_outlet_, dmx_channel); // Channel should be first (right outlet) so the left outlet bangs after the right updates
                outlet_int(x->dmx_value_outlet_, value);
                
                dmx_channel++;
            }
        }
    }
}

void audio_levels_to_dmx_scale_and_clamp(t_audio_levels_to_dmx *x, float *value, float from_min, float from_max, float to_min, float to_max)
{
    float input_range = from_max - from_min;
    float normalised_input = (*value - from_min) / input_range;
    
    float output_range = to_max - to_min;
    float scaled_output = normalised_input * output_range + to_min;
    
    CLAMP(scaled_output, to_min, to_max);
    
    *value = scaled_output;
}

float audio_levels_to_dmx_cosine_interp(float value)
{
    return (-cosf(PI * value) + 1.0f) / 2.0f;
}
