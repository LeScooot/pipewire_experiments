#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include <spa/pod/builder.h>
#include <spa/param/latency-utils.h>

#include <pipewire/pipewire.h>
#include <pipewire/filter.h>

#define NUM_SAMPLES  1024
#define ORDER 7

static float b[ORDER/2][ORDER]  = {{0.00152573, 0.00305145, 0.00152573},
{1.00000000, 2.00000000, 1.00000000},
{1.00000000, 1.00000000, 0.00000000},
};

static float a[ORDER/2][ORDER]  = {{1.00000000, -0.49314543, 0.00000000},
{1.00000000, -1.06312124, 0.34003499},
{1.00000000, -1.33547945, 0.68333501}
};


struct data;

struct port {
        struct data *data;
};

struct data {
        struct pw_main_loop *loop;
        struct pw_filter *filter;
        struct port *in_port_l;
        struct port *in_port_r;
        struct port *out_port_l;
        struct port *out_port_r;
};

/* [on_process] */
static void on_process(void *userdata, struct spa_io_position *position)
{
        
        static float l_buffer[NUM_SAMPLES + ORDER] =  {0};
        static float r_buffer[NUM_SAMPLES + ORDER] =  {0};
        static float memory_in_l[ORDER/2][2] = {0};
        static float memory_in_r[ORDER/2][2] = {0};
        static float memory_out_l[ORDER/2][2] = {0};
        static float memory_out_r[ORDER/2][2] = {0};




        struct data *data = userdata;
        float *in_l, *in_r, *out_l, *out_r;
        uint32_t n_samples = position->clock.duration;

        pw_log_trace("do process %d", n_samples);

        in_l = pw_filter_get_dsp_buffer(data->in_port_l, n_samples);
        in_r = pw_filter_get_dsp_buffer(data->in_port_r, n_samples);
        out_l = pw_filter_get_dsp_buffer(data->out_port_l, n_samples);
        out_r = pw_filter_get_dsp_buffer(data->out_port_r, n_samples);
        
        if (in_l == NULL || out_l == NULL || in_r == NULL || out_r == NULL){ // Can often return null pointers
                                                                             //
                return;

        }

        memcpy(&l_buffer[ORDER], in_l, n_samples * sizeof(float)); 
        memcpy(&r_buffer[ORDER], in_r, n_samples * sizeof(float)); 


        //need memory buffer for each stage... only two elements each however...
        for(int stage = 0; stage < ORDER/2; stage++){

                for(uint32_t i = ORDER; i < n_samples + ORDER; i++){
                out_l[i-ORDER] = l_buffer[i] * b[stage][0] + l_buffer[i-1] * b[stage][0]1] + l_buffer[i-2] * b[stage][0]2] + l_buffer[i-3] * b[stage][0]3] + l_buffer[i-4] * b[stage][0]4] + l_buffer[i-5] * b[stage][0]5] + l_buffer[i-6] * b[stage][0]6];
                out_r[i-ORDER] = r_buffer[i] * b[stage][0] + r_buffer[i-1] * b[stage][0]1] + r_buffer[i-2] * b[stage][0]2] + r_buffer[i-3] * b[stage][0]3] + r_buffer[i-4] * b[stage][0]4] + r_buffer[i-5] * b[stage][0]5] + r_buffer[i-6] * b[stage][0]6];

                }
        }

        memcpy(&l_buffer[0], &l_buffer[n_samples], ORDER * sizeof(float)); //
        memcpy(&r_buffer[0], &r_buffer[n_samples], ORDER * sizeof(float)); //inefficent

}

/* [on_process] */

static const struct pw_filter_events filter_events = {
        PW_VERSION_FILTER_EVENTS,
        .process = on_process,
};

static void do_quit(void *userdata, int signal_number)
{
        struct data *data = userdata;
        pw_main_loop_quit(data->loop);
}

int main(int argc, char *argv[])
{
        struct data data = { 0, };
        const struct spa_pod *params[1];
        uint32_t n_params = 0;
        uint8_t buffer[1024];
        struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

        pw_init(&argc, &argv);

        /* make a main loop. If you already have another main loop, you can add
         * the fd of this pipewire mainloop to it. */
        data.loop = pw_main_loop_new(NULL);

        pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGINT, do_quit, &data);
        pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGTERM, do_quit, &data);

        /* Create a simple filter, the simple filter manages the core and remote
         * objects for you if you don't need to deal with them.
         *
         * Pass your events and a user_data pointer as the last arguments. This
         * will inform you about the filter state. The most important event
         * you need to listen to is the process event where you need to process
         * the data.
         */
        data.filter = pw_filter_new_simple(
                        pw_main_loop_get_loop(data.loop),
                        "audio-filter",
                        pw_properties_new(
                                PW_KEY_MEDIA_TYPE, "Audio",
                                PW_KEY_MEDIA_CATEGORY, "Filter",
                                PW_KEY_MEDIA_ROLE, "DSP",
                                NULL),
                        &filter_events,
                        &data);

        /* make an audio DSP input port */
        data.in_port_l = pw_filter_add_port(data.filter,
                        PW_DIRECTION_INPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, "input_l",
                                NULL),
                        NULL, 0);

        data.in_port_r = pw_filter_add_port(data.filter,
                        PW_DIRECTION_INPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, "input_r",
                                NULL),
                        NULL, 0);


        /* make an audio DSP output port */
        data.out_port_l = pw_filter_add_port(data.filter,
                        PW_DIRECTION_OUTPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, "output_l",
                                NULL),
                        NULL, 0);

        data.out_port_r = pw_filter_add_port(data.filter,
                        PW_DIRECTION_OUTPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, "output_r",
                                NULL),
                        NULL, 0);


        /* Set processing latency information */
        params[n_params++] = spa_process_latency_build(&b,
                        SPA_PARAM_ProcessLatency,
                        &SPA_PROCESS_LATENCY_INFO_INIT(
                                .ns = 10 * SPA_NSEC_PER_MSEC
                        ));

        /* Now connect this filter. We ask that our process function is
         * called in a realtime thread. */
        if (pw_filter_connect(data.filter,
                                PW_FILTER_FLAG_RT_PROCESS,
                                params, n_params) < 0) {
                fprintf(stderr, "can't connect\n");
                return -1;
        }

        fprintf(stdout, "Pipewire filter setup complete!\n");

        /* and wait while we let things run */
        pw_main_loop_run(data.loop);

        pw_filter_destroy(data.filter);
        pw_main_loop_destroy(data.loop);
        pw_deinit();

        return 0;
}