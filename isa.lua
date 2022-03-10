--[[
    Project ISA
    Dissector implementation
    Author: Kozhevnikov Dmitrii
    Login: xkozhe00
]]

--protocol announcement
local isaProto = Proto("isaProto", "isaProto")


-- protokol headers
local status = ProtoField.string("isaProto.status", "Status", base.ASCII)
local loginToken = ProtoField.string("isaProto.loginToken", "Login-token", base.ASCII)
local login = ProtoField.string("isaProto.login", "Login", base.ASCII)
local theme = ProtoField.string("isaProto.theme", "Theme", base.ASCII)
local message = ProtoField.string("isaProto.message", "Message", base.ASCII)
local number = ProtoField.string("isaProto.number", "List number", base.ASCII)
local from = ProtoField.string("isaProto.from", "From", base.ASCII)
local subject = ProtoField.string("isaProto.subject", "Subject", base.ASCII)

-- function for obtaining tokens for verification
function takeString(buffer, counter, len)

    for i = counter, len - 1, 1 do
        if (buffer(i, 1):string() == "\"") then
            return i - counter
        end
    end
    
end

-- Registration of protocol fields
isaProto.fields = {status, loginToken , login, theme, message, number, from, subject}

-- dissector:
function isaProto.dissector(buffer, pinfo, tree)
    local bufferLength = buffer:len()

    -- buffer length control
    if bufferLength == 0 then 
        return 
    end
    
    pinfo.cols.protocol = isaProto.name                                       -- protokol name 
    local subtree = tree:add(isaProto, buffer(0))                             -- subtrea creating
    
    -- PARSE PACKETS -- 

    -- send 
    if buffer(1, 4):string() == 'send' then
        local tokenStart = 7
        subtree:add(status, buffer(1,4))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))

        tokenStart = tokenStart + len + 3
        len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(login, buffer(tokenStart, len))

        tokenStart = tokenStart + len + 3
        len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(theme,  buffer(tokenStart, len))

        tokenStart = tokenStart + len + 3
        len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(message,  buffer(tokenStart, len))
    
    -- ok
    elseif buffer(1, 2):string() == 'ok' then
        local tokenStart = 5
        subtree:add(status, buffer(1,2))
        local len = takeString(buffer, tokenStart, bufferLength)

        local bol = true

        -- empty list
        if buffer(tokenStart, len):string() == '))' then
            bol = false
        
        -- non empty list
        elseif buffer(tokenStart, 1):string() == '(' then

            while bol do
                tokenStart = tokenStart + 1
                len = takeString(buffer, tokenStart, bufferLength)
                subtree:add(number, buffer(tokenStart, len))

                while buffer(tokenStart, 1):string() ~= '\"' do
                    tokenStart = tokenStart + 1
                end

                tokenStart = tokenStart + 1
                len = takeString(buffer, tokenStart, bufferLength)
                subtree:add(from, buffer(tokenStart, len))

                tokenStart = tokenStart + len + 3
                len = takeString(buffer, tokenStart, bufferLength)
                subtree:add(subject, buffer(tokenStart, len))
            
                if buffer(tokenStart + len + 3, 1):string() == ')' then
                    break
                end

                tokenStart = tokenStart + len + 3
            end
        
        -- ok fetch
        elseif buffer(tokenStart, 1):string() == '\"' then
            len = takeString(buffer, tokenStart + 1, bufferLength)
            subtree:add(from, buffer(tokenStart + 1, len))

            tokenStart = tokenStart + len + 4
            len = takeString(buffer, tokenStart, bufferLength)
            subtree:add(subject, buffer(tokenStart, len))

            tokenStart = tokenStart + len + 3
            len = takeString(buffer, tokenStart, bufferLength)
            subtree:add(message, buffer(tokenStart, len))
        
        -- ok logout
        elseif buffer(tokenStart, 10):string() == 'logged out' then
            subtree:add(theme,  buffer(tokenStart, len))

        -- ok registered user
        elseif buffer(tokenStart, 10):string() == 'registered' then
            subtree:add(theme,  buffer(tokenStart, len))

        -- ok message sent
        elseif buffer(tokenStart,12):string() == 'message sent' then
            subtree:add(theme,  buffer(tokenStart, len))

        -- ok user logged in
        elseif buffer(tokenStart, len):string() == 'user logged in' then
            subtree:add(theme,  buffer(tokenStart, len))

            tokenStart = tokenStart + len + 3
            len = takeString(buffer, tokenStart, bufferLength)
            subtree:add(loginToken,  buffer(tokenStart, len))  
        end
    
    -- list command
    elseif buffer(1, 4):string() == 'list' then
        local tokenStart = 7
        subtree:add(status, buffer(1,4))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))
    
    -- errors from server
    elseif buffer(1, 3):string() == 'err' then
        local tokenStart = 6
        subtree:add(status, buffer(1,3))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(theme,  buffer(tokenStart, len))

    -- login
    elseif buffer(1, 5):string() == 'login' then
        local tokenStart = 8
        subtree:add(status, buffer(1,5))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(login,  buffer(tokenStart, len))

        tokenStart = tokenStart + len + 3
        len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))

    -- fetch
    elseif buffer(1, 5):string() == 'fetch' then
        local tokenStart = 8
        subtree:add(status, buffer(1, 5))

        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))
 
        tokenStart = tokenStart + len + 2
        local count = 0

        while true do
            if buffer(tokenStart, 1):string() == ')' then
                break
            end
            count = count + 1
            tokenStart = tokenStart + 1
        end
        
        subtree:add(number,  buffer(tokenStart - count, count))

    -- logout
    elseif buffer(1, 6):string() == 'logout' then  
        local tokenStart = 9
        subtree:add(status, buffer(1, 6))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))

    -- register
    elseif buffer(1, 8):string() == 'register' then
        local tokenStart = 11
        subtree:add(status, buffer(1,8))
        local len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(login,  buffer(tokenStart, len))

        tokenStart = tokenStart + len + 3
        len = takeString(buffer, tokenStart, bufferLength)
        subtree:add(loginToken,  buffer(tokenStart, len))

    -- some mistakes
    else
        subtree:append_text("ALARM! SOMETHING WRONG!")  
    end

end

-- Register the dissector
--local tcp_dissector_table = DissectorTable.get("tcp.port")
--tcp_dissector_table:add(60000, isaProto)
isaProto:register_heuristic("tcp", isaProto.dissector)