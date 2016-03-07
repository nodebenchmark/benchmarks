init = function(args)
   seed = tonumber( tostring( {} ):sub(8) )
   math.randomseed(seed)
end

request = function()
   requestID = math.random(0, 9)

   if requestID == 0 then
      path = "/favicon.ico"
      requestID = requestID + 1
   elseif requestID == 1 then
      path = "/styles/fonts/source-sans-pro/stylesheet.css"
      requestID = requestID + 1
   elseif requestID == 2 then
      path = "/styles/highlight.css"
      requestID = requestID + 1
   elseif requestID == 3 then
      path = "/styles/media.css"
      requestID = requestID + 1
   elseif requestID == 4 then
      path = "/images/line-tile.png"
      requestID = requestID + 1
   elseif requestID == 5 then
      path = "/images/header-bottom.png"
      requestID = requestID + 1
   elseif requestID == 6 then
      path = "/images/rss_feed.gif"
      requestID = requestID + 1
   elseif requestID == 7 then
      path = "/images/noise.png"
      requestID = requestID + 1
   elseif requestID == 8 then
      path = "/images/rss.png"
      requestID = requestID + 1
   elseif requestID == 9 then
      path = "/service.xml"
      requestID = 0
   end

    return wrk.format(nil, path)
end
