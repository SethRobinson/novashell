
/*
Object: Particle
Information about a certain particle type that is shared between effects.

Group: Member Functions

func: SetColor
(code)
nil SetColor(Color color)
(end)
Sets the initial color of the particle.

Usage:
(code)
particle:SetColor(Color(255,110,60,255));

(end)


func: SetColoring1
(code)
nil SetColoring1(Color color1, number startTimePercent)
(end)
Particle will fade to this color.

Parameters:

color1 - A <Color> object holding the color we want to change to interpolate to.
startTimePercent - At what point the change should start to happen, 0 to 1.


func: SetColoring2
(code)
nil SetColoring2(Color color1, Color color2, number startTimePercent)
(end)
Like <Particle::SetColoring1> except it allows you to fade to three colors total.

Parameters:

color1 - A <Color> object holding the color we want to change to interpolate to.
color2 - The second <Color> object holding the color we want to change to interpolate to.
startTimePercent - At what point the change should start to happen, 0 to 1.

func: GetMotionController
(code)
MotionController GetMotionController()
(end)

Returns:

The <MotionController> associated with this particle template.  (created if one doesn't exist)

func: SetBlendMode
(code)
nil SetBlendMode(number blendConstant)
(end)
Only normal and additive modes are currently supported.

Parameters:

blendConstant - Must be one of the <C_BLEND_MODE_CONSTANTS>.
*/
