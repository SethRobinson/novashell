
/*
Object: Joystick
By using <InputManager::GetJoystick> you can access extra information about a specific gamepad or joystick.

Group: Member Functions

func: GetName
(code)
string GetName()
(end)

Returns:

The device name of the joystick.

func: GetButtonCount
(code)
number GetButtonCount()
(end)

Returns:

How many buttons this joystick/gamepad has.

func: GetID
(code)
number GetID()
(end)

Returns:

The ID of this joystick. Starts a 0.

func: GetLeftStickPosition
(code)
Vector2 GetLeftStickPosition()
(end)

Although you can use <InputManager:AddBinding> to map to easily map to a joysticks directions, it's useless if you want true 360 degree analog data.

This lets you poll a sticks position at any time yourself.  

Returns:

A <Vector2> object containing this sticks' current position as a unit vector. (Ie, -1, 0 would mean moving left 100%, 0,0 would mean not pushing in any direction)

func: GetRightStickPosition
(code)
Vector2 GetRightStickPosition()
(end)

Although you can use <InputManager:AddBinding> to map to easily map to a joysticks directions, it's useless if you want true 360 degree analog data.

This lets you poll a sticks position at any time yourself.  

Note:

Not all game controllers have a right stick, and even if they do, they tend to use different axis for them.  See <SetRightStickAxis>.

Returns:

A <Vector2> object containing this sticks' current position as a unit vector. (Ie, -1, 0 would mean moving left 100%, 0,0 would mean not pushing in any direction)


func: SetRightStickAxis
(code)
nil SetRightStickAxis(number axisX, number axisY)
(end)

This lets you setup which axis are used for a specific stick on this controller.  We don't have a SetLeftStickAxis because that is always 0 (x) and 1 (y).

The engine will attempt to use detect the controller by name and setup this itself, check base/script/dialog.gui to see where I detect XBOX controllers and setup for them.

If you have an additional setting I should add, please email it to me. (Seth)

I plan on adding a nice Joystick setup utility in a base script for controllers that need to be remapped that any app can use/customize.

Note:

Not all game controllers have a right stick, and even if they do, they tend to use different axis for them.  See <SetRightStickAxis>.

Returns:

A <Vector2> object containing this sticks' current position as a unit vector. (Ie, -1, 0 would mean moving left 100%, 0,0 would mean not pushing in any direction)



*/