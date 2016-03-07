init = function(args)
   requestID = 0
end

request = function()
   path = "/search"
   method = "POST"
   if requestID == 0 then
      wrk.body   = string.format("{ \"pattern\" : \"%s\" }", "he__o")
      --print(wrk.body)
      wrk.headers["Content-Type"] = "application/json"
      requestID = requestID + 1
   elseif requestID == 1 then
      wrk.body   = string.format("{ \"pattern\" : \"%s\" }", "st???")
      --print(wrk.body)
      wrk.headers["Content-Type"] = "application/json"
      requestID = requestID + 1
   elseif requestID == 2 then
      wrk.body   = string.format("{ \"pattern\" : \"%s\" }", "/boo/")
      --print(wrk.body)
      wrk.headers["Content-Type"] = "application/json"
      requestID = 0
   end

   --print("-----")
   --print(path)
   return wrk.format(method, path)
end

