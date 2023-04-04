
/*
Object: EffectBase
All effect emitters are based on this type and can use its functions.

Group: Member Functions

func: AddParticle
(code)
nil AddParticle(Particle particle, number probability)
(end)
Adds a new particle to be initted and sets the odds of choosing it versus other particles that have been added.

Parameters:
particle - A valid <Particle> object.
probability - The odds of this particle spawning compared with other particles that were added.  0.1 for 10 percent.  Use <C_PROBABILITY_DEFAULT> to let the emitter decide.

func: SetOffset
(code)
nil SetOffset(Vector2 vOffset)
(end)
Sets the effect position in relation to the entity it's attached to.

Parameters:

vOffset - A <Vector2> object containing the offset coordinates.

func: SetAdditionalVector
(code)
nil SetAdditionalVector(Vector2 v)
(end)
This vector is added to the velocity of each particle once as it's created.

Parameters:

v - A <Vector2> object containing the vector to add.  Vector2(0,-1) would mean shoot up, for instance.

Section: Related Constants

Group: C_PROBABILITY_CONSTANTS
Used with <EffectBase::AddParticle>.

constant: C_PROBABILITY_DEFAULT
Used to say "equal chance of creating all types particles we add".
*/
