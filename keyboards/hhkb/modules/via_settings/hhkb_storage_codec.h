/* DMK lossless byte codec. Literal: 0..127 => 1..128 following bytes.
 * Match: 128..255 => 3..130 bytes, followed by distance minus one (1..256).
 * No heap, floating point, or dependencies. Used only when loading/saving. */
#ifndef HHKB_STORAGE_CODEC_H
#define HHKB_STORAGE_CODEC_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t (*hhkb_byte_reader)(size_t, void *);
typedef void (*hhkb_byte_writer)(size_t, uint8_t, void *);
static inline size_t hhkb_pack_stream(hhkb_byte_reader read, void *ctx, size_t size, uint8_t *out, size_t capacity) {
    size_t i = 0, o = 0, literal = 0, start = 0;
    while (i < size) {
        size_t best = 0, distance = 0;
        for (size_t d = 1; d <= i && d <= 256; ++d) {
            size_t n = 0;
            while (n < 130 && i + n < size && read(i + n, ctx) == read(i + n - d, ctx)) ++n;
            if (n > best) { best = n; distance = d; if (best == 130) break; }
        }
        if (best >= 3) {
            if (literal) { out[start] = (uint8_t)(literal - 1); literal = 0; }
            if (o + 2 > capacity) return 0;
            out[o++] = (uint8_t)(0x80 | (best - 3)); out[o++] = (uint8_t)(distance - 1);
            i += best;
        } else {
            if (!literal) { if (o == capacity) return 0; start = o++; }
            if (o == capacity) return 0;
            out[o++] = read(i++, ctx);
            if (++literal == 128) { out[start] = 127; literal = 0; }
        }
    }
    if (literal) out[start] = (uint8_t)(literal - 1);
    return o;
}
static inline bool hhkb_unpack_stream(const uint8_t *in, size_t size, size_t expected,
                                      hhkb_byte_writer write, void *ctx) {
    uint8_t history[256];
    size_t i = 0, o = 0;
    while (i < size) {
        uint8_t token = in[i++];
        size_t n = (token & 0x80) ? (token & 127) + 3U : token + 1U;
        if (n > expected - o) return false;
        size_t distance = 0;
        if (token & 0x80) {
            if (i == size) return false;
            distance = in[i++] + 1U;
            if (distance > o) return false;
        } else if (n > size - i) return false;
        while (n--) {
            uint8_t value = distance ? history[(o-distance)&255] : in[i++];
            history[o&255] = value;
            if (write) write(o, value, ctx);
            ++o;
        }
    }
    return o == expected;
}
static inline uint8_t hhkb_array_read(size_t i, void *p) { return ((const uint8_t *)p)[i]; }
static inline void hhkb_array_write(size_t i, uint8_t v, void *p) { ((uint8_t *)p)[i]=v; }
static inline size_t hhkb_pack(const uint8_t *in, size_t size, uint8_t *out, size_t capacity) {
    return hhkb_pack_stream(hhkb_array_read, (void *)in, size, out, capacity);
}
static inline bool hhkb_unpack(const uint8_t *in, size_t size, uint8_t *out, size_t expected) {
    return hhkb_unpack_stream(in,size,expected,hhkb_array_write,out);
}
#endif
