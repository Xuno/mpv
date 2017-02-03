#ifndef MPV_xuno_H
#define MPV_xuno_H

#include <stdbool.h>

#include <libavcodec/avcodec.h>

#include "demux/stheader.h"
#include "video/mp_image.h"
#include "video/mp_image_pool.h"
#include "video/hwdec.h"

#define hwdec_xuno_DELAY_QUEUE_COUNT 2

// Maximum number of surfaces the player wants to buffer.
// This number might require adjustment depending on whatever the player does;
// for example, if vo_opengl increases the number of reference surfaces for
// interpolation, this value has to be increased too.
// This value does not yet include hwdec_xuno_DELAY_QUEUE_COUNT.
#define hwdec_xuno_EXTRA_SURFACES 4

typedef struct xuno_ctx {
    struct mp_log *log;
    struct MPOpts *opts;
    AVCodecContext *avctx;
    AVFrame *pic;
    struct vd_xuno_hwdec *hwdec;
    AVRational codec_timebase;
    enum AVPixelFormat pix_fmt;
    enum AVDiscard skip_frame;
    bool flushing;
    const char *decoder;
    bool hwdec_xuno_failed;
    bool hwdec_xuno_notified;

    int framedrop_flags;

    // For HDR side-data caching
    double cached_hdr_peak;

    bool hw_probing;
    struct demux_packet **sent_packets;
    int num_sent_packets;

    struct demux_packet **requeue_packets;
    int num_requeue_packets;

    struct mp_image **delay_queue;
    int num_delay_queue;
    int max_delay_queue;

    // From VO
    struct mp_hwdec_xuno_devices *hwdec_xuno_devs;

    // For free use by hwdec implementation
    void *hwdec_xuno_priv;

    int hwdec_xuno_fmt;
    int hwdec_xuno_w;
    int hwdec_xuno_h;
    int hwdec_xuno_profile;

    bool hwdec_xuno_request_reinit;
    int hwdec_xuno_fail_count;

    struct mp_image_pool *hwdec_xuno_swpool;

    AVBufferRef *cached_hw_frames_ctx;
} vd_ffmpeg_ctx;

struct vd_xuno_hwdec {
    enum hwdec_type type;
    // If not-0: the IMGFMT_ format that should be accepted in the libavcodec
    // get_format callback.
    int image_format;
    // Always returns a non-hwaccel image format.
    bool copying;
    // Setting this will queue the given number of frames before calling
    // process_image() or returning them to the renderer. This can increase
    // efficiency by not blocking on the hardware pipeline by reading back
    // immediately after decoding.
    int delay_queue;
    // If true, AVCodecContext will destroy the underlying decoder.
    bool volatile_context;
    int (*probe)(struct xuno_ctx *ctx, struct vd_xuno_hwdec *hwdec,
                 const char *codec);
    int (*init)(struct xuno_ctx *ctx);
    int (*init_decoder)(struct xuno_ctx *ctx, int w, int h);
    void (*uninit)(struct xuno_ctx *ctx);
    // Note: if init_decoder is set, this will always use the values from the
    //       last successful init_decoder call. Otherwise, it's up to you.
    struct mp_image *(*allocate_image)(struct xuno_ctx *ctx, int w, int h);
    // Process the image returned by the libavcodec decoder.
    struct mp_image *(*process_image)(struct xuno_ctx *ctx, struct mp_image *img);
    // For horrible Intel shit-drivers only
    void (*lock)(struct xuno_ctx *ctx);
    void (*unlock)(struct xuno_ctx *ctx);
    // Optional; if a special hardware decoder is needed (instead of "hwaccel").
    const char *(*get_codec)(struct xuno_ctx *ctx, const char *codec);
    // Suffix for libavcodec decoder. If non-NULL, get_codec() is overridden
    // with hwdec_xuno_find_decoder.
    // Intuitively, this will force the corresponding wrapper decoder.
    const char *xuno_suffix;
};

enum {
    hwdec_xuno_ERR_NO_CTX = -2,
    hwdec_xuno_ERR_NO_CODEC = -3,
    hwdec_xuno_ERR_EMULATED = -4,    // probing successful, but emulated API detected
};

struct hwdec_xuno_profile_entry {
    enum AVCodecID av_codec;
    int ff_profile;
    uint64_t hw_profile;
};

const struct hwdec_xuno_profile_entry *hwdec_xuno_find_profile(
    struct xuno_ctx *ctx, const struct hwdec_xuno_profile_entry *table);
bool hwdec_xuno_check_codec_support(const char *codec,
                               const struct hwdec_xuno_profile_entry *table);
int hwdec_xuno_get_max_refs(struct xuno_ctx *ctx);
int hwdec_xuno_setup_hw_frames_ctx(struct xuno_ctx *ctx, AVBufferRef *device_ctx,
                              int av_sw_format, int initial_pool_size);

const char *hwdec_xuno_find_decoder(const char *codec, const char *suffix);

#endif
