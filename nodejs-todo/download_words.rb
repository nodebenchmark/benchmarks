require 'net/http'
require 'open-uri'

url = "http://www.sitopreferito.it/html/all_english_words.html"
#url = "http://google.com"

puts Net::HTTP.get(URI.parse(url))

