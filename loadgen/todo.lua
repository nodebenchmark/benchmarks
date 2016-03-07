local json = require("loadgen.json")

init = function(args)
   requestID = 0
   responseID = 0
   todoID = ""
end

request = function()
   if requestID == 0 then
      path = "/todos/create"
      wrk.body   = "{ \"description\" : \"todo-test\" }"
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 1 then
      path = "/todos"
      method = "GET"
      requestID = requestID + 1
   elseif requestID == 2 then
      path = "/todos/delete"
      wrk.body   = "{ \"id\" : \"" .. todoID .. "\" }"
      wrk.headers["Content-Type"] = "application/json"
      method = "POST"
      requestID = requestID + 1
   elseif requestID == 3 then
      path = "/todos"
      method = "GET"
      requestID = 0
   end
   return wrk.format(method, path)
end

response = function (status, headers, body)
   if responseID == 0 then
      obj, pos = json.parse(body)
      todoID = obj["id"]
   end
   if responseID < 3 then
      responseID = responseID + 1
   elseif responseID == 3 then
      responseID = 0
   end
end

