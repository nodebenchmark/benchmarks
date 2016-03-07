local json = require("loadgen.json")

init = function(args)
  id = tonumber( tostring( {} ):sub(8) )
  cid = 0
  name = "trinity"
  email = "trinity@gmail.com"
  company = "UT Austin"
  born = "636530400000"
  phase = 0
end

request = function()
  if phase == 0 then
    wrk.method = "POST"
    wrk.path = "/api/v1/clients"
    wrk.headers["Content-Type"] = "application/json"
    wrk.body = string.format("{ \"name\" : \"%s%d\", \"email\" : \"%d%s\", \"company\" : \"%s\", \"born\" : \"%s\" }", name, id, id, email, company, born)
    phase = 1
    return wrk.format(method, path, headers, body)
  elseif phase == 1 then
    wrk.method = "PUT"
    wrk.path = string.format("/api/v1/clients/%s", cid)
    wrk.headers["Content-Type"] = "application/json"
    wrk.body = string.format("{ \"name\" : \"%d%s%d\", \"email\" : \"%d%s\", \"company\" : \"%s\", \"born\" : \"%s\", \"_id\" : \"%s\" }", id, name, id, id, email, company, born, cid)
    id = id + 1
    phase = 2
    return wrk.format(method, path, headers, body)
  elseif phase == 2 then
    wrk.method = "DELETE"
    wrk.path = string.format("/api/v1/clients/%s", cid)
    phase = 0
    return wrk.format(method, path)
  end
end

response = function (status, headers, body)
  --print(phase, body)
  if phase == 1 then
    obj, pos = json.parse(body)
    cid = obj["_id"]
  end
end

