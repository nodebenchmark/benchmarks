api_key = "&apikey=3a3dfd09ad938e39087d8d761ac0deca75257891df0fc1fd2da963543f6f8bd7"

init = function(args)
   padID = "pad" .. tonumber( tostring( {} ):sub(8) )
   requestID = 0
end

request = function()
   local base = "/api/1"

   if requestID == 0 then
      path = base .. "/createPad?padID=" .. padID
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 1 then
      path = base .. "/getText?padID=" .. padID
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 2 then
      path = base .. "/setText?padID=" .. padID
      path = path .. "&text=Hello"
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 3 then
      path = base .. "/getText?padID=" .. padID
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 4 then
      path = base .. "/setText?padID=" .. padID
      path = path .. "&text=From"
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 5 then
      path = base .. "/getText?padID=" .. padID
      path = path .. api_key
      requestID = requestID + 1
   elseif requestID == 6 then
      path = base .. "/deletePad?padID=" .. padID
      path = path .. api_key
      requestID = 0
      --userID = "guest" .. tonumber( tostring( {} ):sub(8) )
   end

   --print(path)
   return wrk.format(nil, path)
end
