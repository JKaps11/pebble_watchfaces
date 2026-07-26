#pragma once

#include <pebble.h>

// The Rocket League scene, shared by the Variants in that Session.
//
// Same reasoning as golf_course.h: the arena is a lot of arithmetic that several
// Variants want at different sizes and vantages, and copying it into each one
// would mean six drifting Octanes. Studio-only, like everything beside it.

// The two team colours, plus the surfaces they sit on. A Variant asks for a
// team, not for a colour, so the whole Batch agrees on what blue means.
#define ROCKET_BLUE GColorVividCerulean
#define ROCKET_BLUE_DEEP GColorDukeBlue
#define ROCKET_ORANGE GColorOrange
#define ROCKET_ORANGE_DEEP GColorWindsorTan
// Yellow rather than the chrome yellow the game uses. Emery's display washes the
// warm end out badly — measured off a render, GColorChromeYellow arrives as a
// peach — and yellow is the one warm colour that survives it.
#define ROCKET_BOOST GColorYellow
#define ROCKET_TURF GColorDarkGreen
#define ROCKET_TURF_LIGHT GColorIslamicGreen
#define ROCKET_FLOOR GColorOxfordBlue
#define ROCKET_NET GColorDarkGray

// The Octane in profile, facing right, scaled to fill `frame`. The shape is
// authored on a 100x48 grid and mapped onto the frame, so a Variant picks a size
// and never touches the geometry.
//
// `boosting` adds the rocket flame, which trails *behind* the frame — up to a
// third of the frame's width to the left of `frame.origin.x`. Leave room for it.
void rocket_octane_draw(GContext *ctx, GRect frame, GColor body, bool boosting);

// The Fennec in profile: `length` pixels nose to tail, centred on `centre`,
// rotated `angle` from level. Zero is level and facing right; the scale is
// Pebble's TRIG_MAX_ANGLE and positive turns the nose down.
//
// Two things differ from the Octane above and both are the reason this car
// exists here. It is a box — a flat roof, an upright screen, a vertical hatch —
// which is a shape that survives being 40 pixels long, and the Octane's wedge
// is not. And it takes an angle, because a car that cannot leave the ground
// cannot do the only two things this Session is about.
void rocket_fennec_draw(GContext *ctx, GPoint centre, int length, int32_t angle,
                        GColor body, bool boosting);

// The same car in a muted form, for the stroboscopic copies that make a spin
// read as a spin rather than as a car at a funny angle. It keeps every feature
// the solid one has — a copy reduced to a silhouette stops being a car.
void rocket_fennec_ghost(GContext *ctx, GPoint centre, int length, int32_t angle,
                         GColor colour);

// The Fennec seen from behind, `width` pixels across, banked by `roll`. For the
// vantage that looks down the pitch instead of across it: the car is between the
// viewer and the goal, so what is on screen is its back and its boost.
//
// `pitch` runs 0 to 100 and is what makes this an aerial rather than a car
// driving away. At 0 the viewer is level with it. Above that the nose is coming
// up, so the car foreshortens and its floor comes into view underneath — from
// directly behind, seeing the underside is the whole cue, because a rear view
// pitched up has no other way to say it left the ground.
//
// `plume` colours the tail lights and the boost, and is a parameter rather than
// a constant because a Variant restricted to one hue cannot afford the orange
// the game uses. `outline` draws the car as a keyline rather than a solid.
void rocket_fennec_rear(GContext *ctx, GPoint centre, int width, int32_t roll,
                        int pitch, GColor body, GColor plume, bool boosting,
                        bool outline);

// The same car from above, `length` tall, nosed down or up the display. For the
// kickoff vantage, where a profile car would be the wrong picture.
void rocket_octane_top(GContext *ctx, GPoint centre, int length, GColor body,
                       bool facing_down);

// The ball. Below about seven pixels of radius the panels are dropped, because
// at that size they close up into a grey disc.
void rocket_ball_draw(GContext *ctx, GPoint centre, int radius);

// A boost pad, seen from above: the yellow diamond.
void rocket_boost_pad(GContext *ctx, GPoint centre, int radius);

// The goal seen head-on — net, frame and the team light around the mouth.
void rocket_goal_draw(GContext *ctx, GRect frame, GColor team);

// The pitch seen from above, filling `frame`: lines, both goal mouths and the
// boost pads. Blue defends the top.
void rocket_pitch_draw(GContext *ctx, GRect frame);

// The arena from a car's eye view: black above `horizon`, the lit wall on it,
// and the floor running to `bottom` with its lines converging on the centre.
void rocket_floor_draw(GContext *ctx, int horizon, int bottom);

// The same arena looking up instead of ahead: the lit ceiling ending at `soffit`
// and black below it. For a car driving the roof of the stadium.
void rocket_ceiling_draw(GContext *ctx, int soffit);

// Looking down the pitch at the goal end: the far wall, the side walls falling
// away toward it, and the floor running back to the viewer with its lines
// converging on the middle of the far wall.
//
// The goal itself is not drawn — a Variant sizes and colours that with
// rocket_goal_draw, because where the goal sits on this horizon is most of what
// separates one treatment of the scene from another.
void rocket_downfield_draw(GContext *ctx, int horizon, GColor floor, GColor line,
                           GColor wall);

// Terraces above the far wall, which is the cheapest thing that turns an empty
// arena into an occupied one.
void rocket_stands_draw(GContext *ctx, int horizon);
