require 'net/http'
require 'open-uri'

url = "http://www.sitopreferito.it/html/all_english_words.html"

puts Net::HTTP.get(URI.parse(url))

