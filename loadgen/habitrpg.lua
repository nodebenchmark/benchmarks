local json = require("loadgen.json")

init = function(args)
  _v = 0
  rand1 = ""
  rand2 = ""
  soccer = ""
  todo = ""
  date = ""
  phase = 0
end

request = function()
  date = string.format("%s%d", os.time(), math.random(100, 999))
  _v = tonumber( tostring( {} ):sub(8) )

  wrk.method = "POST"
  wrk.path = "/api/v2/user/batch-update?_v=" .. _v .. "&data=" .. date
  wrk.headers["Content-Type"] = "application/json"
  wrk.headers["x-api-user"] = "ac41bda8-fb6d-41f6-9b53-241117bab630"
  wrk.headers["x-api-key"] = "9623f724-b910-442e-82e3-91a4ac2a9737"

  if phase == 0 then
    rand1 = string.format("%x", math.random(1000, 0Xffff))
    rand2 = string.format("%x", math.random(1000, 0Xffff))
    soccer = string.format("fadb85a4-%s-%s-ae8a-d72724a26007", rand1, rand2)
    todo = string.format("24ff95fd-%s-%s-800e-660441ed04ad", rand1, rand2)
    --Add Todo Task
    wrk.body = string.format("[{\"op\": \"addTask\",\"body\":{\"text\":\"Watch Soccer %s\",\"type\":\"todo\",\"tags\":{},\"id\":\"%s\",\"notes\":\"\",\"priority\":1,\"challenge\":{},\"attribute\":\"str\",\"dateCreated\":\"2016-03-09T15:36:39.%dZ\",\"completed\":false,\"_id\":\"%s\",\"value\":0,\"_advanced\":true}}]", rand1, todo, math.random(0, 1000), todo)
  elseif phase == 1 then
    --Delete Todo Task
    wrk.body = string.format("[{\"op\": \"deleteTask\", \"params\":{\"id\":\"%s\"}}]", todo)
  elseif phase == 2 then
    --Add Habit Task
    wrk.body = string.format("[{\"op\": \"addTask\",\"body\":{\"text\":\"Play Soccer %s\",\"type\":\"habit\",\"tags\":{},\"id\":\"%s\",\"notes\":\"\",\"priority\":1,\"challenge\":{},\"attribute\":\"str\",\"dateCreated\":\"2016-03-09T15:46:39.%dZ\",\"up\":true,\"down\":true,\"history\":[],\"_id\":\"%s\",\"value\":0,\"_advanced\":true}}]", rand1, soccer, math.random(0, 1000), soccer)
  elseif phase == 3 then
    --Edit Habit Task
    wrk.body = string.format("[{\"op\":\"updateTask\",\"params\":{\"id\":\"%s\"},\"body\":{\"text\":\"Play Soccer %s\",\"type\":\"habit\",\"tags\":{\"0b0128e2-01c4-4a1b-848e-94328622482a\":true},\"id\":\"%s\",\"notes\":\"Once a week\",\"priority\":1.5,\"challenge\":{},\"attribute\":\"str\",\"dateCreated\":\"2016-03-09T16:12:33.%dZ\",\"up\":true,\"down\":true,\"history\":[],\"_id\":\"%s\",\"value\":0,\"_advanced\":true,\"_editing\":false,\"_tags\":true}}]", soccer, rand1, soccer, math.random(0, 1000), soccer)
  elseif phase == 4 then
    --Update Habit Task Score
    wrk.body = string.format("[{\"op\": \"score\", \"params\": {\"id\": \"%s\", \"direction\": \"up\"}}]", soccer)
  elseif phase == 5 then
    --Delete Habit Task
    wrk.body = string.format("[{\"op\": \"deleteTask\", \"params\":{\"id\":\"%s\"}}]", soccer)
  elseif phase == 6 then
    --Update Exp
    wrk.body = string.format("[{\"op\":\"update\",\"body\":{\"stats.exp\":10006,\"stats.gp\":10001.02,\"stats.mp\":10020.75}}]")
  elseif phase == 7 then
    --Update Body
    wrk.body = string.format("[{\"op\":\"update\",\"body\":{\"preferences.shirt\":\"blue\"}}]")
  end

  --Just to get the entire database for habitrpg
  --wrk.body = "[{}]"

  --print(phase, wrk.path, wrk.body)

  return wrk.format(method, path, headers, body)
end

response = function (status, headers, body)
  --obj, pos = json.parse(body)
  --print(phase, rand1, obj["err"])

  phase = phase + 1
  if phase == 8 then
    phase = 0
  end
end
