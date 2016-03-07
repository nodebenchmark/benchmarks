init = function(args)
   roomID = tonumber( tostring( {} ):sub(8) )
   requestID = 0
end

request = function()
   if requestID == 0 then
      path = "/rooms"
      wrk.body   = string.format("{ \"name\" : %d, \"slug\" : %d, \"description\" : %d }", roomID, roomID, roomID)
      --print(wrk.body)
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 1 then
      path = string.format("/rooms/%s/messages", roomID)
      wrk.body   = string.format("{ \"text\" : \"%s\" }", "Howdy?")
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 2 then
      path = string.format("/rooms/%s/messages", roomID)
      wrk.body   = string.format("{ \"text\" : \"%s\" }", "Okay!")
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 3 then
      path = string.format("/rooms/%s/messages", roomID)
      wrk.body   = string.format("{ \"text\" : \"%s\" }", "Byebye:)")
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 4 then
      path = string.format("/rooms/%s/messages", roomID)
      method = "GET"
      requestID = requestID + 1
   elseif requestID == 5 then
      path = string.format("/rooms/%s", roomID)
      method = "DELETE"
      roomID = tonumber( tostring( {} ):sub(8) )
      requestID = 0
   end

   --print("-----")
   --print(path)
   return wrk.format(method, path)
end
