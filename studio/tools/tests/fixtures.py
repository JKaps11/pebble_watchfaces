"""Building blocks for synthetic renders whose measurements are known by arithmetic.

Fixtures are drawn as solid rectangles at Emery's native size, so every expected
value in a test is a quotient of two numbers written in the test itself rather
than something read off a picture.
"""

import os
import tempfile

from studio import png

# Emery's framebuffer. Fixtures use the real thing so a measurement that depends
# on the display size is exercised at the size it will actually see.
WIDTH = 200
HEIGHT = 228

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
MID_GREY = (119, 119, 119)


class Canvas:
    """A mutable raster that knows how to write itself out as a PNG."""

    def __init__(self, background=WHITE, width=WIDTH, height=HEIGHT):
        self.width = width
        self.height = height
        self.rows = [[background] * width for _ in range(height)]

    def rect(self, x, y, width, height, colour=BLACK):
        """Fill a rectangle. Returns self so fixtures read as one expression."""
        for row in range(y, y + height):
            self.rows[row][x:x + width] = [colour] * width
        return self

    def to_image(self):
        return png.Image(self.width, self.height,
                         tuple(tuple(row) for row in self.rows))

    def write(self, directory, name='fixture.png'):
        path = os.path.join(directory, name)
        png.write(self.to_image(), path)
        return path


class FixtureDirectory:
    """A temporary directory for fixture PNGs, cleaned up with the test."""

    def __init__(self, test_case):
        self._directory = tempfile.TemporaryDirectory()
        test_case.addCleanup(self._directory.cleanup)
        self._count = 0

    def png(self, canvas):
        """Write a canvas out and return its path."""
        self._count += 1
        return canvas.write(self._directory.name, 'fixture{}.png'.format(self._count))
