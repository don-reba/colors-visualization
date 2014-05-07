require 'HTTParty'

def GetColors(s, n)
  HTTParty.get("http://www.colourlovers.com/api/colors?resultOffset=#{s}&numResults=#{n}").first[1]['color']
end

File.open('data.txt', 'a') do |file|
  (0...6911482).step(100).each do |i|
    begin
      colors = GetColors(i, 100)
      colors.each_index do |j|
        c = colors[j]
        file.puts "#{i+j}\t#{c['rank']}\t#{c['hex']}\t#{c['numVotes']}\t#{c['numComments']}\t#{c['numHearts']}"
      end
      puts i
    rescue Errno::ETIMEDOUT, Timeout::Error, EOFError
      puts 'retry'
      retry
    end
  end
end
