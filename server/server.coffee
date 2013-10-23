zmq = require 'zmq'
rep = zmq.socket 'rep'
rep.bind 'tcp://127.0.0.1:5565'
rep.on 'recv', (data) ->
  console.log data.toString()
