require 'logger'

l = Logger.new("test.log")
l.level = Logger::WARN

file = File.open("test.log", 'w') 

while (true) do
  sleep(1)
  file.write("test_message\n")
end
