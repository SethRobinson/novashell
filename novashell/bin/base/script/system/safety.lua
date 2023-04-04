-- Forces strict variable checking.  Most of this is written by Ian (note to self, get his full name sometime!)
-- Requires that all global variables be assigned a value (even if nil) in the
-- global scope ("main chunk") before it may be read.
-- The warnings/errors can be temporarily disabled by setting g_allowStrict to 0, then 1 to enable.  Don't change
-- this var name because the engine sets it directly when it needs to.'


IGNORE = 1
WARNING = 2
ERROR = 3

safety = {internalException = IGNORE,
          readUndefinedGlobal = ERROR,
          writeUndefinedGlobal = ERROR}

do
    local emptyTraceback = "\nstack traceback:"
    local stackLevel = 3

    function Raise(level, err, ...)
        if level == IGNORE then
            return
        end
        local traceback = debug.traceback("", stackLevel)
        if traceback == emptyTraceback then
            Raise(safety.internalException, err, ...)
        elseif level == WARNING then
            print(string.format(err, ...), traceback, stackLevel)
        elseif level == ERROR then
            error(string.format(err, ...), stackLevel)
        else
            Raise(ERROR, "Invalid error level.")
        end
    end
end

do
    -- Get the metatable for the global namespace.
    local globalMetatable = getmetatable(_G)

    -- Augment the metatable if it already exists; otherwise make a new one.
    if globalMetatable == nil then
        globalMetatable = {}
        setmetatable(_G, globalMetatable)
    end

    globalMetatable.__declared = {}

    -- Deletes a global variable.
    function Delete(variable)
        rawset(_G, variable, nil)
        globalMetatable.__declared[variable] = nil
    end

    function metaExists(variable)
        return globalMetatable.__declared[variable]
    end

    -- Invoked whenever a global variable is read.
    function globalMetatable:__index(variable)
         if g_allowStrict == true and globalMetatable.__declared[variable] == nil
           -- C functions should be allowed to do weird stuff, but for some
           -- reason accessing this stack frame causes a crash. Uncomment the
           -- following line if this has changed.
           and debug.getinfo(2, "S").what ~= "C" then
            Raise(safety.readUndefinedGlobal,
                  "Variable '%s' is not defined.", variable)
        end
        return rawget(self, variable)
    end

    -- Invoked whenever a global variable is created.
    function globalMetatable:__newindex(variable, value)

        if not metaExists(variable) then
// Was unable to  get this part working right.. -Seth
//            local what = debug.getinfo(2, "S").what
//            -- Assignment to undefined variables is allowed in the global
//            -- namespace ("main chunk") and C functions.
//            if what ~= "main" and what ~= "C" then
//                if type(value) ~= "function" then
//                    Raise(safety.writeUndefinedGlobal,
//                          "Assignment to undefined variable '%s'.", variable)
//                else
//                    Raise(safety.writeUndefinedGlobal,
//                          "Inner function '%s' should be made local.",
//                          variable)
//                end
//            end
            globalMetatable.__declared[variable] = true
        end
        rawset(self, variable, value)
    end
end

//global that can handle testing for a function or variable without showing an error.  Note that this function
//automatically checks the entity's namespace first if it is available.

  