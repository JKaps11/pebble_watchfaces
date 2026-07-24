"""Read and write the slice of PNG the Studio deals in, using only the stdlib.

Two callers need this: the metrics module reads framebuffer PNGs the emulator
writes, and its tests write synthetic ones. Pillow would do both, but it would
be the Studio's only third-party dependency, and the subset of PNG involved
here is small — 8-bit channels, non-interlaced, RGB or RGBA — so carrying it
costs less than requiring an install for a sandbox nobody ships.
"""

import struct
import zlib

PNG_SIGNATURE = b'\x89PNG\r\n\x1a\n'

# Bytes per pixel for the colour types read here: truecolour, truecolour with
# alpha, and palette. Palette is included because a Contact sheet of 64-colour
# framebuffers comes out of ImageMagick indexed, and it is committed — leaving it
# indexed keeps the kept record cheap, which is the point of keeping it.
_CHANNELS = {2: 3, 6: 4, 3: 1}
_PALETTE = 3


class UnsupportedPNG(Exception):
    """The file is a PNG, but not one this module reads."""


class Image:
    """An RGB raster. Alpha is dropped on read — a framebuffer is opaque."""

    __slots__ = ('width', 'height', 'rows')

    def __init__(self, width, height, rows):
        self.width = width
        self.height = height
        self.rows = rows  # tuple of `height` tuples of `width` (r, g, b)

    def pixel(self, x, y):
        return self.rows[y][x]

    def pixels(self):
        """Every pixel as (x, y, colour), in no order callers may rely on."""
        for y, row in enumerate(self.rows):
            for x, colour in enumerate(row):
                yield x, y, colour


def _chunks(data):
    if data[:8] != PNG_SIGNATURE:
        raise UnsupportedPNG('not a PNG file')
    offset = 8
    while offset < len(data):
        (length,) = struct.unpack('>I', data[offset:offset + 4])
        kind = data[offset + 4:offset + 8]
        body = data[offset + 8:offset + 8 + length]
        yield kind, body
        offset += 12 + length


def _unfilter(raw, width, height, channels):
    """Reverse the per-scanline filters PNG applies before compression."""
    stride = width * channels
    out = []
    previous = bytearray(stride)
    offset = 0
    for _ in range(height):
        filter_type = raw[offset]
        line = bytearray(raw[offset + 1:offset + 1 + stride])
        offset += 1 + stride
        if filter_type == 1:      # Sub
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif filter_type == 2:    # Up
            for i in range(stride):
                line[i] = (line[i] + previous[i]) & 0xFF
        elif filter_type == 3:    # Average
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((left + previous[i]) >> 1)) & 0xFF
        elif filter_type == 4:    # Paeth
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                up = previous[i]
                up_left = previous[i - channels] if i >= channels else 0
                estimate = left + up - up_left
                da, db, dc = (abs(estimate - left), abs(estimate - up),
                              abs(estimate - up_left))
                if da <= db and da <= dc:
                    predictor = left
                elif db <= dc:
                    predictor = up
                else:
                    predictor = up_left
                line[i] = (line[i] + predictor) & 0xFF
        elif filter_type != 0:    # 0 is None
            raise UnsupportedPNG('unknown scanline filter {}'.format(filter_type))
        out.append(line)
        previous = line
    return out


def read(path):
    """Load a PNG file as an Image."""
    with open(path, 'rb') as handle:
        data = handle.read()

    header = None
    palette = None
    compressed = bytearray()
    for kind, body in _chunks(data):
        if kind == b'IHDR':
            header = struct.unpack('>IIBBBBB', body[:13])
        elif kind == b'PLTE':
            palette = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
        elif kind == b'IDAT':
            compressed += body

    if header is None:
        raise UnsupportedPNG('no IHDR chunk')
    width, height, depth, colour_type, _, _, interlace = header
    if depth != 8:
        raise UnsupportedPNG('only 8-bit channels are supported, got {}'.format(depth))
    if interlace:
        raise UnsupportedPNG('interlaced PNGs are not supported')
    if colour_type not in _CHANNELS:
        raise UnsupportedPNG('unsupported colour type {}'.format(colour_type))

    channels = _CHANNELS[colour_type]
    lines = _unfilter(zlib.decompress(bytes(compressed)), width, height, channels)

    if colour_type == _PALETTE:
        if palette is None:
            raise UnsupportedPNG('palette image with no PLTE chunk')
        rows = tuple(tuple(palette[index] for index in line[:width])
                     for line in lines)
    else:
        rows = tuple(
            tuple((line[i], line[i + 1], line[i + 2])
                  for i in range(0, width * channels, channels))
            for line in lines
        )
    return Image(width, height, rows)


def _chunk(kind, body):
    return (struct.pack('>I', len(body)) + kind + body
            + struct.pack('>I', zlib.crc32(kind + body) & 0xFFFFFFFF))


def write(image, path):
    """Save an Image as an 8-bit RGBA PNG, matching what the emulator writes."""
    raw = bytearray()
    for row in image.rows:
        raw.append(0)  # filter type None: these images compress well regardless
        for r, g, b in row:
            raw += bytes((r, g, b, 255))

    header = struct.pack('>IIBBBBB', image.width, image.height, 8, 6, 0, 0, 0)
    with open(path, 'wb') as handle:
        handle.write(PNG_SIGNATURE)
        handle.write(_chunk(b'IHDR', header))
        handle.write(_chunk(b'IDAT', zlib.compress(bytes(raw), 9)))
        handle.write(_chunk(b'IEND', b''))
