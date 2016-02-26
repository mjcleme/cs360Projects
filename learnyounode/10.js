var net = require('net');

var port = Number(process.argv[2]);

var zeroPad = function(num) {
    if (num < 10) {
        return '0'+num;
    }
    return num;
}

var server = net.createServer(function(socket) {
    var date = new Date();
    var year = date.getFullYear();
    var month = zeroPad(date.getMonth()+1);
    var day = zeroPad(date.getDate());
    var hours = zeroPad(date.getHours());
    var minutes = zeroPad(date.getMinutes());
    var toSend = year+'-'+month+'-'+day+' '+hours+':'+minutes+'\n';
    socket.write(toSend);
    socket.end();
});

server.listen(port);
