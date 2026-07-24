"""Measure a Studio render.

A pure function from a framebuffer PNG to numbers. The eye is bad at contrast
ratios and worse at noticing a date that clips one pixel off the edge, so these
measurements exist to be believed when they contradict what the picture seems to
show.
"""

from collections import Counter
from dataclasses import dataclass, field

from studio import png


# Anything drawn closer than this to the display edge reads as crowding it.
SAFE_MARGIN_PX = 8

# Below this ratio of tallest element to next tallest, the eye has to choose
# where to land; below NO_CLEAR_PRIMARY there is nothing to choose between.
GLANCE_HIERARCHY_MIN = 1.5
NO_CLEAR_PRIMARY = 1.2

# Marks closer than this on a shared line are glyphs in one run, not separate
# elements. Wide enough to span a word space at watchface sizes, narrow enough
# to keep a time and a neighbouring complication apart.
GLYPH_GAP_PX = 8

# WCAG floors are 4.5:1 for normal text and 3:1 for large. The time is held
# higher than either because a reflective display is read in direct sunlight,
# where effective contrast is lower than measured contrast.
PRIMARY_CONTRAST_MIN = 7.0
COMPLICATION_CONTRAST_MIN = 4.5

# How far out from an element to look for the ground it sits on.
LOCAL_GROUND_PX = 4

# Above this share of inked pixels a design reads as dense. Informational: a
# dense design may be exactly what was asked for, so this never fails.
INK_DENSE_ABOVE = 0.40

FAILURE = 'failure'
FLAG = 'flag'

# Which render each measurement is taken from. Clipping and contrast are
# measured at the Stress state because that is where they fail; the other three
# describe the design being compared, so they come from the Canonical render.
STRESS_MEASUREMENTS = ('contrast', 'clipping')
CANONICAL_MEASUREMENTS = ('glance hierarchy', 'ink coverage', 'safe margin')


@dataclass(frozen=True)
class Finding:
    """Something worth saying about a render. Passing measurements produce none."""

    measurement: str
    severity: str
    detail: str
    element: 'Element | None' = None

    @property
    def is_failure(self):
        return self.severity == FAILURE


@dataclass(frozen=True)
class Element:
    """One run of marks the eye reads as a single thing — a time, a date, a label."""

    x: int
    y: int
    width: int
    height: int
    ink: tuple
    ground: tuple
    contrast: float
    # The tallest element is taken to be the time, and is held to the higher
    # floor. Everything else is a complication.
    primary: bool

    @property
    def right(self):
        return self.x + self.width - 1

    @property
    def bottom(self):
        return self.y + self.height - 1

    @property
    def contrast_floor(self):
        return PRIMARY_CONTRAST_MIN if self.primary else COMPLICATION_CONTRAST_MIN

    @property
    def contrast_ok(self):
        return self.contrast >= self.contrast_floor


@dataclass(frozen=True)
class Measurements:
    elements: tuple  # of Element, tallest first
    ink_coverage: float
    clipping: bool
    safe_margin: int | None  # None when nothing is drawn

    @property
    def safe_margin_violated(self):
        return self.safe_margin is not None and self.safe_margin < SAFE_MARGIN_PX

    @property
    def glance_hierarchy(self):
        """Tallest element's height over the next tallest. None if there is no pair."""
        if len(self.elements) < 2:
            return None
        return self.elements[0].height / self.elements[1].height

    @property
    def glance_hierarchy_weak(self):
        ratio = self.glance_hierarchy
        return ratio is not None and ratio < GLANCE_HIERARCHY_MIN

    @property
    def no_clear_primary(self):
        ratio = self.glance_hierarchy
        return ratio is not None and ratio < NO_CLEAR_PRIMARY

    @property
    def contrast_failures(self):
        return tuple(e for e in self.elements if not e.contrast_ok)

    @property
    def ink_dense(self):
        return self.ink_coverage > INK_DENSE_ABOVE

    @property
    def findings(self):
        """Everything wrong or worth noting, and nothing that is fine.

        Six Variants' worth of passing numbers is a wall nobody reads, and a
        report that gets skipped is worse than no report.
        """
        found = []

        for element in self.contrast_failures:
            role = 'time' if element.primary else 'complication'
            found.append(Finding(
                'contrast', FAILURE,
                '{} element at {:.1f}:1 against its ground, below the {:.1f}:1 '
                'floor'.format(role, element.contrast, element.contrast_floor),
                element))

        if self.clipping:
            found.append(Finding(
                'clipping', FAILURE,
                'something is drawn on the outermost row or column, so it is '
                'cut off'))

        if self.no_clear_primary:
            found.append(Finding(
                'glance hierarchy', FAILURE,
                'tallest element is only {:.2f}x the next, so there is no '
                'clear primary'.format(self.glance_hierarchy)))
        elif self.glance_hierarchy_weak:
            found.append(Finding(
                'glance hierarchy', FLAG,
                'tallest element is {:.2f}x the next, below {:.1f}x'.format(
                    self.glance_hierarchy, GLANCE_HIERARCHY_MIN)))

        if self.ink_dense:
            found.append(Finding(
                'ink coverage', FLAG,
                '{:.0%} of pixels are inked, a dense design'.format(
                    self.ink_coverage)))

        if self.safe_margin_violated:
            found.append(Finding(
                'safe margin', FLAG,
                'nearest element is {} px from the display edge, below '
                '{} px'.format(self.safe_margin, SAFE_MARGIN_PX)))

        return tuple(found)


def _background_colour(image):
    """The most common colour. On a watchface that is always the ground."""
    return Counter(colour for _, _, colour in image.pixels()).most_common(1)[0][0]


def _ink_pixels(image, background):
    """Every pixel that is not the ground, as {(x, y): colour}."""
    return {(x, y): colour
            for x, y, colour in image.pixels() if colour != background}


def _clips(image, ink):
    """Whether anything is drawn on the outermost row or column."""
    last_x, last_y = image.width - 1, image.height - 1
    return any(x in (0, last_x) or y in (0, last_y) for x, y in ink)


def _safe_margin(image, ink):
    """How close the nearest drawn pixel comes to any display edge."""
    if not ink:
        return None
    last_x, last_y = image.width - 1, image.height - 1
    return min(min(x, y, last_x - x, last_y - y) for x, y in ink)


def _connected_runs(ink):
    """Group ink into 8-connected blobs. One glyph is usually one blob."""
    remaining = set(ink)
    while remaining:
        seed = remaining.pop()
        blob = {seed}
        frontier = [seed]
        while frontier:
            x, y = frontier.pop()
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    neighbour = (x + dx, y + dy)
                    if neighbour in remaining:
                        remaining.discard(neighbour)
                        blob.add(neighbour)
                        frontier.append(neighbour)
        yield blob


def _bounds(pixels):
    xs = [x for x, _ in pixels]
    ys = [y for _, y in pixels]
    return min(xs), min(ys), max(xs) - min(xs) + 1, max(ys) - min(ys) + 1


def _shares_a_line(first, second):
    """Whether two boxes sit on one line close enough to read as one run."""
    (ax, ay, aw, ah), (bx, by, bw, bh) = first, second
    vertically_overlapping = max(ay, by) <= min(ay + ah - 1, by + bh - 1)
    gap = max(ax, bx) - min(ax + aw - 1, bx + bw - 1) - 1
    return vertically_overlapping and gap <= GLYPH_GAP_PX


def _elements(ink):
    """Merge blobs that read as one run, tallest element first.

    Glyphs arrive as separate blobs, so a time like "10:09" would otherwise
    count as five elements and flatten the hierarchy ratio to 1.0.
    """
    groups = [(_bounds(blob), set(blob)) for blob in _connected_runs(ink)]

    merged = True
    while merged:
        merged = False
        for i in range(len(groups)):
            for j in range(i + 1, len(groups)):
                if _shares_a_line(groups[i][0], groups[j][0]):
                    pixels = groups[i][1] | groups[j][1]
                    groups[i] = (_bounds(pixels), pixels)
                    del groups[j]
                    merged = True
                    break
            if merged:
                break

    groups.sort(key=lambda group: group[0][3], reverse=True)  # by height
    return groups


def _relative_luminance(colour):
    """WCAG 2.x relative luminance of an sRGB colour."""
    channels = []
    for value in colour:
        srgb = value / 255
        channels.append(srgb / 12.92 if srgb <= 0.03928
                        else ((srgb + 0.055) / 1.055) ** 2.4)
    red, green, blue = channels
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue


def contrast_ratio(foreground, background):
    """WCAG 2.x contrast ratio, from 1:1 (identical) to 21:1 (black on white)."""
    lighter, darker = sorted(
        (_relative_luminance(foreground), _relative_luminance(background)),
        reverse=True)
    return (lighter + 0.05) / (darker + 0.05)


def _local_ground(image, bounds, pixels, ink, fallback):
    """The dominant ground colour in a band just outside the element.

    Contrast is a local property: a design that inverts polarity halfway down
    the display would be measured wrongly against a single global ground.
    """
    x, y, width, height = bounds
    band = Counter()
    for row in range(max(0, y - LOCAL_GROUND_PX),
                     min(image.height, y + height + LOCAL_GROUND_PX)):
        for column in range(max(0, x - LOCAL_GROUND_PX),
                            min(image.width, x + width + LOCAL_GROUND_PX)):
            if (column, row) not in ink:
                band[image.pixel(column, row)] += 1
    return band.most_common(1)[0][0] if band else fallback


def _dominant(pixels, image):
    return Counter(image.pixel(x, y) for x, y in pixels).most_common(1)[0][0]


def measure(path):
    """Measure the render at `path`."""
    image = png.read(path)
    background = _background_colour(image)
    ink = _ink_pixels(image, background)

    elements = []
    for index, (bounds, pixels) in enumerate(_elements(ink)):
        mark = _dominant(pixels, image)
        ground = _local_ground(image, bounds, pixels, ink, background)
        elements.append(Element(
            *bounds,
            ink=mark,
            ground=ground,
            contrast=contrast_ratio(mark, ground),
            primary=(index == 0),
        ))

    return Measurements(
        elements=tuple(elements),
        ink_coverage=len(ink) / (image.width * image.height),
        clipping=_clips(image, ink),
        safe_margin=_safe_margin(image, ink),
    )
