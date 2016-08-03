require 'open-uri'
require 'nokogiri'

def GetTotalResults
  data = Nokogiri::XML open 'http://www.colourlovers.com/api/colors'
  data.at_xpath('//colors/@totalResults').value.to_i
end

def GetColors(s, n)
  begin
    data = Nokogiri::XML open "http://www.colourlovers.com/api/colors?resultOffset=#{s}&numResults=#{n}"
    data.xpath('//colors/color').map do |node|
      ['rank', 'hex', 'numVotes', 'numComments', 'numHearts', 'dateCreated', 'title'].map do |member|
        node.at_xpath(member).text.gsub(/\s+/, ' ')
      end
    end
  rescue Errno::ETIMEDOUT, Timeout::Error, EOFError
    puts 'retry'
    sleep 10
    retry
  end
end

def DownloadColors(file, i, n)
  puts i
  colors = GetColors(i, n)
  colors.each_index do |j|
    file.puts ([ i+j ] + colors[j]).join "\t"
  end
end

totalResults = GetTotalResults()
raise "Too few results" unless totalResults > 0
puts "downloading #{totalResults} results..."

File.open('data.txt', 'w') do |file|
  (0...totalResults).step(100).each { |i| DownloadColors(file, i, 100) }
end
